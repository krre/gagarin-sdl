#include "RenderEngine.h"
#include "Core/Defines.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Core/Screen.h"
#include "Core/Utils.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
#include "Gui/Control.h"
#include "Gui/Batch2D.h"
#include "Debug/DebugEnvironment.h"
#include "Graphics/Render/RenderEngine.h"
#include "Graphics/Vulkan/Command/CommandBuffer.h"
#include "Graphics/Vulkan/Command/CommandBuffers.h"
#include "Graphics/Vulkan/Command/CommandPool.h"
#include "Graphics/Vulkan/Device/Device.h"
#include "Graphics/Vulkan/Device/PhysicalDevice.h"
#include "Graphics/Vulkan/Device/PhysicalDevices.h"
#include "Graphics/Vulkan/Framebuffer.h"
#include "Graphics/Vulkan/Instance.h"
#include "Graphics/Vulkan/Queue.h"
#include "Graphics/Vulkan/RenderPass.h"
#include "Graphics/Vulkan/Semaphore.h"
#include "Graphics/Vulkan/Surface/Swapchain.h"
#include "Graphics/Vulkan/Surface/Surface.h"
#include "Graphics/Vulkan/Instance.h"
#include "Graphics/Vulkan/Surface/Swapchain.h"
#include "Graphics/Vulkan/Device/PhysicalDevice.h"
#include "Graphics/Vulkan/Device/DeviceMemory.h"
#include "Graphics/Vulkan/RenderPass.h"
#include "Graphics/Vulkan/Framebuffer.h"
#include "Graphics/Vulkan/Semaphore.h"
#include "Graphics/Vulkan/Fence.h"
#include "Graphics/Vulkan/Image/Image.h"
#include "Graphics/Vulkan/Image/ImageView.h"
#include "Graphics/Vulkan/Command/CommandBufferOneTime.h"
#include <lodepng/lodepng.h>
#include <SDL_syswm.h>

#if defined(OS_WIN)
    #include "Graphics/Vulkan/Surface/Win32Surface.h"
#elif defined(OS_LINUX)
    #include "Graphics/Vulkan/Surface/XcbSurface.h"
#endif

namespace Origin {

RenderEngine::RenderEngine() {
    window = Application::get()->getWindow();

    createAll();

    renderer2d = std::make_unique<Renderer2D>();
    renderer3d = std::make_unique<Renderer3D>();

    createShaderPrograms();
    createRenderStates();
}

RenderEngine::~RenderEngine() {
    queue->waitIdle();

    if (graphicsDevice) {
        graphicsDevice->waitIdle();
    }
    if (computeDevice) {
        computeDevice->waitIdle();
    }
}

void RenderEngine::render(Screen* screen) {
    renderer2d->prepare(screen->getRootControl());
    renderer2d->render();

    for (auto view3d : renderer2d->getRenderViews()) {
        renderer3d->render(view3d);
    }

    VkResult result = swapchain->acquireNextImage(imageAvailableSemaphore.get());
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        resizeSwapchain();
    }

    queue->clearCommandBuffers();
    queue->addCommandBuffer(commandBuffers.at(swapchain->getImageIndex()).get(),
                            renderFinishedSemaphore.get(),
                            imageAvailableSemaphore.get(), VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
    queue->submit();
    queue->present();
    queue->waitIdle();
}

void RenderEngine::createAll() {
    instance = std::make_unique<Vulkan::Instance>();

    if (DebugEnvironment::get()->getSettings()["vulkan"]["layers"]["use"]) {
        instance->setEnabledLayers(DebugEnvironment::get()->getSettings()["vulkan"]["layers"]["list"]);
    }

    if (DebugEnvironment::get()->getSettings()["vulkan"]["extensions"]["use"]) {
        instance->setEnabledExtensions(DebugEnvironment::get()->getSettings()["vulkan"]["extensions"]["list"]);
    }

    instance->create();

    physicalDevices = std::make_unique<Vulkan::PhysicalDevices>(instance.get());
    physicalDevices->dumpDevices();

    // Create graphics logical device and command pool
    Vulkan::PhysicalDevice* gpd = physicalDevices->findDevice(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    if (gpd == nullptr) {
        gpd = physicalDevices->findDevice(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    }

    graphicsDevice = std::make_unique<Vulkan::Device>(gpd);
    graphicsDevice->addQueueCreateInfo(graphicsFamily, { 1.0 });
    graphicsDevice->create();

    graphicsFamily = gpd->findQueueFamily(VK_QUEUE_GRAPHICS_BIT);

    graphicsCommandPool = std::make_shared<Vulkan::CommandPool>(graphicsDevice.get(), graphicsFamily);
    graphicsCommandPool->create();

    // Create compute logical device and command pool
    Vulkan::PhysicalDevice* cpd = physicalDevices->findDevice(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    if (cpd == nullptr) {
        cpd = physicalDevices->findDevice(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    }

    computeDevice = std::make_unique<Vulkan::Device>(gpd);
    computeDevice->addQueueCreateInfo(computeFamily, { 1.0 });
    computeDevice->create();

    computeFamily = gpd->findQueueFamily(VK_QUEUE_COMPUTE_BIT);

    computeCommandPool = std::make_shared<Vulkan::CommandPool>(computeDevice.get(), computeFamily);
    computeCommandPool->create();

    device = RenderEngine::get()->getGraphicsDevice();

    imageAvailableSemaphore = std::make_unique<Vulkan::Semaphore>(device);
    imageAvailableSemaphore->create();

    renderFinishedSemaphore = std::make_unique<Vulkan::Semaphore>(device);
    renderFinishedSemaphore->create();

    queue = std::make_unique<Vulkan::Queue>(device, graphicsFamily);
    queue->create();
    queue->addPresentWaitSemaphore(renderFinishedSemaphore.get());

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    SDL_GetWindowWMInfo(window->getHandle(), &wminfo);

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

    resizeSwapchain();
}

void RenderEngine::createShaderPrograms() {
//    shaderPrograms[ShaderProgram::ProgamType::Base] = RenderContext::get()->createShaderProgram("Base");
//    shaderPrograms[ShaderProgram::ProgamType::Sdf] = RenderContext::get()->createShaderProgram("Sdf");
//    shaderPrograms[ShaderProgram::ProgamType::Voxel] = RenderContext::get()->createShaderProgram("Voxel");
}

void RenderEngine::createRenderStates() {
//    // Shape2D
//    std::shared_ptr<RenderState> shape2DRS = RenderContext::get()->createRenderState();
//    shape2DRS->setShaderProgram(shaderPrograms[ShaderProgram::ProgamType::Base].get());
//    shape2DRS->setBlendEnable(true);
//    shape2DRS->create();
//    renderStates[RenderState::Type::Shape2D] = shape2DRS;

//    // Freetype text
//    std::shared_ptr<RenderState> freeTypeTextRS = RenderContext::get()->createRenderState();
//    shape2DRS->setBlendEnable(true);
//    shape2DRS->create();
//    renderStates[RenderState::Type::FreeTypeText] = freeTypeTextRS;

//    // SDF text
//    std::shared_ptr<RenderState> sdfTextRS = RenderContext::get()->createRenderState();
//    sdfTextRS->setShaderProgram(shaderPrograms[ShaderProgram::ProgamType::Sdf].get());
//    shape2DRS->setBlendEnable(true);
//    shape2DRS->create();
//    renderStates[RenderState::Type::SdfText] = sdfTextRS;

//    // Voxel
//    std::shared_ptr<RenderState> voxelRS = RenderContext::get()->createRenderState();
//    voxelRS->setShaderProgram(shaderPrograms[ShaderProgram::ProgamType::Voxel].get());
//    voxelRS->create();
//    renderStates[RenderState::Type::Voxel] = voxelRS;
}

void RenderEngine::updateCommandBuffers() {
    const Color& color = window->getColor();
    renderPass->setClearValue({ color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha() });

    if (!commandBuffers.size()) {
        commandBufferHandlers = std::make_unique<Vulkan::CommandBuffers>(device, graphicsCommandPool.get());
        commandBufferHandlers->allocate(swapchain->getCount());

        for (int i = 0; i < commandBufferHandlers->getCount(); i++) {
            auto commandBuffer = std::make_unique<Vulkan::CommandBuffer>(commandBufferHandlers->at(i));
            commandBuffers.push_back(std::move(commandBuffer));
        }
    }

    for (int i = 0; i < commandBuffers.size(); i++) {
        renderPass->setFramebuffer(framebuffers.at(i)->getHandle());

        Vulkan::CommandBuffer* commandBuffer = commandBuffers.at(i).get();
        commandBuffer->reset();
        commandBuffer->begin();

        VkExtent2D extent = { window->getWidth(), window->getHeight() };

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

void RenderEngine::resizeSwapchain() {
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

    updateCommandBuffers();
}

void RenderEngine::saveScreenshot() {
    std::string directoryPath = Application::getCurrentDirectory() + Utils::getPathSeparator() + "Screenshot";
    namespace fs = std::experimental::filesystem;
    if (!fs::exists(directoryPath)) {
        fs::create_directory(directoryPath);
    }

    time_t t = std::time(0); // Get time now
    struct tm* now = std::localtime(&t);
    std::string filename =
            std::to_string(now->tm_year + 1900) + "-" +
            Utils::zeroFill(std::to_string(now->tm_mon + 1)) + "-" +
            Utils::zeroFill(std::to_string(now->tm_mday)) + "_" +
            Utils::zeroFill(std::to_string(now->tm_hour)) + "-" +
            Utils::zeroFill(std::to_string(now->tm_min)) + "-" +
            Utils::zeroFill(std::to_string(now->tm_sec)) + ".png";
    std::string filePath = directoryPath + Utils::getPathSeparator() + filename;

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

    if (supportsBlit) {
        lodepng::encode(filePath, data, width, height);
    } else {
        std::vector<unsigned char> output;
        output.resize(width * height * 4);
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

        lodepng::encode(filePath, output.data(), width, height);
    }

    image.getMemory()->unmap();

    std::string message = "Screenshot saved to " + filename;
//    Toast::get()->showToast(message);
    PRINT(message)
}

} // Origin
