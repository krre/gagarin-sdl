#pragma once
#include "Base/Handle.h"
#include "Device/Devicer.h"
#include "Device/DeviceMemory.h"

namespace Vulkan {

class Buffer : public Handle<VkBuffer>, public Devicer {

public:

    Buffer(VkBufferUsageFlagBits usage, VkDeviceSize size, bool moveToDevice = true, Device* device = nullptr);
    ~Buffer();
    void create() override;
    void destroy() override;
    VkDeviceSize getSize() const { return createInfo.size; }
    void write(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    void read(void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    void copyToBuffer(VkBuffer dstBuffer, VkDeviceSize size);

    VkDescriptorBufferInfo descriptorInfo = {};

private:
    VkBufferCreateInfo createInfo = {};
    DeviceMemory memory;
    bool moveToDevice;
};

} // Vulkan
