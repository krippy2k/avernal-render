#pragma once

#include <avernal/render/camera.hpp>
#include <cmath>
#include <cstdint>

namespace avernal::render {

struct DirectionalLight {
    float direction[3]{-0.55f, -0.82f, -0.38f};
    float intensity{1.35f};
    float color[3]{1.0f, 0.97f, 0.90f};
    float ambient{0.24f};
};

struct LitFrameConstants {
    Matrix4x4 mvp = Matrix4x4::identity();
    Matrix4x4 model = Matrix4x4::identity();
    Matrix4x4 light_view_proj = Matrix4x4::identity();
    float light_dir_intensity[4]{};
    float light_color_ambient[4]{};
    float camera_pos[4]{};
    float material[4]{};
};

static_assert(sizeof(LitFrameConstants) == 256);

[[nodiscard]] inline Matrix4x4 directional_light_view_projection(const DirectionalLight& light,
    float center_x, float center_y, float center_z, float radius) {
    float dx = light.direction[0];
    float dy = light.direction[1];
    float dz = light.direction[2];
    const float length = std::sqrt(dx * dx + dy * dy + dz * dz);
    dx /= length;
    dy /= length;
    dz /= length;

    const float distance = radius + 8.0f;
    const float eye_x = center_x - dx * distance;
    const float eye_y = center_y - dy * distance;
    const float eye_z = center_z - dz * distance;

    float up_x = 0.0f;
    float up_y = 1.0f;
    float up_z = 0.0f;
    if (std::abs(dy) > 0.9f) {
        up_y = 0.0f;
        up_z = 1.0f;
    }

    const auto view = Matrix4x4::look_at(
        eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z);
    const auto projection =
        Matrix4x4::ortho(-radius, radius, -radius, radius, 1.0f, distance + radius + 4.0f);
    return Matrix4x4::multiply(projection, view);
}

inline void fill_lit_frame_constants(LitFrameConstants& constants, const Matrix4x4& model,
    const Matrix4x4& view_projection, const Matrix4x4& light_view_proj, const DirectionalLight& light,
    float camera_x, float camera_y, float camera_z, float metallic, float roughness) {
    constants.mvp = Matrix4x4::multiply(view_projection, model);
    constants.model = model;
    constants.light_view_proj = light_view_proj;
    float dx = light.direction[0];
    float dy = light.direction[1];
    float dz = light.direction[2];
    const float length = std::sqrt(dx * dx + dy * dy + dz * dz);
    constants.light_dir_intensity[0] = dx / length;
    constants.light_dir_intensity[1] = dy / length;
    constants.light_dir_intensity[2] = dz / length;
    constants.light_dir_intensity[3] = light.intensity;
    constants.light_color_ambient[0] = light.color[0];
    constants.light_color_ambient[1] = light.color[1];
    constants.light_color_ambient[2] = light.color[2];
    constants.light_color_ambient[3] = light.ambient;
    constants.camera_pos[0] = camera_x;
    constants.camera_pos[1] = camera_y;
    constants.camera_pos[2] = camera_z;
    constants.camera_pos[3] = 1.0f;
    constants.material[0] = metallic;
    constants.material[1] = roughness;
    constants.material[2] = 0.0f;
    constants.material[3] = 0.0f;
}

}  // namespace avernal::render
