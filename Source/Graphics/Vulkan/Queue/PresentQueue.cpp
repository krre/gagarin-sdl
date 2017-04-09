#include "PresentQueue.h"

using namespace Vulkan;

PresentQueue::PresentQueue(const Device* device, uint32_t queueFamilyIndex, uint32_t queueIndex) :
    Queue(device, queueFamilyIndex, queueIndex) {
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
}

VkResult PresentQueue::present() {
    assert(!swapchains.empty());
    presentInfo.waitSemaphoreCount = waitSemaphores.size();
    presentInfo.pWaitSemaphores = waitSemaphores.data();
    presentInfo.swapchainCount = swapchains.size();
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = imageIndices.data();
    VULKAN_CHECK_RESULT(vkQueuePresentKHR(handle, &presentInfo), "Failed to present swapchain image");
}

void PresentQueue::addSwapchain(VkSwapchainKHR swapchain) {
    swapchains.push_back(swapchain);
    imageIndices.resize(swapchains.size());
}

void PresentQueue::clearSwapchain() {
    swapchains.clear();
    imageIndices.clear();
}

uint32_t* PresentQueue::getImageIndex(int i) {
    assert(!imageIndices.empty() && i < imageIndices.size());
    return &imageIndices[i];
}
