#include "PolygonalOctreeRenderer.h"
#include "Base/Window.h"
#include "Vulkan/API/RenderPass.h"
#include "Vulkan/API/Framebuffer.h"
#include "Vulkan/API/Image/ImageView.h"
#include "Vulkan/API/Command/CommandBuffer.h"
#include "Vulkan/API/Surface/Surface.h"
#include "Vulkan/API/RenderPass.h"
#include "Vulkan/API/Descriptor/DescriptorSets.h"
#include "Vulkan/API/Pipeline/PipelineLayout.h"
#include "Graphics/Render/RenderManager.h"
#include "Vulkan/ShaderProgram.h"
#include "Vulkan/GpuBuffer.h"
#include "Vulkan/API/Pipeline/GraphicsPipeline.h"
#include "Resource/ResourceManager.h"
#include "Octree/Octree.h"

namespace Origin {

PolygonalOctreeRenderer::PolygonalOctreeRenderer(Object* parent) : OctreeRenderer(parent) {
    Vulkan::Device* device = getDevice();

    uint32_t startSize = 1000000; // TODO: Set optimal value or take from constant
    vertexBuffer = std::make_unique<Vulkan::GpuBuffer>(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, startSize);

    uboBuffer = std::make_unique<Vulkan::GpuBuffer>(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4));

    shaderProgram = std::make_unique<Vulkan::ShaderProgram>(device);
    shaderProgram->loadShader(ResourceManager::get()->getDataPath() + "/Shader/PolygonalOctree.vert.spv");
    shaderProgram->loadShader(ResourceManager::get()->getDataPath() + "/Shader/PolygonalOctree.frag.spv");

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uboBuffer->getHandle();
    bufferInfo.range = VK_WHOLE_SIZE;
    shaderProgram->bindBuffer("ubo", bufferInfo);

    shaderProgram->create();

    graphicsPipeline = std::make_unique<Vulkan::GraphicsPipeline>(device);
    graphicsPipeline->setRenderPass(RenderManager::get()->getRenderPass()->getHandle());
    graphicsPipeline->setPipelineLayout(shaderProgram->getPipelineLayout()->getHandle());

    graphicsPipeline->addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
    graphicsPipeline->addDynamicState(VK_DYNAMIC_STATE_SCISSOR);

    for (auto& shader : shaderProgram->getShaders()) {
        graphicsPipeline->addShaderCode(shader->getStage(), shader->getCode().size() * sizeof(uint32_t), shader->getCode().data(), "main");
    }

    VkVertexInputBindingDescription bindingDescription;
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Octree::Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    graphicsPipeline->addVertexBindingDescription(bindingDescription);

    {
        const Vulkan::Shader::LocationInfo locationInfo = shaderProgram->getLocationInfo("position");
        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = bindingDescription.binding;
        attributeDescription.location = locationInfo.location;
        attributeDescription.format = locationInfo.format;
        graphicsPipeline->addVertexAttributeDescription(attributeDescription);
    }

    {
        const Vulkan::Shader::LocationInfo locationInfo = shaderProgram->getLocationInfo("color");
        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = bindingDescription.binding;
        attributeDescription.location = locationInfo.location;
        attributeDescription.format = locationInfo.format;
        attributeDescription.offset = sizeof(Octree::Vertex::pos);
        graphicsPipeline->addVertexAttributeDescription(attributeDescription);
    }

    {
        const Vulkan::Shader::LocationInfo locationInfo = shaderProgram->getLocationInfo("normal");
        VkVertexInputAttributeDescription attributeDescription = {};
        attributeDescription.binding = bindingDescription.binding;
        attributeDescription.location = locationInfo.location;
        attributeDescription.format = locationInfo.format;
        attributeDescription.offset = sizeof(Octree::Vertex::pos) + sizeof(Octree::Vertex::color);
        graphicsPipeline->addVertexAttributeDescription(attributeDescription);
    }

    graphicsPipeline->create();
}

PolygonalOctreeRenderer::~PolygonalOctreeRenderer() {

}

void PolygonalOctreeRenderer::writeCommandBuffer(Vulkan::CommandBuffer* commandBuffer, Vulkan::Framebuffer* framebuffer) {
    UNUSED(framebuffer)

    if (vertextCount) {
        commandBuffer->bindPipeline(graphicsPipeline.get());

        commandBuffer->addVertexBuffer(vertexBuffer->getHandle());
        commandBuffer->bindVertexBuffers();

        for (int i = 0; i < shaderProgram->getDescriptorSets()->getCount(); i++) {
            commandBuffer->addDescriptorSet(shaderProgram->getDescriptorSets()->at(i));
        }
        commandBuffer->bindDescriptorSets(graphicsPipeline->getBindPoint(), shaderProgram->getPipelineLayout()->getHandle());

        commandBuffer->draw(vertextCount, 1, 0, 0);
    }
}

void PolygonalOctreeRenderer::draw() {

}

void PolygonalOctreeRenderer::drawView(View3D* view) {

}

void PolygonalOctreeRenderer::setVertexCount(uint32_t vertextCount) {
    this->vertextCount = vertextCount;

    uint32_t size = vertextCount * sizeof(Octree::Vertex);

    if (size > vertexBuffer->getSize()) {
        vertexBuffer = std::make_unique<Vulkan::GpuBuffer>(getDevice(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size);
    }
}

void PolygonalOctreeRenderer::updateMvp(const glm::mat4& mvp) {
    uboBuffer->write(&mvp, sizeof(mvp));
}

} // Origin
