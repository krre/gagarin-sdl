#pragma once
#include "Queue.h"
#include <vector>

namespace Vulkan {

class PresentQueue : public Queue {

public:
    PresentQueue(uint32_t queueFamilyIndex, uint32_t queueIndex = 0, Device* device = nullptr);
    VkResult present();
    void addSwapchain(VkSwapchainKHR swapchain);
    void clearSwapchain();
    uint32_t* getImageIndex(int i);

    VkPresentInfoKHR presentInfo = {};

private:
    std::vector<VkSwapchainKHR> swapchains;
    std::vector<uint32_t> imageIndices;
};

} // Vulkan
