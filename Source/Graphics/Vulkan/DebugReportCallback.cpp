#include "DebugReportCallback.h"

using namespace Vulkan;

DebugReportCallback::DebugReportCallback(VkInstance instance, PFN_vkDebugReportCallbackEXT debugCallback) : instance(instance) {
    pfnCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (!pfnCreateDebugReportCallback) {
        throw std::runtime_error("GetInstanceProcAddr: Unable to find vkCreateDebugReportCallbackEXT function");
    }

    pfnDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (!pfnDestroyDebugReportCallback) {
        throw std::runtime_error("GetInstanceProcAddr: Unable to find vkDestroyDebugReportCallbackEXT function");
    }

    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

}

DebugReportCallback::~DebugReportCallback() {
    destroy();
}

void DebugReportCallback::create() {
    VULKAN_CHECK_RESULT(pfnCreateDebugReportCallback(instance, &createInfo, nullptr, &handle), "Failed to create debug report callback");
}

void DebugReportCallback::destroy() {
    pfnDestroyDebugReportCallback(instance, handle, nullptr);
}
