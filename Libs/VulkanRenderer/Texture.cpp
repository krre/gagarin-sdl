#include "Texture.h"
#include <lodepng/lodepng.h>
#include "Graphics/Render/RenderEngine.h"
#include "Vulkan/Instance.h"
#include "Vulkan/Command/CommandBufferOneTime.h"
#include "Vulkan/Fence.h"
#include "Vulkan/Queue.h"
#include "Vulkan/Image/ImageView.h"
#include "Vulkan/Image/Image.h"
#include "Vulkan/Device/PhysicalDevice.h"

namespace Origin {

Texture::Texture(const std::string& path) {
    uint32_t width;
    uint32_t height;
    unsigned result = lodepng::decode(data, width, height, path);
    if (result) {
        throw std::runtime_error("Failed to decode image " + path);
    }

    image = std::make_unique<Vulkan::Image>(RenderEngine::get()->getGraphicsDevice());

    image->setWidth(width);
    image->setHeight(height);
    image->setFormat(VK_FORMAT_R8G8B8A8_UNORM);
    image->setUsage(VK_IMAGE_USAGE_SAMPLED_BIT);
    image->setInitialLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
    image->create();

    image->write(data.data(), data.size());

    imageView = std::make_unique<Vulkan::ImageView>(RenderEngine::get()->getGraphicsDevice(), image->getHandle());
    imageView->setFormat(image->getFormat());
    imageView->create();

    VkFormatProperties formatProps;
//    vkGetPhysicalDeviceFormatProperties(RenderEngine::get()->getGraphicsDevice()->getPhysicalDevice()->getHandle(), VK_FORMAT_R8G8B8A8_UNORM, &formatProps);

    /* See if we can use a linear tiled image for a texture, if not, we will
     * need a staging image for the texture data */
    bool needStaging = (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) ? true : false;
    if (needStaging) {

    }

    Vulkan::CommandBufferOneTime commandBuffer(RenderEngine::get()->getGraphicsDevice(), RenderEngine::get()->getGraphicsCommandPool());
    commandBuffer.setImageLayout(image->getHandle(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    commandBuffer.apply();
}

Texture::~Texture() {

}

} // Origin