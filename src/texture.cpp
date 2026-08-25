#include <avernal/render/texture.hpp>
#include <avernal/core/assert.hpp>

namespace avernal::render {

std::unique_ptr<Texture> Texture::create(
    Device& device,
    std::uint32_t width,
    std::uint32_t height,
    Format format,
    const void* data) {
    
    AV_ENSURE(width > 0);
    AV_ENSURE(height > 0);

    auto texture = std::make_unique<Texture>();
    texture->width_ = width;
    texture->height_ = height;

    texture->texture_ = device.create_texture({
        .width = width,
        .height = height,
        .format = format,
        .data = data,
    });

    AV_ENSURE(texture->texture_ != nullptr);

    return texture;
}

}  // namespace avernal::render
