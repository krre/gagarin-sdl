#include "Renderer.h"
#include "API/Command/CommandBuffer.h"
#include "API/Command/CommandBuffers.h"
#include "API/Command/CommandPool.h"
#include "API/Device/Device.h"
#include "API/Device/PhysicalDevice.h"
#include "API/Device/PhysicalDevices.h"
#include "API/Framebuffer.h"
#include "API/Instance.h"
#include "API/Queue.h"
#include "API/RenderPass.h"
#include "API/Semaphore.h"
#include "API/Surface/Swapchain.h"
#include "API/Surface/Surface.h"
#include "API/Instance.h"
#include "API/Surface/Swapchain.h"
#include "API/Device/PhysicalDevice.h"
#include "API/Device/DeviceMemory.h"
#include "API/RenderPass.h"
#include "API/Framebuffer.h"
#include "API/Semaphore.h"
#include "API/Fence.h"
#include "API/Image/Image.h"
#include "API/Image/ImageView.h"
#include "API/Command/CommandBufferOneTime.h"

#if defined(_WIN32)
    #include "API/Surface/Win32Surface.h"
#elif defined(__linux__)
    #include "API/Surface/XcbSurface.h"
#endif

namespace Vulkan {

Renderer::Renderer() {
    createAll();
}

Renderer::~Renderer() {
    queue->waitIdle();

    if (graphicsDevice) {
        graphicsDevice->waitIdle();
    }
    if (computeDevice) {
        computeDevice->waitIdle();
    }
}

void Renderer::create() {

}

void Renderer::render() {
    VkResult result = swapchain->acquireNextImage(imageAvailableSemaphore.get());
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        resize();
    }

    queue->clearCommandBuffers();
    queue->addCommandBuffer(commandBuffers.at(swapchain->getImageIndex()).get(),
                            renderFinishedSemaphore.get(),
                            imageAvailableSemaphore.get(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
    queue->submit();
    queue->present();
//    queue->waitIdle();
}

void Renderer::createAll() {
    instance = std::make_unique<Vulkan::Instance>();

    if (DebugEnvironment::get()->getEnable()) {
        if (DebugEnvironment::get()->getSettings()["vulkan"]["layers"]["use"]) {
            instance->setEnabledLayers(DebugEnvironment::get()->getSettings()["vulkan"]["layers"]["list"]);
        }

        if (DebugEnvironment::get()->getSettings()["vulkan"]["extensions"]["use"]) {
            instance->setEnabledExtensions(DebugEnvironment::get()->getSettings()["vulkan"]["extensions"]["list"]);
        }
    }

//    instance->setUseDebugCallback(true); // TODO: Take from debug settings
    instance->create();

    physicalDevices = std::make_unique<Vulkan::PhysicalDevices>(instance.get());

    // Default graphics and compute device are same
    Vulkan::PhysicalDevice* graphicsPhysicalDevice = physicalDevices->getPhysicalDevice(0);
    Vulkan::PhysicalDevice* computePhysicalDevice = physicalDevices->getPhysicalDevice(0);

    if (physicalDevices->getCount() > 1) {
        if (DebugEnvironment::get()->getEnable()) {
            // Take from debug settings
            int index = DebugEnvironment::get()->getVulanDevice();
            graphicsPhysicalDevice = physicalDevices->getPhysicalDevice(index);
            computePhysicalDevice = physicalDevices->getPhysicalDevice(1 - index);
        } else {
            // Select by hardware properties
            graphicsPhysicalDevice = physicalDevices->findDevice(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
            if (graphicsPhysicalDevice == nullptr) {
                graphicsPhysicalDevice = physicalDevices->findDevice(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
            }

            computePhysicalDevice = physicalDevices->findDevice(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
            if (computePhysicalDevice == nullptr) {
                computePhysicalDevice = physicalDevices->findDevice(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
            }
        }
    }

    // Create graphics logical device and command pool
    graphicsFamily = graphicsPhysicalDevice->findQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    graphicsDevice = std::make_unique<Vulkan::Device>(graphicsPhysicalDevice);
    graphicsDevice->addQueueCreateInfo(graphicsFamily, { 1.0 });
    graphicsDevice->create();

    graphicsCommandPool = std::make_shared<Vulkan::CommandPool>(graphicsDevice.get(), graphicsFamily);
    graphicsCommandPool->create();

    // Create compute logical device and command pool
    if (graphicsPhysicalDevice != computePhysicalDevice) {
        computeFamily = computePhysicalDevice->findQueueFamily(VK_QUEUE_COMPUTE_BIT);
        computeDevice = std::make_unique<Vulkan::Device>(computePhysicalDevice);
        computeDevice->addQueueCreateInfo(computeFamily, { 1.0 });
        computeDevice->create();

        computeCommandPool = std::make_shared<Vulkan::CommandPool>(computeDevice.get(), computeFamily);
        computeCommandPool->create();
    }

    device = graphicsDevice.get();

    imageAvailableSemaphore = std::make_unique<Vulkan::Semaphore>(device);
    imageAvailableSemaphore->create();

    renderFinishedSemaphore = std::make_unique<Vulkan::Semaphore>(device);
    renderFinishedSemaphore->create();

    queue = std::make_unique<Vulkan::Queue>(device, graphicsFamily);
    queue->create();
    queue->addPresentWaitSemaphore(renderFinishedSemaphore.get());

#if defined(OS_WIN)
    surface = std::make_unique<Vulkan::Win32Surface>(instance.get(), device->getPhysicalDevice(), GetModuleHandle(nullptr), wminfo.info.win.window);
#elif defined(OS_LINUX)
    surface = std::make_unique<Vulkan::XcbSurface>(instance.get(), device->getPhysicalDevice(), XGetXCBConnection(wminfo.info.x11.display), wminfo.info.x11.window);
#endif

    surface->create();

    renderPass = std::make_unique<Vulkan::RenderPass>(device);
    renderPass->setColorFormat(surface->getFormats().at(0).format);
    renderPass->create();

    swapchain = std::make_unique<Vulkan::Swapchain>(device, surface.get());

    resize();
}

void Renderer::writeCommandBuffers(Vulkan::CommandBuffer* commandBuffer) {
    const Color& color = window->getColor();
    renderPass->setClearValue({ color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha() });

    for (int i = 0; i < commandBuffers.size(); i++) {
        renderPass->setFramebuffer(framebuffers.at(i)->getHandle());

        Vulkan::CommandBuffer* commandBuffer = commandBuffers.at(i).get();
        commandBuffer->reset();
        commandBuffer->begin();

        VkExtent2D extent = { surface->getCurrentExtent().width, surface->getCurrentExtent().height };

        VkViewport viewport = {};
        viewport.width = extent.width;
        viewport.height = extent.height;
        viewport.maxDepth = 1.0;

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;

        commandBuffer->beginRenderPass(renderPass->getBeginInfo());

        //    commandBuffer->bindPipeline(shaderProgram.getGraphicsPipeline());

        commandBuffer->addViewport(viewport);
        commandBuffer->setViewport(0);

        commandBuffer->addScissor(scissor);
        commandBuffer->setScissor(0);

        //    commandBuffer->addVertexBuffer(vertexBuffer->getHandle());
        //    commandBuffer->bindVertexBuffers();
        //    commandBuffer->bindIndexBuffer(indexBuffer->getHandle(), indexBuffer->getIndexType());

        //    for (int i = 0; i < shaderProgram.getDescriptorSets()->getCount(); i++) {
        //        commandBuffer->addDescriptorSet(shaderProgram.getDescriptorSets()->at(i));
        //    }
        //    commandBuffer->bindDescriptorSets(shaderProgram.getGraphicsPipeline()->getBindPoint(), shaderProgram.getPipelineLayout()->getHandle());
        //    commandBuffer->drawIndexed(MAX_CHAR_COUNT, 1, 0, 0, 0);

        commandBuffer->endRenderPass();

        // TODO: Write commands to commandBuffer

        commandBuffer->end();
    }
}

void Renderer::resize() {
    uint32_t width = surface->getCurrentExtent().width;
    uint32_t height = surface->getCurrentExtent().height;
    renderPass->setExtent({ width, height });

    swapchain->destroy();
    swapchain->create();
    queue->clearSwapchains();
    queue->addSwapchain(swapchain.get());

    imageViews.clear();
    framebuffers.clear();

    for (const auto& image : swapchain->getImages()) {
        std::unique_ptr<Vulkan::ImageView> imageView = std::make_unique<Vulkan::ImageView>(device, image);
        imageView->setFormat(surface->getFormats().at(0).format);
        imageView->create();

        std::unique_ptr<Vulkan::Framebuffer> framebuffer = std::make_unique<Vulkan::Framebuffer>(device);
        framebuffer->addAttachment(imageView.get());
        framebuffer->setRenderPass(renderPass.get());
        framebuffer->setWidth(width);
        framebuffer->setHeight(height);
        framebuffer->create();

        imageViews.push_back(std::move(imageView));
        framebuffers.push_back(std::move(framebuffer));
    }

    if (!commandBuffers.size()) {
        commandBufferHandlers = std::make_unique<Vulkan::CommandBuffers>(device, graphicsCommandPool.get());
        commandBufferHandlers->allocate(swapchain->getCount());

        for (int i = 0; i < commandBufferHandlers->getCount(); i++) {
            auto commandBuffer = std::make_unique<Vulkan::CommandBuffer>(commandBufferHandlers->at(i));
            writeCommandBuffers(commandBuffer.get());
            commandBuffers.push_back(std::move(commandBuffer));
        }
    }
}

std::vector<unsigned char> Renderer::readFramebuffer() {
    VkImage srcImage = swapchain->getCurrentImage();

    uint32_t width = surface->getCurrentExtent().width;
    uint32_t height = surface->getCurrentExtent().height;

    Vulkan::Image image(device);
    image.setWidth(width);
    image.setHeight(height);
    image.create();
    VkImage dstImage = image.getHandle();

    Vulkan::CommandBufferOneTime commandBuffer(device, graphicsCommandPool.get());
    commandBuffer.setImageLayout(dstImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    commandBuffer.setImageLayout(srcImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    bool supportsBlit = device->getPhysicalDevice()->getSupportBlit(swapchain->getImageFormat());
    if (supportsBlit) {
        VkOffset3D blitSize;
        blitSize.x = width;
        blitSize.y = height;
        blitSize.z = 1;

        VkImageBlit imageBlitRegion = {};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;

        commandBuffer.addBlitRegion(imageBlitRegion);
        commandBuffer.blitImage(srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    } else {
        VkImageCopy imageCopy;
        imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.srcSubresource.mipLevel = 0;
        imageCopy.srcSubresource.baseArrayLayer = 0;
        imageCopy.srcSubresource.layerCount = 1;
        imageCopy.srcOffset = {};
        imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.dstSubresource.mipLevel = 0;
        imageCopy.dstSubresource.baseArrayLayer = 0;
        imageCopy.dstSubresource.layerCount = 1;
        imageCopy.dstOffset = {};
        imageCopy.extent.width = width;
        imageCopy.extent.height = height;
        imageCopy.extent.depth = 1;

        commandBuffer.addImageCopy(imageCopy);
        commandBuffer.copyImage(srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    }

    commandBuffer.setImageLayout(dstImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT);
    commandBuffer.apply();

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource = {};
    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(device->getHandle(), dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const unsigned char* data;
    image.getMemory()->map((void**)&data, VK_WHOLE_SIZE);
    data += subResourceLayout.offset;

    std::vector<unsigned char> output;
    output.resize(width * height * 4);

    if (supportsBlit) {

    } else {
        // Convert from BGR to RGB
        uint32_t offset = 0;
        for (uint32_t y = 0; y < height; y++) {
            unsigned int *row = (unsigned int*)data;
            for (uint32_t x = 0; x < width; x++) {
                output[offset++] = *((char*)row + 2);
                output[offset++] = *((char*)row + 1);
                output[offset++] = *((char*)row);
                output[offset++] = *((char*)row + 3);

                row++;
            }

            data += subResourceLayout.rowPitch;
        }
    }

//    lodepng::encode(filePath, output.data(), width, height);

    image.getMemory()->unmap();

    return std::move(output);
}

} // Vulkan
