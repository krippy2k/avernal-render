#include <avernal/render/material.hpp>
#include <avernal/core/assert.hpp>

namespace avernal::render {

std::unique_ptr<Material> Material::create(Device& device, const GraphicsPipelineDesc& desc) {
    auto material = std::make_unique<Material>();
    material->pipeline_ = device.create_graphics_pipeline(desc);
    AV_ENSURE(material->pipeline_ != nullptr);
    return material;
}

}  // namespace avernal::render
