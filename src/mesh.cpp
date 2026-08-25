#include <avernal/render/mesh.hpp>
#include <avernal/core/assert.hpp>

namespace avernal::render {

std::unique_ptr<Mesh> Mesh::create(
    Device& device,
    std::span<const Vertex> vertices,
    std::span<const std::uint16_t> indices) {
    
    AV_ENSURE(!vertices.empty());
    AV_ENSURE(!indices.empty());

    auto mesh = std::make_unique<Mesh>();

    // Create vertex buffer
    mesh->vertex_buffer_ = device.create_buffer({
        .size = static_cast<std::uint32_t>(vertices.size() * sizeof(Vertex)),
        .usage = BufferUsage::vertex,
        .data = vertices.data(),
    });

    // Create index buffer
    mesh->index_buffer_ = device.create_buffer({
        .size = static_cast<std::uint32_t>(indices.size() * sizeof(std::uint16_t)),
        .usage = BufferUsage::index,
        .data = indices.data(),
    });

    mesh->index_count_ = static_cast<std::uint32_t>(indices.size());

    return mesh;
}

}  // namespace avernal::render
