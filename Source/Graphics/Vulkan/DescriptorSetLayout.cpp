#include "DescriptorSetLayout.h"

using namespace Vulkan;

DescriptorSetLayout::DescriptorSetLayout(const Device* device) : device(device) {
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(device->getHandle(), handle, nullptr);
}

VkResult DescriptorSetLayout::create() {
    return checkError(vkCreateDescriptorSetLayout(device->getHandle(), &createInfo, nullptr, &handle), "Failed to create descriptor set layout");
}
