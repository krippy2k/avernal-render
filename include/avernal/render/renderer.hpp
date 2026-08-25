#pragma once

#include <avernal/rhi/rhi.hpp>
#include <memory>
#include <vector>

namespace avernal::render {

// Forward declarations
class Mesh;
class Material;
class Camera;

/// High-level renderer that manages rendering of meshes with materials
class Renderer {
public:
    explicit Renderer(Device& device);
    ~Renderer();

    /// Begin a frame
    void begin_frame(Swapchain& swapchain, CommandList& commands);

    /// End a frame
    void end_frame();

    /// Render a mesh with a material
    void render(const Mesh& mesh, const Material& material, const Camera& camera);

    /// Clear the current render target
    void clear(const Color& color);

    [[nodiscard]] Device& device() noexcept { return device_; }

private:
    Device& device_;
    CommandList* current_commands_{};
    Swapchain* current_swapchain_{};
    bool in_frame_{};
};

}  // namespace avernal::render
