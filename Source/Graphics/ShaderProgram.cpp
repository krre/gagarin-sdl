#include "ShaderProgram.h"
#include "../Resource/ResourceManager.h"
#include "../Core/Utils.h"

using namespace Vulkan;

ShaderProgram::ShaderProgram(const Device* device) :
    device(device),
    graphicsPipeline(device),
    pipelineLayout(device),
    descriptorPool(device),
    descriptorSetLayout(device),
    descriptorSets(device, &descriptorPool) {
}

ShaderProgram::~ShaderProgram() {
    descriptorSets.destroy();
    descriptorPool.destroy();
}

void ShaderProgram::addShader(const std::string& path) {
    ShaderResource* shaderResource = ResourceManager::get()->load<ShaderResource>(path);
    shaderResources.push_back(shaderResource);
    graphicsPipeline.addShaderCode(shaderResource->getStage(), shaderResource->getCodeSize() * sizeof(uint32_t), shaderResource->getCodeData());
}

void ShaderProgram::createResources() {
    std::map<VkDescriptorType, uint32_t> descriptorsTypes;
    int vertexBindingCount = 0;

    for (auto& shaderResource : shaderResources) {
//        parser.dumpDescriptors();
//        parser.dumpInputs();

        for (auto& descriptorIt : shaderResource->descriptors) {
            ShaderResource::Descriptor* descriptor = &descriptorIt.second;
            if (descriptorsTypes.find(descriptor->descriptorType) == descriptorsTypes.end()) {
                descriptorsTypes[descriptor->descriptorType] = 1;
            } else {
                descriptorsTypes[descriptor->descriptorType]++;
            }

            VkDescriptorSetLayoutBinding layoutBinding = {};
            layoutBinding.binding = descriptor->binding;
            layoutBinding.descriptorCount = 1;
            layoutBinding.descriptorType = descriptor->descriptorType;
            layoutBinding.stageFlags = shaderResource->stage;

            descriptorSetLayout.addLayoutBinding(layoutBinding);

            VkWriteDescriptorSet descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstBinding = layoutBinding.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = layoutBinding.descriptorType;
            descriptorWrite.descriptorCount = layoutBinding.descriptorCount;

            const auto& bufferIt = bufferInfos.find(descriptorIt.first);
            if (bufferIt != bufferInfos.end()) {
                VkBufferUsageFlagBits usage;
                if (descriptor->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                    usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
                } else if (descriptor->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                    usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                }
                BufferInfo* bufferInfo = &bufferIt->second;
                std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(device, usage, bufferInfo->size, false);
                buffer->create();
                buffers.push_back(buffer);
                bufferInfo->buffer = buffer.get();
                descriptorWrite.pBufferInfo = &buffer->descriptorInfo;
                descriptorSets.addWriteDescriptorSet(descriptorWrite);
            }

            const auto& imageIt = imageInfos.find(descriptorIt.first);
            if (imageIt != imageInfos.end()) {
                descriptorWrite.pImageInfo = &imageIt->second;
                descriptorSets.addWriteDescriptorSet(descriptorWrite);
            }
        }

        for (auto& inputIt : shaderResource->inputs) {
            const auto& inputInfoIt = inputInfos.find(inputIt.first);
            if (inputInfoIt != inputInfos.end()) {
                ShaderResource::Input* input = &inputIt.second;
                InputInfo* inputInfo = &inputInfoIt->second;
                if (input->variableType == "OpTypeVector") {

                }
            }
        }
    }

    descriptorSetLayout.create();
    descriptorSets.addDescriptorSetLayout(descriptorSetLayout.getHandle());
    pipelineLayout.addDescriptorSetLayout(descriptorSetLayout.getHandle());
    pipelineLayout.create();
    graphicsPipeline.setPipelineLayout(pipelineLayout.getHandle());

    // Descriptor pool
    for (auto& it : descriptorsTypes) {
        descriptorPool.addPoolSize(it.first, it.second);
    }

    descriptorPool.create();
    descriptorSets.allocate();
    descriptorSets.writeDescriptors();

    graphicsPipeline.create();
}

void ShaderProgram::linkUniform(const std::string& name, uint32_t size, void* uniform) {
    BufferInfo linkInfo = {};
    linkInfo.size = size;
    linkInfo.uniform = uniform;
    bufferInfos[name] = linkInfo;
}

void ShaderProgram::linkImage(const std::string& name, VkDescriptorImageInfo descriptorImageInfo) {
    imageInfos[name] = descriptorImageInfo;
}

void ShaderProgram::linkInput(const std::string& name, VkDeviceSize size, VkBufferUsageFlagBits usage, bool moveToDevice) {
    InputInfo inputInfo = {};
    inputInfo.size = size;
    inputInfo.usage = usage;
    inputInfo.moveToDevice = moveToDevice;
    inputInfos[name] = inputInfo;
}

void ShaderProgram::writeUniform(const std::string& name, VkDeviceSize offset, VkDeviceSize size, void* data) {
    bufferInfos.at(name).buffer->write(data != nullptr ? data : bufferInfos.at(name).uniform, size ? size : bufferInfos.at(name).size, offset);
}

void ShaderProgram::readUniform(const std::string& name, VkDeviceSize offset, VkDeviceSize size, void* data) {
    bufferInfos.at(name).buffer->read(data != nullptr ? data : bufferInfos.at(name).uniform, size ? size : bufferInfos.at(name).size, offset);
}

