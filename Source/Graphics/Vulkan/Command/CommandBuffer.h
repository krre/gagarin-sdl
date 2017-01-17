#pragma once
#include "../Base/Handle.h"
#include "../Device/Device.h"
#include "CommandPool.h"
#include "../Pipeline/PipelineBarrier.h"
#include <vector>

namespace Vulkan {

class CommandBuffer : public Handle<VkCommandBuffer> {

public:
    CommandBuffer(VkCommandBuffer handle);
    VkResult create() override {}
    void setFlags(VkCommandBufferUsageFlags flags);
    void addViewport(VkViewport viewport);

    VkResult begin();
    VkResult end();
    void pipelineBarrier(PipelineBarrier* pipelineBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags = 0);

private:
    VkCommandBufferBeginInfo beginInfo = {};
    std::vector<VkViewport> viewports;
};

} // Vulkan
