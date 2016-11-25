#include "Pipeline.h"

using namespace Vulkan;

Pipeline::Pipeline(PipelineType type, const Device* device) :
    type(type),
    device(device) {

}

Pipeline::~Pipeline() {
    if (handle != VK_NULL_HANDLE) {
        vkDestroyPipeline(device->getHandle(), handle, nullptr);
    }
}