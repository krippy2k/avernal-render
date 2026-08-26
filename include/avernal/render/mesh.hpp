#pragma once

#include <avernal/render/avmesh.hpp>
#include <avernal/rhi/rhi.hpp>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace avernal::render {

struct Vertex {
    float position[3];
    float normal[3];
    float texcoord[2];
    float color[4];
};

inline constexpr std::uint32_t vertex_stride = sizeof(Vertex);

inline constexpr VertexAttribute default_vertex_attributes[] = {
    {VertexSemantic::position, VertexFormat::float32x3, 0, 0},
    {VertexSemantic::normal, VertexFormat::float32x3, 0, 12},
    {VertexSemantic::tex_coord0, VertexFormat::float32x2, 0, 24},
    {VertexSemantic::color0, VertexFormat::float32x4, 0, 32},
};

[[nodiscard]] MeshGeometry mesh_geometry_from_vertices(
    std::span<const Vertex> vertices, std::span<const std::uint16_t> indices);

[[nodiscard]] Bounds compute_vertex_bounds(std::span<const Vertex> vertices);

class Mesh {
public:
    Mesh() = default;
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    static std::unique_ptr<Mesh> create(
        Device& device, std::span<const Vertex> vertices, std::span<const std::uint16_t> indices);

    static std::unique_ptr<Mesh> create(Device& device, const MeshGeometry& geometry);

    [[nodiscard]] Buffer* vertex_buffer(std::size_t stream = 0) const noexcept;
    [[nodiscard]] Buffer* index_buffer() const noexcept { return index_buffer_.get(); }

    [[nodiscard]] std::uint32_t stream_count() const noexcept {
        return static_cast<std::uint32_t>(vertex_buffers_.size());
    }
    [[nodiscard]] std::uint32_t stream_stride(std::size_t stream = 0) const noexcept;
    [[nodiscard]] std::uint32_t vertex_count(std::size_t stream = 0) const noexcept;
    [[nodiscard]] std::uint32_t vertex_stride() const noexcept { return stream_stride(0); }
    [[nodiscard]] std::uint32_t index_count() const noexcept { return index_count_; }
    [[nodiscard]] IndexFormat index_format() const noexcept { return index_format_; }
    [[nodiscard]] std::uint32_t flags() const noexcept { return flags_; }

    [[nodiscard]] std::span<const VertexAttribute> attributes() const noexcept { return attributes_; }
    [[nodiscard]] std::span<const Submesh> submeshes() const noexcept { return submeshes_; }
    [[nodiscard]] const Bounds& bounds() const noexcept { return bounds_; }

private:
    std::vector<std::unique_ptr<Buffer>> vertex_buffers_{};
    std::vector<std::uint32_t> stream_strides_{};
    std::vector<std::uint32_t> stream_vertex_counts_{};
    std::unique_ptr<Buffer> index_buffer_{};
    std::vector<VertexAttribute> attributes_{};
    std::vector<Submesh> submeshes_{};
    Bounds bounds_{};
    IndexFormat index_format_{IndexFormat::uint16};
    std::uint32_t flags_{};
    std::uint32_t index_count_{};
};

}  // namespace avernal::render
