#pragma once
#include "../Base/Handle.h"
#include "../Device/Devicer.h"

namespace Vulkan {

class Buffer : public Handle<VkBuffer>, public Devicer {

public:
    Buffer(Device* device, VkBufferUsageFlagBits usage, VkDeviceSize size);
    ~Buffer();
    void create() override;
    void destroy() override;
    VkDeviceSize getSize() const { return createInfo.size; }
    const VkDescriptorBufferInfo* getDescriptorInfo() const { return &descriptorInfo; }

private:
    VkBufferCreateInfo createInfo = {};
    VkDescriptorBufferInfo descriptorInfo = {};
};

} // Vulkan
