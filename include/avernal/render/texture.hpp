#pragma once

#include <avernal/rhi/rhi.hpp>
#include <cstdint>
#include <memory>

namespace avernal::render {

/// Texture wraps an RHI texture resource
class Texture {
public:
    Texture() = default;
    ~Texture() = default;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept = default;
    Texture& operator=(Texture&&) noexcept = default;

    /// Create a texture from image data
    static std::unique_ptr<Texture> create(
        Device& device,
        std::uint32_t width,
        std::uint32_t height,
        Format format,
        const void* data);

    /// Get the underlying RHI texture
    [[nodiscard]] avernal::Texture* rhi_texture() const noexcept { return texture_.get(); }

    /// Get the width
    [[nodiscard]] std::uint32_t width() const noexcept { return width_; }

    /// Get the height
    [[nodiscard]] std::uint32_t height() const noexcept { return height_; }

private:
    std::unique_ptr<avernal::Texture> texture_;
    std::uint32_t width_{};
    std::uint32_t height_{};
};

}  // namespace avernal::render
