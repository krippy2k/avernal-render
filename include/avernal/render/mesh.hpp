#pragma once

#include <avernal/rhi/rhi.hpp>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

namespace avernal::render {

/// Vertex format for rendering
struct Vertex {
    float position[3];  // xyz
    float normal[3];    // xyz
    float texcoord[2];  // uv
    float color[4];     // rgba
};

inline constexpr std::uint32_t vertex_stride = sizeof(Vertex);

/// Mesh represents geometry data for rendering
class Mesh {
public:
    Mesh() = default;
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    /// Create a mesh from vertex and index data
    static std::unique_ptr<Mesh> create(
        Device& device,
        std::span<const Vertex> vertices,
        std::span<const std::uint16_t> indices);

    /// Get the vertex buffer
    [[nodiscard]] Buffer* vertex_buffer() const noexcept { return vertex_buffer_.get(); }

    /// Get the index buffer
    [[nodiscard]] Buffer* index_buffer() const noexcept { return index_buffer_.get(); }

    /// Get the number of indices
    [[nodiscard]] std::uint32_t index_count() const noexcept { return index_count_; }

private:
    std::unique_ptr<Buffer> vertex_buffer_;
    std::unique_ptr<Buffer> index_buffer_;
    std::uint32_t index_count_{};
};

}  // namespace avernal::render
