#pragma once

#include <avernal/rhi/rhi.hpp>
#include <memory>

namespace avernal::render {

// Forward declaration
class Texture;

/// Material represents the visual properties of a surface
class Material {
public:
    Material() = default;
    ~Material() = default;

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    Material(Material&&) noexcept = default;
    Material& operator=(Material&&) noexcept = default;

    /// Create a material with a pipeline
    static std::unique_ptr<Material> create(Device& device, const GraphicsPipelineDesc& desc);

    /// Set the texture for this material
    void set_texture(Texture* texture) { texture_ = texture; }

    void set_pbr(float metallic, float roughness) {
        metallic_ = metallic;
        roughness_ = roughness;
    }

    /// Get the texture
    [[nodiscard]] Texture* texture() const noexcept { return texture_; }

    [[nodiscard]] float metallic() const noexcept { return metallic_; }
    [[nodiscard]] float roughness() const noexcept { return roughness_; }

    /// Get the pipeline
    [[nodiscard]] Pipeline* pipeline() const noexcept { return pipeline_.get(); }

private:
    std::unique_ptr<Pipeline> pipeline_;
    Texture* texture_{};
    float metallic_{0.0f};
    float roughness_{1.0f};
};

}  // namespace avernal::render
