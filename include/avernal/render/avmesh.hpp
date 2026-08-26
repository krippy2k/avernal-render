#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace avernal::render {

inline constexpr std::uint32_t avmesh_version = 1;
inline constexpr std::array<char, 4> avmesh_magic{'A', 'V', 'M', 'S'};
inline constexpr std::uint32_t avmesh_chunk_alignment = 8;
inline constexpr std::uint32_t avmesh_stream_stride_alignment = 4;

enum class ChunkType : std::uint32_t {
    mesh_info = 0x00000001,
    vertex_layout = 0x00000002,
    vertex_data = 0x00000003,
    index_data = 0x00000004,
    submeshes = 0x00000005,
    bounds = 0x00000006,
};

enum class Compression : std::uint32_t {
    none = 0,
    lz4 = 1,
};

enum class VertexSemantic : std::uint8_t {
    position = 0,
    normal,
    tangent,
    tex_coord0,
    tex_coord1,
    color0,
    joints0,
    weights0,
};

enum class VertexFormat : std::uint8_t {
    float32x2 = 0,
    float32x3,
    float32x4,
    float16x2,
    float16x4,
    unorm8x4,
    snorm8x4,
    uint8x4,
    uint16x4,
};

enum class IndexFormat : std::uint8_t {
    uint16 = 0,
    uint32 = 1,
};

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif

struct AvmeshHeader {
    char magic[4];
    std::uint32_t version;
    std::uint32_t flags;
    std::uint32_t chunk_count;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct ChunkTableEntry {
    ChunkType type;
    std::uint64_t offset;
    std::uint64_t compressed_size;
    std::uint64_t uncompressed_size;
    Compression compression;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct MeshInfo {
    std::uint32_t flags;
    IndexFormat index_format;
    std::uint8_t reserved[3];
    std::uint32_t vertex_count;
    std::uint32_t index_count;
    std::uint32_t stream_count;
    std::uint32_t attribute_count;
    std::uint32_t submesh_count;
    std::uint32_t reserved1;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct VertexAttribute {
    VertexSemantic semantic;
    VertexFormat format;
    std::uint8_t stream;
    std::uint16_t offset;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct VertexStream {
    std::uint32_t stride;
    std::uint32_t vertex_count;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct BoundingBox {
    float min[3];
    float max[3];
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct Bounds {
    float min[3];
    float max[3];
    float sphere_center[3];
    float sphere_radius;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

struct Submesh {
    std::uint32_t first_index;
    std::uint32_t index_count;
    std::int32_t vertex_offset;
    std::uint32_t material_slot;
    BoundingBox bounds;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

static_assert(sizeof(AvmeshHeader) == 16);
static_assert(sizeof(ChunkTableEntry) == 32);
static_assert(sizeof(MeshInfo) == 32);
static_assert(sizeof(VertexAttribute) == 5);
static_assert(sizeof(VertexStream) == 8);
static_assert(sizeof(BoundingBox) == 24);
static_assert(sizeof(Bounds) == 40);
static_assert(sizeof(Submesh) == 40);

struct MeshGeometry {
    std::uint32_t flags{};
    IndexFormat index_format{IndexFormat::uint16};
    std::vector<VertexStream> streams{};
    std::vector<VertexAttribute> attributes{};
    std::vector<std::vector<std::byte>> stream_data{};
    std::vector<std::byte> index_data{};
    std::vector<Submesh> submeshes{};
    Bounds bounds{};
};

[[nodiscard]] constexpr std::uint32_t vertex_format_size(VertexFormat format) noexcept {
    switch (format) {
    case VertexFormat::float32x2:
        return 8;
    case VertexFormat::float32x3:
        return 12;
    case VertexFormat::float32x4:
        return 16;
    case VertexFormat::float16x2:
        return 4;
    case VertexFormat::float16x4:
        return 8;
    case VertexFormat::unorm8x4:
    case VertexFormat::snorm8x4:
    case VertexFormat::uint8x4:
        return 4;
    case VertexFormat::uint16x4:
        return 8;
    }
    return 0;
}

[[nodiscard]] constexpr std::uint32_t index_format_size(IndexFormat format) noexcept {
    return format == IndexFormat::uint32 ? 4u : 2u;
}

[[nodiscard]] constexpr std::uint64_t align_up(std::uint64_t value, std::uint64_t alignment) noexcept {
    return (value + alignment - 1u) & ~(alignment - 1u);
}

[[nodiscard]] std::vector<std::byte> write_avmesh(const MeshGeometry& geometry);
[[nodiscard]] std::optional<MeshGeometry> read_avmesh(std::span<const std::byte> bytes);

}  // namespace avernal::render
