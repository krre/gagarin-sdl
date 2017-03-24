#include "DeviceMemory.h"

using namespace Vulkan;

DeviceMemory::DeviceMemory(const Device* device) : Devicer(device) {
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
}

DeviceMemory::~DeviceMemory() {
    destroy();
}

void DeviceMemory::destroy() {
    VULKAN_DESTROY_HANDLE(vkFreeMemory(device->getHandle(), handle, nullptr))
}

VkResult DeviceMemory::allocate(VkDeviceSize size) {
    allocateInfo.allocationSize = size;
    CHECK_RESULT(vkAllocateMemory(device->getHandle(), &allocateInfo, nullptr, &handle), "Failed to allocate memory");
}

void DeviceMemory::setMemoryTypeIndex(uint32_t index) {
    allocateInfo.memoryTypeIndex = index;
}

VkResult DeviceMemory::map(void** data, VkDeviceSize size, VkDeviceSize offset) {
    CHECK_RESULT(vkMapMemory(device->getHandle(), handle, offset, size, 0, data), "Failed to map device memory");
}

void DeviceMemory::unmap() {
    vkUnmapMemory(device->getHandle(), handle);
}
