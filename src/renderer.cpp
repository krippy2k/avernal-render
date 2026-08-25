#include <avernal/render/renderer.hpp>
#include <avernal/render/camera.hpp>
#include <avernal/render/material.hpp>
#include <avernal/render/mesh.hpp>
#include <avernal/render/texture.hpp>
#include <avernal/core/assert.hpp>

namespace avernal::render {

Renderer::Renderer(Device& device) : device_(device) {}

Renderer::~Renderer() = default;

void Renderer::begin_frame(Swapchain& swapchain, CommandList& commands) {
    AV_ENSURE(!in_frame_);
    current_swapchain_ = &swapchain;
    current_commands_ = &commands;
    in_frame_ = true;

    commands.reset();
    commands.begin_render(swapchain);
}

void Renderer::end_frame() {
    AV_ENSURE(in_frame_);
    current_commands_->end_render();
    current_commands_->close();
    in_frame_ = false;
}

void Renderer::render(const Mesh& mesh, const Material& material, const Camera& camera) {
    AV_ENSURE(in_frame_);
    AV_ENSURE(current_commands_ != nullptr);

    // Set pipeline
    current_commands_->set_pipeline(*material.pipeline());

    // Set vertex and index buffers
    current_commands_->set_vertex_buffer(*mesh.vertex_buffer(), vertex_stride);
    current_commands_->set_index_buffer(*mesh.index_buffer());

    // Set texture if available
    if (material.texture()) {
        current_commands_->set_texture(*material.texture()->rhi_texture());
    }

    // Draw indexed
    current_commands_->draw_indexed(mesh.index_count());
}

void Renderer::clear(const Color& color) {
    AV_ENSURE(in_frame_);
    AV_ENSURE(current_commands_ != nullptr);
    current_commands_->clear_color(color);
}

}  // namespace avernal::render
