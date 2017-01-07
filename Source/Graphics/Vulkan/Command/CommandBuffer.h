#pragma once
#include "../Base/Handle.h"
#include "../Device/Device.h"
#include "CommandPool.h"

namespace Vulkan {

class CommandBuffer : public Handle<VkCommandBuffer> {

public:
    CommandBuffer(VkCommandBuffer handle);
    VkResult create() override {}
    void setFlags(VkCommandBufferUsageFlags flags);
    VkResult begin();
    VkResult end();

private:
    VkCommandBufferBeginInfo beginInfo = {};

};

} // Vulkan
