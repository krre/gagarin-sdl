#include "ShaderProgram.h"
#include "../Resource/ResourceManager.h"
#include "../Core/Utils.h"
#include "SpirvParser.h"

using namespace Vulkan;

ShaderProgram::ShaderProgram(const Device* device) :
    device(device),
    descriptorPool(device),
    descriptorSetLayout(device),
    descriptorSets(device, &descriptorPool) {

}

ShaderProgram::~ShaderProgram() {
    descriptorSets.destroy();
    descriptorPool.destroy();
}

void ShaderProgram::addShader(const std::string& path) {
    // Template of shader filename: name.type.spv (e.g. Voxel.frag.spv)
    std::vector<std::string> words = Utils::split(path, '.');
    std::string type = words.at(words.size() - 2);
    Type enumType;
    if (type == "vert") {
        enumType = Type::VERTEX;
    } else if (type == "frag") {
        enumType = Type::FRAGMENT;
    } else if (type == "comp") {
        enumType = Type::COMPUTE;
    }
    shaderResources[enumType] = ResourceManager::get()->load<ShaderResource>(path);
}

void ShaderProgram::createDescriptors() {
    SpirvParser parser;
    std::map<VkDescriptorType, uint32_t> descriptorsTypes;

    for (auto& it : shaderResources) {
        ShaderResource* shaderResource = it.second;
        parser.parse(shaderResource->getData(), shaderResource->getSize());
        parser.dumpDescriptors();

        for (auto& descriptor : parser.descriptors) {
            if (descriptor.variableType == "Uniform") {
                if (descriptorsTypes.find(descriptor.descriptorType) == descriptorsTypes.end()) {
                    descriptorsTypes[descriptor.descriptorType] = 1;
                } else {
                    descriptorsTypes[descriptor.descriptorType]++;
                }

                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding = descriptor.binding;
                layoutBinding.descriptorCount = 1;
                layoutBinding.descriptorType = descriptor.descriptorType;
                if (it.first == Type::VERTEX) {
                    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                } else if (it.first == Type::FRAGMENT) {
                    layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                }

                descriptorSetLayout.addLayoutBinding(layoutBinding);
            }
        }
    }

    descriptorSetLayout.create();

    // Descriptor pool
    for (auto& it : descriptorsTypes) {
        descriptorPool.addPoolSize(it.first, it.second);
    }

    descriptorPool.create();
}

void ShaderProgram::linkBuffer(std::string name, UniformBuffer* uniformBuffer, uint32_t size) {
    LinkInfo linkInfo = {};
    linkInfo.name = name;
    linkInfo.size = size;
    uniformLinks[uniformBuffer] = linkInfo;
}
