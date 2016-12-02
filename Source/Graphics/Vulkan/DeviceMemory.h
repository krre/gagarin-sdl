#pragma once
#include "CreatableObject.h"
#include "Device.h"

namespace Vulkan {

class DeviceMemory : public CreatableObject<VkDeviceMemory> {

public:
    DeviceMemory(const Device* device);
    ~DeviceMemory();
    bool create() override;

private:
    const Device* device;
    VkMemoryAllocateInfo allocateInfo;
};

} // Vulkan
