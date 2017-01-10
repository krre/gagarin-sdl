#include "PipelineBarrier.h"

using namespace Vulkan;

PipelineBarrier::PipelineBarrier() {

}

void PipelineBarrier::addMemoryBarrier(VkMemoryBarrier memoryBarrier) {
    memoryBarriers.push_back(memoryBarrier);
}

void PipelineBarrier::addImageMemoryBarrier(VkImageMemoryBarrier imageMemoryBarrier) {
    imageMemoryBarriers.push_back(imageMemoryBarrier);
}

VkImageMemoryBarrier PipelineBarrier::createImageMemoryBarrier() {
    VkImageSubresourceRange imageSubresourceRange = {};
    imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageSubresourceRange.baseMipLevel = 0;
    imageSubresourceRange.levelCount = 1;
    imageSubresourceRange.layerCount = 1;

    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange = imageSubresourceRange;

    return imageMemoryBarrier;
}
