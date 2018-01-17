#include "VulkanShaderProgram.h"
#include "Graphics/Vulkan/VulkanRenderContext.h"
#include "Graphics/Vulkan/Wrapper/Pipeline/PipelineLayout.h"
#include "Graphics/Vulkan/Wrapper/Pipeline/GraphicsPipeline.h"
#include "Graphics/Vulkan/Wrapper/Descriptor/DescriptorPool.h"
#include "Graphics/Vulkan/Wrapper/Descriptor/DescriptorSetLayout.h"
#include "Graphics/Vulkan/Wrapper/Descriptor/DescriptorSets.h"
#include "Graphics/Vulkan/Wrapper/RenderPass.h"
#include "Resource/ResourceManager.h"
#include "Resource/ShaderResource.h"
#include "Core/Utils.h"
#include "Core/Application.h"
#include "Graphics/Render/RenderWindow.h"
#include "Graphics/Vulkan/Wrapper/Instance.h"
#include "Graphics/Vulkan/Wrapper/Surface/Swapchain.h"
#include "Graphics/Vulkan/Wrapper/Buffer/Buffer.h"
#include "Graphics/Vulkan/Wrapper/Surface/Surface.h"

namespace Origin {

VulkanShaderProgram::VulkanShaderProgram() {
    descriptorPool = std::make_unique<Vulkan::DescriptorPool>(vkCtx->getGraphicsDevice());
    descriptorSets = std::make_unique<Vulkan::DescriptorSets>(vkCtx->getGraphicsDevice(), descriptorPool.get());

    graphicsPipeline = std::make_unique<Vulkan::GraphicsPipeline>(vkCtx->getGraphicsDevice());
//    graphicsPipeline->setExtent(Application::get()->getWindow()->getSurface()->getCapabilities().currentExtent);

    pipelineLayout = std::make_unique<Vulkan::PipelineLayout>(vkCtx->getGraphicsDevice());
    descriptorSetLayout = std::make_unique<Vulkan::DescriptorSetLayout>(vkCtx->getGraphicsDevice());
}

VulkanShaderProgram::~VulkanShaderProgram() {
    descriptorSets->destroy();
    descriptorPool->destroy();
}

void VulkanShaderProgram::addShader(ShaderType type, const std::string& name) {
//    ShaderResource* shaderResource = ResourceManager::get()->load<ShaderResource>(name);
//    shaderResources.push_back(shaderResource);
//    graphicsPipeline->addShaderCode(shaderResource->getStage(), shaderResource->getCodeSize() * sizeof(uint32_t), shaderResource->getCodeData());
}

void VulkanShaderProgram::createPipeline() {
    assert(graphicsPipeline->getHandle() == VK_NULL_HANDLE);

    for (const auto& shaderResource : shaderResources) {
//        shaderResource->dumpBindings();
//        shaderResource->dumpLocations();

        for (auto& bindingIt : shaderResource->bindings) {
            VkDescriptorSetLayoutBinding* layoutBinding = &bindingIt.second;
            if (descriptorsTypes.find(layoutBinding->descriptorType) == descriptorsTypes.end()) {
                descriptorsTypes[layoutBinding->descriptorType] = 1;
            } else {
                descriptorsTypes[layoutBinding->descriptorType]++;
            }

            descriptorSetLayout->addLayoutBinding(*layoutBinding);

            VkWriteDescriptorSet writeDescriptorSet = {};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstBinding = layoutBinding->binding;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = layoutBinding->descriptorType;
            writeDescriptorSet.descriptorCount = layoutBinding->descriptorCount;

            const auto& bufferIt = bufferInfos.find(bindingIt.first);
            if (bufferIt != bufferInfos.end()) {
                VkBufferUsageFlagBits usage;
                if (layoutBinding->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                    usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                } else if (layoutBinding->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                    usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                }
                BufferInfo* bufferInfo = &bufferIt->second;
                std::shared_ptr<Vulkan::Buffer> buffer = std::make_shared<Vulkan::Buffer>(vkCtx->getGraphicsDevice(), usage, bufferInfo->size);
                buffer->create();
                bufferInfo->buffer = buffer;
                writeDescriptorSet.pBufferInfo = buffer->getDescriptorInfo();
                descriptorSets->addWriteDescriptorSet(writeDescriptorSet);
            }

            const auto& imageIt = imageInfos.find(bindingIt.first);
            if (imageIt != imageInfos.end()) {
                writeDescriptorSet.pImageInfo = &imageIt->second;
                descriptorSets->addWriteDescriptorSet(writeDescriptorSet);
            }
        }

        for (auto& location : shaderResource->locations) {
            const std::string& name = location.first;
            const auto& inputInfoIt = locationInfos.find(name);
            if (inputInfoIt != locationInfos.end()) {
                ShaderResource::Location* input = &location.second;
                locationInfos.at(name).location = input->location;
                locationInfos.at(name).format = input->format;
                graphicsPipeline->addVertexAttributeDescription(locationInfos.at(name));
            }
        }
    }

    descriptorSetLayout->create();
    descriptorSets->addDescriptorSetLayout(descriptorSetLayout->getHandle());
    pipelineLayout->addDescriptorSetLayout(descriptorSetLayout->getHandle());
    pipelineLayout->create();
    graphicsPipeline->setPipelineLayout(pipelineLayout->getHandle());

    // Descriptor pool
    for (const auto& it : descriptorsTypes) {
        descriptorPool->addPoolSize(it.first, it.second);
    }

    descriptorPool->create();
    descriptorSets->allocate();
    descriptorSets->updateDescriptorSets();

    graphicsPipeline->create();
}

int VulkanShaderProgram::createVertexInputBindingDescription(uint32_t stride, VkVertexInputRate inputRate) {
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = vertexBindingCount++;
    bindingDescription.inputRate = inputRate;
    bindingDescription.stride = stride;
    graphicsPipeline->addVertexBindingDescription(bindingDescription);
    return bindingDescription.binding;
}

void VulkanShaderProgram::bindUniform(const std::string& name, uint32_t size, void* uniform) {
    BufferInfo linkInfo = {};
    linkInfo.size = size;
    linkInfo.uniform = uniform;
    bufferInfos[name] = linkInfo;
}

void VulkanShaderProgram::bindImage(const std::string& name, VkDescriptorImageInfo descriptorImageInfo) {
    imageInfos[name] = descriptorImageInfo;
}

void VulkanShaderProgram::bindInput(const std::string& name, uint32_t binding, uint32_t offset) {
    locationInfos[name].binding = binding;
    locationInfos[name].offset = offset;
}

void VulkanShaderProgram::writeUniform(const std::string& name, VkDeviceSize offset, VkDeviceSize size, void* data) {
//    bufferInfos.at(name).buffer->write(data != nullptr ? data : bufferInfos.at(name).uniform, size ? size : bufferInfos.at(name).size, offset);
}

void VulkanShaderProgram::readUniform(const std::string& name, VkDeviceSize offset, VkDeviceSize size, void* data) {
//    bufferInfos.at(name).buffer->read(data != nullptr ? data : bufferInfos.at(name).uniform, size ? size : bufferInfos.at(name).size, offset);
}

} // Origin