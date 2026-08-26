#include <avernal/render/avmesh.hpp>
#include <avernal/render/mesh.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <vector>

namespace {

avernal::render::MeshGeometry make_triangle() {
    using namespace avernal::render;

    const Vertex vertices[] = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };
    const std::uint16_t indices[] = {0, 1, 2};
    return mesh_geometry_from_vertices(vertices, indices);
}

}  // namespace

TEST(Avmesh, WritesHeaderMagicAndVersion) {
    const auto bytes = avernal::render::write_avmesh(make_triangle());
    ASSERT_GE(bytes.size(), sizeof(avernal::render::AvmeshHeader));

    avernal::render::AvmeshHeader header{};
    std::memcpy(&header, bytes.data(), sizeof(header));
    EXPECT_EQ(header.magic[0], 'A');
    EXPECT_EQ(header.magic[1], 'V');
    EXPECT_EQ(header.magic[2], 'M');
    EXPECT_EQ(header.magic[3], 'S');
    EXPECT_EQ(header.version, avernal::render::avmesh_version);
    EXPECT_EQ(header.chunk_count, 6u);
}

TEST(Avmesh, RoundtripsGeometry) {
    auto original = make_triangle();
    original.flags = 7;
    original.submeshes[0].material_slot = 3;

    const auto bytes = avernal::render::write_avmesh(original);
    const auto loaded = avernal::render::read_avmesh(bytes);
    ASSERT_TRUE(loaded.has_value());

    EXPECT_EQ(loaded->flags, original.flags);
    EXPECT_EQ(loaded->index_format, avernal::render::IndexFormat::uint16);
    ASSERT_EQ(loaded->streams.size(), 1u);
    EXPECT_EQ(loaded->streams[0].stride, avernal::render::vertex_stride);
    EXPECT_EQ(loaded->streams[0].vertex_count, 3u);
    ASSERT_EQ(loaded->attributes.size(), 4u);
    EXPECT_EQ(loaded->attributes[0].semantic, avernal::render::VertexSemantic::position);
    EXPECT_EQ(loaded->attributes[3].semantic, avernal::render::VertexSemantic::color0);
    ASSERT_EQ(loaded->stream_data.size(), 1u);
    EXPECT_EQ(loaded->stream_data[0], original.stream_data[0]);
    EXPECT_EQ(loaded->index_data, original.index_data);
    ASSERT_EQ(loaded->submeshes.size(), 1u);
    EXPECT_EQ(loaded->submeshes[0].index_count, 3u);
    EXPECT_EQ(loaded->submeshes[0].material_slot, 3u);
    EXPECT_FLOAT_EQ(loaded->bounds.min[0], -1.0f);
    EXPECT_FLOAT_EQ(loaded->bounds.max[1], 1.0f);
    EXPECT_GT(loaded->bounds.sphere_radius, 0.0f);
}

TEST(Avmesh, RejectsBadMagic) {
    auto bytes = avernal::render::write_avmesh(make_triangle());
    bytes[0] = std::byte{'X'};
    EXPECT_FALSE(avernal::render::read_avmesh(bytes).has_value());
}

TEST(Avmesh, RejectsUnsupportedVersion) {
    auto bytes = avernal::render::write_avmesh(make_triangle());
    avernal::render::AvmeshHeader header{};
    std::memcpy(&header, bytes.data(), sizeof(header));
    header.version = 99;
    std::memcpy(bytes.data(), &header, sizeof(header));
    EXPECT_FALSE(avernal::render::read_avmesh(bytes).has_value());
}

TEST(Avmesh, IgnoresUnknownChunks) {
    using namespace avernal::render;
    auto bytes = write_avmesh(make_triangle());

    AvmeshHeader header{};
    std::memcpy(&header, bytes.data(), sizeof(header));
    const auto old_count = header.chunk_count;
    header.chunk_count = old_count + 1;
    std::memcpy(bytes.data(), &header, sizeof(header));

    const auto insert_at = sizeof(AvmeshHeader) + sizeof(ChunkTableEntry) * old_count;
    bytes.insert(bytes.begin() + static_cast<std::ptrdiff_t>(insert_at), sizeof(ChunkTableEntry),
        std::byte{0});

    std::vector<ChunkTableEntry> table(old_count);
    std::memcpy(table.data(), bytes.data() + sizeof(AvmeshHeader),
        old_count * sizeof(ChunkTableEntry));
    for (auto& entry : table) {
        entry.offset += sizeof(ChunkTableEntry);
    }
    std::memcpy(bytes.data() + sizeof(AvmeshHeader), table.data(),
        old_count * sizeof(ChunkTableEntry));

    ChunkTableEntry extra{};
    extra.type = static_cast<ChunkType>(0x7FFFFFFFu);
    extra.offset = bytes.size();
    extra.compressed_size = 4;
    extra.uncompressed_size = 4;
    extra.compression = Compression::none;
    std::memcpy(bytes.data() + insert_at, &extra, sizeof(extra));
    bytes.insert(bytes.end(), 4, std::byte{0xAB});

    const auto loaded = read_avmesh(bytes);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->streams[0].vertex_count, 3u);
}

TEST(Avmesh, RejectsLz4Compression) {
    using namespace avernal::render;
    auto bytes = write_avmesh(make_triangle());
    auto* entry = reinterpret_cast<ChunkTableEntry*>(bytes.data() + sizeof(AvmeshHeader));
    entry->compression = Compression::lz4;
    EXPECT_FALSE(read_avmesh(bytes).has_value());
}

TEST(Avmesh, ChunkOffsetsAreAligned) {
    using namespace avernal::render;
    auto bytes = write_avmesh(make_triangle());
    AvmeshHeader header{};
    std::memcpy(&header, bytes.data(), sizeof(header));

    std::vector<ChunkTableEntry> table(header.chunk_count);
    std::memcpy(table.data(), bytes.data() + sizeof(AvmeshHeader),
        header.chunk_count * sizeof(ChunkTableEntry));
    for (const auto& entry : table) {
        EXPECT_EQ(entry.offset % avmesh_chunk_alignment, 0u);
        EXPECT_EQ(entry.compression, Compression::none);
        EXPECT_EQ(entry.compressed_size, entry.uncompressed_size);
    }
}

TEST(Avmesh, DefaultVertexLayoutMatchesPackedVertex) {
    EXPECT_EQ(sizeof(avernal::render::Vertex), 48u);
    EXPECT_EQ(avernal::render::default_vertex_attributes[0].offset, 0);
    EXPECT_EQ(avernal::render::default_vertex_attributes[1].offset, 12);
    EXPECT_EQ(avernal::render::default_vertex_attributes[2].offset, 24);
    EXPECT_EQ(avernal::render::default_vertex_attributes[3].offset, 32);
}
