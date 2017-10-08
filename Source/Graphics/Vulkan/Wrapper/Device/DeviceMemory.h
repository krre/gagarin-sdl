#pragma once
#include "Graphics/Vulkan/Wrapper/Base/Handle.h"
#include "Devicer.h"

namespace Vulkan {

class DeviceMemory : public Handle<VkDeviceMemory>, public Devicer {

public:
    DeviceMemory(Device* device = nullptr);
    ~DeviceMemory();
    void create() override {}
    void destroy() override;
    void allocate(VkDeviceSize size);
    void setMemoryTypeIndex(uint32_t index);
    VkDeviceSize getSize() const { return allocateInfo.allocationSize; }
    void map(void** data, VkDeviceSize size, VkDeviceSize offset = 0);
    void unmap();

private:
    VkMemoryAllocateInfo allocateInfo = {};
};

} // Vulkan