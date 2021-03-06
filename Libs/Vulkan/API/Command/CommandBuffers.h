#pragma once
#include "../Base/Collection.h"
#include "../Device/Devicer.h"

namespace Vulkan {

class CommandPool;

class CommandBuffers : public Collection<VkCommandBuffer>, public Devicer {

public:
    CommandBuffers(Device* device, CommandPool* commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    ~CommandBuffers();
    void allocate(uint32_t count);
    void destroy() override;

private:
    VkCommandBufferAllocateInfo allocateInfo = {};
    CommandPool* commandPool;
};

} // Vulkan
