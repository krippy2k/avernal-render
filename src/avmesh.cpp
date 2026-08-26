#include <avernal/render/avmesh.hpp>

#include <algorithm>
#include <cstring>

namespace avernal::render {
namespace {

constexpr std::size_t header_size = sizeof(AvmeshHeader);
constexpr std::size_t chunk_entry_size = sizeof(ChunkTableEntry);

[[nodiscard]] bool has_magic(const AvmeshHeader& header) noexcept {
    return header.magic[0] == avmesh_magic[0] && header.magic[1] == avmesh_magic[1] &&
           header.magic[2] == avmesh_magic[2] && header.magic[3] == avmesh_magic[3];
}

class Writer {
public:
    void write_bytes(const void* data, std::size_t size) {
        const auto* bytes = static_cast<const std::byte*>(data);
        buffer_.insert(buffer_.end(), bytes, bytes + size);
    }

    template<typename T>
    void write(const T& value) {
        write_bytes(&value, sizeof(T));
    }

    void pad_to(std::uint64_t alignment) {
        const auto aligned = align_up(buffer_.size(), alignment);
        buffer_.resize(static_cast<std::size_t>(aligned), std::byte{0});
    }

    [[nodiscard]] std::size_t size() const noexcept { return buffer_.size(); }
    [[nodiscard]] std::vector<std::byte>& buffer() noexcept { return buffer_; }

    void write_at(std::size_t offset, const void* data, std::size_t size) {
        std::memcpy(buffer_.data() + offset, data, size);
    }

private:
    std::vector<std::byte> buffer_{};
};

[[nodiscard]] std::optional<std::span<const std::byte>> chunk_payload(
    std::span<const std::byte> bytes, const ChunkTableEntry& entry) {
    if (entry.compression != Compression::none) {
        return std::nullopt;
    }
    if (entry.compressed_size != entry.uncompressed_size) {
        return std::nullopt;
    }
    if (entry.offset > bytes.size()) {
        return std::nullopt;
    }
    const auto remaining = bytes.size() - static_cast<std::size_t>(entry.offset);
    if (entry.uncompressed_size > remaining) {
        return std::nullopt;
    }
    return bytes.subspan(static_cast<std::size_t>(entry.offset),
        static_cast<std::size_t>(entry.uncompressed_size));
}

template<typename T>
[[nodiscard]] bool read_value(std::span<const std::byte> bytes, std::size_t& offset, T& out) {
    if (offset + sizeof(T) > bytes.size()) {
        return false;
    }
    std::memcpy(&out, bytes.data() + offset, sizeof(T));
    offset += sizeof(T);
    return true;
}

[[nodiscard]] const ChunkTableEntry* find_chunk(
    std::span<const ChunkTableEntry> table, ChunkType type) noexcept {
    for (const auto& entry : table) {
        if (entry.type == type) {
            return &entry;
        }
    }
    return nullptr;
}

}  // namespace

std::vector<std::byte> write_avmesh(const MeshGeometry& geometry) {
    const auto stream_count = static_cast<std::uint32_t>(geometry.streams.size());
    const auto attribute_count = static_cast<std::uint32_t>(geometry.attributes.size());
    const auto submesh_count = static_cast<std::uint32_t>(geometry.submeshes.size());

    std::uint32_t vertex_count = 0;
    if (!geometry.streams.empty()) {
        vertex_count = geometry.streams.front().vertex_count;
    }

    const auto index_stride = index_format_size(geometry.index_format);
    const auto index_count = index_stride == 0
                                 ? 0u
                                 : static_cast<std::uint32_t>(geometry.index_data.size() / index_stride);

    MeshInfo info{};
    info.flags = geometry.flags;
    info.index_format = geometry.index_format;
    info.vertex_count = vertex_count;
    info.index_count = index_count;
    info.stream_count = stream_count;
    info.attribute_count = attribute_count;
    info.submesh_count = submesh_count;

    std::vector<std::byte> layout;
    {
        Writer writer;
        writer.write(stream_count);
        writer.write(attribute_count);
        for (const auto& stream : geometry.streams) {
            writer.write(stream);
        }
        for (const auto& attribute : geometry.attributes) {
            writer.write(attribute);
        }
        layout = std::move(writer.buffer());
    }

    std::vector<std::byte> vertex_data;
    {
        Writer writer;
        for (std::size_t i = 0; i < geometry.streams.size(); ++i) {
            const auto& stream = geometry.streams[i];
            const auto expected = static_cast<std::size_t>(stream.stride) * stream.vertex_count;
            const auto& data = i < geometry.stream_data.size() ? geometry.stream_data[i]
                                                               : std::vector<std::byte>{};
            writer.write_bytes(data.data(), std::min(expected, data.size()));
            if (data.size() < expected) {
                writer.buffer().resize(writer.size() + (expected - data.size()), std::byte{0});
            }
            writer.pad_to(avmesh_chunk_alignment);
        }
        vertex_data = std::move(writer.buffer());
    }

    std::vector<std::byte> submeshes;
    {
        Writer writer;
        writer.write(submesh_count);
        for (const auto& submesh : geometry.submeshes) {
            writer.write(submesh);
        }
        submeshes = std::move(writer.buffer());
    }

    std::vector<std::byte> bounds(sizeof(Bounds));
    std::memcpy(bounds.data(), &geometry.bounds, sizeof(Bounds));

    struct Payload {
        ChunkType type{};
        std::span<const std::byte> bytes{};
    };

    std::vector<std::byte> info_bytes(sizeof(MeshInfo));
    std::memcpy(info_bytes.data(), &info, sizeof(MeshInfo));

    const Payload payloads[] = {
        {ChunkType::mesh_info, info_bytes},
        {ChunkType::vertex_layout, layout},
        {ChunkType::vertex_data, vertex_data},
        {ChunkType::index_data, geometry.index_data},
        {ChunkType::submeshes, submeshes},
        {ChunkType::bounds, bounds},
    };

    constexpr std::uint32_t chunk_count = 6;
    const auto table_bytes = chunk_count * chunk_entry_size;
    Writer file;
    AvmeshHeader header{};
    std::memcpy(header.magic, avmesh_magic.data(), 4);
    header.version = avmesh_version;
    header.flags = 0;
    header.chunk_count = chunk_count;
    file.write(header);
    file.buffer().resize(header_size + table_bytes, std::byte{0});

    std::array<ChunkTableEntry, chunk_count> table{};
    for (std::uint32_t i = 0; i < chunk_count; ++i) {
        file.pad_to(avmesh_chunk_alignment);
        auto& entry = table[i];
        entry.type = payloads[i].type;
        entry.offset = file.size();
        entry.compressed_size = payloads[i].bytes.size();
        entry.uncompressed_size = payloads[i].bytes.size();
        entry.compression = Compression::none;
        file.write_bytes(payloads[i].bytes.data(), payloads[i].bytes.size());
    }

    file.write_at(header_size, table.data(), sizeof(table));
    return std::move(file.buffer());
}

std::optional<MeshGeometry> read_avmesh(std::span<const std::byte> bytes) {
    if (bytes.size() < header_size) {
        return std::nullopt;
    }

    AvmeshHeader header{};
    std::memcpy(&header, bytes.data(), sizeof(header));
    if (!has_magic(header) || header.version != avmesh_version) {
        return std::nullopt;
    }

    const auto table_bytes = static_cast<std::uint64_t>(header.chunk_count) * chunk_entry_size;
    if (header_size + table_bytes > bytes.size()) {
        return std::nullopt;
    }

    std::vector<ChunkTableEntry> table(header.chunk_count);
    std::memcpy(table.data(), bytes.data() + header_size, static_cast<std::size_t>(table_bytes));

    const auto* info_entry = find_chunk(table, ChunkType::mesh_info);
    const auto* layout_entry = find_chunk(table, ChunkType::vertex_layout);
    const auto* vertex_entry = find_chunk(table, ChunkType::vertex_data);
    const auto* index_entry = find_chunk(table, ChunkType::index_data);
    const auto* submesh_entry = find_chunk(table, ChunkType::submeshes);
    const auto* bounds_entry = find_chunk(table, ChunkType::bounds);
    if (info_entry == nullptr || layout_entry == nullptr || vertex_entry == nullptr ||
        index_entry == nullptr || submesh_entry == nullptr || bounds_entry == nullptr) {
        return std::nullopt;
    }

    const auto info_bytes = chunk_payload(bytes, *info_entry);
    const auto layout_bytes = chunk_payload(bytes, *layout_entry);
    const auto vertex_bytes = chunk_payload(bytes, *vertex_entry);
    const auto index_bytes = chunk_payload(bytes, *index_entry);
    const auto submesh_bytes = chunk_payload(bytes, *submesh_entry);
    const auto bounds_bytes = chunk_payload(bytes, *bounds_entry);
    if (!info_bytes || !layout_bytes || !vertex_bytes || !index_bytes || !submesh_bytes ||
        !bounds_bytes) {
        return std::nullopt;
    }
    if (info_bytes->size() < sizeof(MeshInfo) || bounds_bytes->size() < sizeof(Bounds)) {
        return std::nullopt;
    }

    MeshInfo info{};
    std::memcpy(&info, info_bytes->data(), sizeof(MeshInfo));

    std::size_t layout_offset = 0;
    std::uint32_t stream_count = 0;
    std::uint32_t attribute_count = 0;
    if (!read_value(*layout_bytes, layout_offset, stream_count) ||
        !read_value(*layout_bytes, layout_offset, attribute_count)) {
        return std::nullopt;
    }
    if (stream_count != info.stream_count || attribute_count != info.attribute_count) {
        return std::nullopt;
    }

    MeshGeometry geometry{};
    geometry.flags = info.flags;
    geometry.index_format = info.index_format;
    geometry.streams.resize(stream_count);
    geometry.attributes.resize(attribute_count);
    geometry.stream_data.resize(stream_count);

    for (auto& stream : geometry.streams) {
        if (!read_value(*layout_bytes, layout_offset, stream)) {
            return std::nullopt;
        }
        if (stream.stride == 0 || stream.stride % avmesh_stream_stride_alignment != 0) {
            return std::nullopt;
        }
    }
    for (auto& attribute : geometry.attributes) {
        if (!read_value(*layout_bytes, layout_offset, attribute)) {
            return std::nullopt;
        }
        if (attribute.stream >= stream_count) {
            return std::nullopt;
        }
    }

    std::size_t vertex_offset = 0;
    for (std::uint32_t i = 0; i < stream_count; ++i) {
        const auto size =
            static_cast<std::size_t>(geometry.streams[i].stride) * geometry.streams[i].vertex_count;
        if (vertex_offset + size > vertex_bytes->size()) {
            return std::nullopt;
        }
        geometry.stream_data[i].assign(
            vertex_bytes->begin() + static_cast<std::ptrdiff_t>(vertex_offset),
            vertex_bytes->begin() + static_cast<std::ptrdiff_t>(vertex_offset + size));
        vertex_offset = static_cast<std::size_t>(align_up(vertex_offset + size, avmesh_chunk_alignment));
    }

    geometry.index_data.assign(index_bytes->begin(), index_bytes->end());

    std::size_t submesh_offset = 0;
    std::uint32_t submesh_count = 0;
    if (!read_value(*submesh_bytes, submesh_offset, submesh_count) ||
        submesh_count != info.submesh_count) {
        return std::nullopt;
    }
    geometry.submeshes.resize(submesh_count);
    for (auto& submesh : geometry.submeshes) {
        if (!read_value(*submesh_bytes, submesh_offset, submesh)) {
            return std::nullopt;
        }
    }

    std::memcpy(&geometry.bounds, bounds_bytes->data(), sizeof(Bounds));
    return geometry;
}

}  // namespace avernal::render
