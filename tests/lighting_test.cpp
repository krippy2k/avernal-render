#include <avernal/render/camera.hpp>
#include <avernal/render/light.hpp>
#include <avernal/rhi/rhi.hpp>

#include <cmath>
#include <gtest/gtest.h>

TEST(Matrix4x4, OrthoPlacesOriginAtClipCenter) {
    const auto projection = avernal::render::Matrix4x4::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 1.0f, 33.0f);
    EXPECT_FLOAT_EQ(projection.m[0], 0.125f);
    EXPECT_FLOAT_EQ(projection.m[5], 0.125f);
    EXPECT_FLOAT_EQ(projection.m[10], 1.0f / 32.0f);
    EXPECT_FLOAT_EQ(projection.m[14], -1.0f / 32.0f);
    EXPECT_FLOAT_EQ(projection.m[15], 1.0f);
}

TEST(LitFrameConstants, OccupiesOneConstantBufferPage) {
    EXPECT_EQ(sizeof(avernal::render::LitFrameConstants), 256u);
}

TEST(DirectionalLight, ViewProjectionLooksAlongLightDirection) {
    avernal::render::DirectionalLight light{};
    light.direction[0] = 0.0f;
    light.direction[1] = -1.0f;
    light.direction[2] = 0.0f;
    const auto matrix =
        avernal::render::directional_light_view_projection(light, 0.0f, 0.0f, 0.0f, 8.0f);
    float sum = 0.0f;
    for (float value : matrix.m) {
        sum += value * value;
        EXPECT_TRUE(std::isfinite(value));
    }
    EXPECT_GT(sum, 0.0f);
}

TEST(Device, NullCreatesDepthTexture) {
    const auto device = avernal::create_null_device();
    ASSERT_NE(device, nullptr);
    const auto depth = device->create_texture({
        .width = 64,
        .height = 64,
        .format = avernal::Format::d32_float,
    });
    ASSERT_NE(depth, nullptr);

    auto pipeline = device->create_graphics_pipeline({
        .use_depth = true,
        .use_3d = true,
        .depth_only = true,
    });
    ASSERT_NE(pipeline, nullptr);

    auto commands = device->create_command_list();
    commands->reset();
    commands->begin_depth(*depth);
    commands->clear_depth(1.0f);
    commands->end_render();
    commands->close();
}
