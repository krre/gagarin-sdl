#include "StorageBuffer.h"
#include "Graphics/Render/Vulkan/Wrapper/Buffer/Buffer.h"

StorageBuffer::StorageBuffer(uint64_t size) {
    buffer = std::make_unique<Vulkan::Buffer>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, size, false);
    buffer->create();
}
