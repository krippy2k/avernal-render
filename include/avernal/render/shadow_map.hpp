#pragma once

#include <avernal/render/camera.hpp>
#include <avernal/render/light.hpp>
#include <avernal/rhi/rhi.hpp>
#include <cstdint>
#include <memory>

namespace avernal::render {

class ShadowMap {
public:
    static std::unique_ptr<ShadowMap> create(Device& device, std::uint32_t size = 1024);

    void set_view(const DirectionalLight& light, float center_x, float center_y, float center_z,
        float radius);

    [[nodiscard]] avernal::Texture& texture() noexcept { return *depth_; }
    [[nodiscard]] Pipeline& depth_pipeline() noexcept { return *depth_pipeline_; }
    [[nodiscard]] const Matrix4x4& light_view_projection() const noexcept { return light_vp_; }
    [[nodiscard]] std::uint32_t size() const noexcept { return size_; }

private:
    std::unique_ptr<avernal::Texture> depth_{};
    std::unique_ptr<Pipeline> depth_pipeline_{};
    Matrix4x4 light_vp_ = Matrix4x4::identity();
    std::uint32_t size_{};
};

}  // namespace avernal::render
