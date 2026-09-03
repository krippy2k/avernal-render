#include <avernal/render/shadow_map.hpp>
#include <avernal/core/assert.hpp>

namespace avernal::render {

std::unique_ptr<ShadowMap> ShadowMap::create(Device& device, std::uint32_t size) {
    AV_ENSURE(size > 0);

    auto shadow = std::make_unique<ShadowMap>();
    shadow->size_ = size;
    shadow->depth_ = device.create_texture({
        .width = size,
        .height = size,
        .format = Format::d32_float,
    });
    AV_ENSURE(shadow->depth_ != nullptr);

    shadow->depth_pipeline_ = device.create_graphics_pipeline({
        .use_depth = true,
        .use_3d = true,
        .depth_only = true,
    });
    AV_ENSURE(shadow->depth_pipeline_ != nullptr);
    return shadow;
}

void ShadowMap::set_view(const DirectionalLight& light, float center_x, float center_y,
    float center_z, float radius) {
    light_vp_ = directional_light_view_projection(light, center_x, center_y, center_z, radius);
}

}  // namespace avernal::render
