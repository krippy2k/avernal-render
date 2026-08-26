#include <avernal/render/mesh.hpp>

#include <avernal/core/assert.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>
#include <limits>

namespace avernal::render {
namespace {

[[nodiscard]] Submesh default_submesh(std::uint32_t index_count, const Bounds& bounds) {
    Submesh submesh{};
    submesh.first_index = 0;
    submesh.index_count = index_count;
    submesh.vertex_offset = 0;
    submesh.material_slot = 0;
    std::memcpy(submesh.bounds.min, bounds.min, sizeof(bounds.min));
    std::memcpy(submesh.bounds.max, bounds.max, sizeof(bounds.max));
    return submesh;
}

}  // namespace

Bounds compute_vertex_bounds(std::span<const Vertex> vertices) {
    Bounds bounds{};
    if (vertices.empty()) {
        return bounds;
    }

    float min_x = std::numeric_limits<float>::infinity();
    float min_y = std::numeric_limits<float>::infinity();
    float min_z = std::numeric_limits<float>::infinity();
    float max_x = -std::numeric_limits<float>::infinity();
    float max_y = -std::numeric_limits<float>::infinity();
    float max_z = -std::numeric_limits<float>::infinity();

    for (const auto& vertex : vertices) {
        min_x = std::min(min_x, vertex.position[0]);
        min_y = std::min(min_y, vertex.position[1]);
        min_z = std::min(min_z, vertex.position[2]);
        max_x = std::max(max_x, vertex.position[0]);
        max_y = std::max(max_y, vertex.position[1]);
        max_z = std::max(max_z, vertex.position[2]);
    }

    bounds.min[0] = min_x;
    bounds.min[1] = min_y;
    bounds.min[2] = min_z;
    bounds.max[0] = max_x;
    bounds.max[1] = max_y;
    bounds.max[2] = max_z;
    bounds.sphere_center[0] = (min_x + max_x) * 0.5f;
    bounds.sphere_center[1] = (min_y + max_y) * 0.5f;
    bounds.sphere_center[2] = (min_z + max_z) * 0.5f;
    const float dx = max_x - min_x;
    const float dy = max_y - min_y;
    const float dz = max_z - min_z;
    bounds.sphere_radius = 0.5f * std::sqrt(dx * dx + dy * dy + dz * dz);
    return bounds;
}

MeshGeometry mesh_geometry_from_vertices(
    std::span<const Vertex> vertices, std::span<const std::uint16_t> indices) {
    MeshGeometry geometry{};
    geometry.index_format = IndexFormat::uint16;
    geometry.streams.push_back(VertexStream{
        .stride = vertex_stride,
        .vertex_count = static_cast<std::uint32_t>(vertices.size()),
    });
    geometry.attributes.assign(std::begin(default_vertex_attributes),
        std::end(default_vertex_attributes));

    std::vector<std::byte> vertex_bytes(vertices.size() * sizeof(Vertex));
    if (!vertices.empty()) {
        std::memcpy(vertex_bytes.data(), vertices.data(), vertex_bytes.size());
    }
    geometry.stream_data.push_back(std::move(vertex_bytes));

    geometry.index_data.resize(indices.size() * sizeof(std::uint16_t));
    if (!indices.empty()) {
        std::memcpy(geometry.index_data.data(), indices.data(), geometry.index_data.size());
    }

    geometry.bounds = compute_vertex_bounds(vertices);
    geometry.submeshes.push_back(
        default_submesh(static_cast<std::uint32_t>(indices.size()), geometry.bounds));
    return geometry;
}

Buffer* Mesh::vertex_buffer(std::size_t stream) const noexcept {
    return stream < vertex_buffers_.size() ? vertex_buffers_[stream].get() : nullptr;
}

std::uint32_t Mesh::stream_stride(std::size_t stream) const noexcept {
    return stream < stream_strides_.size() ? stream_strides_[stream] : 0;
}

std::uint32_t Mesh::vertex_count(std::size_t stream) const noexcept {
    return stream < stream_vertex_counts_.size() ? stream_vertex_counts_[stream] : 0;
}

std::unique_ptr<Mesh> Mesh::create(
    Device& device, std::span<const Vertex> vertices, std::span<const std::uint16_t> indices) {
    return create(device, mesh_geometry_from_vertices(vertices, indices));
}

std::unique_ptr<Mesh> Mesh::create(Device& device, const MeshGeometry& geometry) {
    AV_ENSURE(!geometry.streams.empty());
    AV_ENSURE(geometry.streams.size() == geometry.stream_data.size());
    AV_ENSURE(!geometry.index_data.empty());

    auto mesh = std::make_unique<Mesh>();
    mesh->flags_ = geometry.flags;
    mesh->index_format_ = geometry.index_format;
    mesh->attributes_ = geometry.attributes;
    mesh->submeshes_ = geometry.submeshes;
    mesh->bounds_ = geometry.bounds;
    mesh->index_count_ = static_cast<std::uint32_t>(
        geometry.index_data.size() / index_format_size(geometry.index_format));

    mesh->vertex_buffers_.reserve(geometry.streams.size());
    mesh->stream_strides_.reserve(geometry.streams.size());
    mesh->stream_vertex_counts_.reserve(geometry.streams.size());

    for (std::size_t i = 0; i < geometry.streams.size(); ++i) {
        const auto& stream = geometry.streams[i];
        AV_ENSURE(stream.stride % avmesh_stream_stride_alignment == 0);
        const auto& data = geometry.stream_data[i];
        AV_ENSURE(!data.empty());

        mesh->vertex_buffers_.push_back(device.create_buffer({
            .size = data.size(),
            .usage = BufferUsage::vertex,
            .data = data.data(),
        }));
        mesh->stream_strides_.push_back(stream.stride);
        mesh->stream_vertex_counts_.push_back(stream.vertex_count);
    }

    mesh->index_buffer_ = device.create_buffer({
        .size = geometry.index_data.size(),
        .usage = BufferUsage::index,
        .data = geometry.index_data.data(),
    });

    if (mesh->submeshes_.empty()) {
        mesh->submeshes_.push_back(default_submesh(mesh->index_count_, mesh->bounds_));
    }

    return mesh;
}

}  // namespace avernal::render
