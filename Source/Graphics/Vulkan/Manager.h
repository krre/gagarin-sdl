#pragma once
#include "../../Core/Singleton.h"
#include "Instance.h"
#include "DebugReportCallback.h"
#include "PhysicalDevices.h"
#include "Device.h"
#include "Surface.h"
#include "Queue.h"
#include "Swapchain.h"
#include "ImageView.h"
#include "SurfaceFormat.h"
#include "PipelineLayout.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Fence.h"
#include "Semaphore.h"
#include <string>
#include <vector>

const std::string initError = "Vulkan initialization error:\n";

namespace Vulkan {

class Manager : public Singleton<Manager> {

public:
    Manager();
    ~Manager();
    bool init();
    bool createSurface();
    std::string getResultDescription() const { return resultDescription; }
    Vulkan::Instance* getInstance() const { return instance; }
    Vulkan::Device* getDevice() const { return device.get(); }
    Vulkan::Surface* getSurface() const { return surface.get(); }
    void drawFrame();

private:
    std::string resultDescription = "None";

    Vulkan::Instance* instance;
    std::unique_ptr<Vulkan::DebugReportCallback> debugCallback;
    std::unique_ptr<Vulkan::PhysicalDevices> physicalDevices;
    std::unique_ptr<Vulkan::Device> device;
    std::unique_ptr<Vulkan::Surface> surface;
    std::unique_ptr<Vulkan::SurfaceFormat> surfaceFormat;
//    std::unique_ptr<Vulkan::Queue> queue;
    std::unique_ptr<Vulkan::Swapchain> swapchain;
    std::unique_ptr<Vulkan::PipelineLayout> pipelineLayout;
    std::unique_ptr<Vulkan::Pipeline> graphicsPipeline;
    std::unique_ptr<Vulkan::RenderPass> renderPass;
    std::vector<std::shared_ptr<Vulkan::ImageView>> imageViews;
    std::vector<std::shared_ptr<Vulkan::Framebuffer>> framebuffers;
    std::unique_ptr<Vulkan::CommandPool> commandPool;
    std::unique_ptr<Vulkan::CommandBuffer> commandBuffer;
    std::unique_ptr<Vulkan::Fence> fence;
    std::unique_ptr<Vulkan::Semaphore> imageAvailableSemaphore;
    std::unique_ptr<Vulkan::Semaphore> renderFinishedSemaphore;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

} // Vulkan
