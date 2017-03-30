#include "MenuScene.h"
#include "../../Core/App.h"
#include "../../Scene/SceneManager.h"
#include "../../Event/Input.h"
#include "../../Graphics/Vulkan/Manager.h"
#include "../../Resource/ShaderResource.h"
#include "../../Resource/ResourceManager.h"
#include "../../Graphics/Vulkan/Command/CommandBuffer.h"
#include "../../Graphics/Plane.h"

MenuScene::MenuScene() :
    bsp(device) {
}

MenuScene::~MenuScene() {

}

void MenuScene::init() {
    Scene::init();

    Vulkan::GraphicsPipeline* graphicsPipeline = bsp.getGraphicsPipeline();

    vertexBuffer = std::make_shared<Vulkan::Buffer>(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, plane.getVerticesSize());
    vertexBuffer->create();

    Vulkan::Buffer vertexStageBuffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, plane.getVerticesSize());
    vertexStageBuffer.create();
    vertexStageBuffer.write(plane.getVertices().data(), plane.getVerticesSize());
    vertexStageBuffer.copyToBuffer(vertexBuffer->getHandle(), plane.getVerticesSize());

    indexBuffer = std::make_shared<Vulkan::Buffer>(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, plane.getIndicesSize());
    indexBuffer->create();

    Vulkan::Buffer indexStageBuffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, plane.getIndicesSize());
    indexStageBuffer.create();
    indexStageBuffer.write(plane.getIndices().data(), plane.getIndicesSize());
    indexStageBuffer.copyToBuffer(indexBuffer->getHandle(), plane.getIndicesSize());

    bsp.write("uboVert");
    bsp.write("uboFrag");

    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(glm::vec2);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    graphicsPipeline->addVertexBindingDescription(bindingDescription);

    VkVertexInputAttributeDescription attributeDescriptions = {};
    attributeDescriptions.binding = 0;
    attributeDescriptions.location = 0;
    attributeDescriptions.format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions.offset = 0;
    graphicsPipeline->addVertexAttributeDescription(attributeDescriptions);

    graphicsPipeline->setExtent(Vulkan::Manager::get()->getSurface()->getCapabilities().currentExtent);
    graphicsPipeline->setRenderPass(Vulkan::Manager::get()->getRenderPass()->getHandle());
    graphicsPipeline->create();

    buildCommandBuffers();
}

void MenuScene::draw(float dt) {
    queue->submit();
}

void MenuScene::update(float dt) {

}

void MenuScene::create() {

}

void MenuScene::onKeyPressed(const SDL_KeyboardEvent& event) {
    if (event.keysym.sym == SDLK_ESCAPE) {
        SceneManager::get()->popScene();
        Input::get()->isKeyAccepted = true;
    }
}

void MenuScene::buildCommandBuffers() {
    Vulkan::Manager::get()->getRenderPass()->setClearValue({ 0.77, 0.83, 0.83, 1.0 });
    VkRenderPassBeginInfo* renderPassBeginInfo = &Vulkan::Manager::get()->getRenderPass()->beginInfo;
    queue->clearCommandBuffers();

    for (size_t i = 0; i < commandBuffers.getCount(); i++) {
        renderPassBeginInfo->framebuffer = Vulkan::Manager::get()->getFramebuffer(i)->getHandle();

        Vulkan::CommandBuffer commandBuffer(commandBuffers.at(i));
        commandBuffer.begin();
        commandBuffer.beginRenderPass(renderPassBeginInfo);
        commandBuffer.bindPipeline(bsp.getGraphicsPipeline());

        commandBuffer.addVertexBuffer(vertexBuffer->getHandle());
        commandBuffer.bindVertexBuffers();
        commandBuffer.bindIndexBuffer(indexBuffer->getHandle(), VK_INDEX_TYPE_UINT16);

        Vulkan::DescriptorSets* descriptorSets = &bsp.descriptorSets;
        for (int i = 0; i < descriptorSets->getCount(); i++) {
            commandBuffer.addDescriptorSet(descriptorSets->at(i));
        }
        commandBuffer.bindDescriptorSets(bsp.getGraphicsPipeline(), bsp.getPipelineLayout()->getHandle());
        commandBuffer.drawIndexed(plane.getIndices().size(), 1, 0, 0, 0);

        commandBuffer.endRenderPass();
        commandBuffer.end();

        queue->addCommandBuffer(commandBuffer.getHandle());
    }
}
