#pragma once
#include "RenderLayer.h"
#include <glm/glm.hpp>

namespace Vulkan {
    class GpuBuffer;
    class ShaderProgram;
    class GraphicsPipeline;
}

namespace Origin {

class RenderLayerOctree : public RenderLayer {

public:

    struct UBO {
        glm::vec4 backgroundColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
        glm::vec4 lightColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
        glm::vec4 lightPos = glm::vec4(1.0, 1.0, 1.0, 1.0);

        int shadeless = 1;
        int frameWidth = 0;
        int frameHeight = 0;

        float ambientStrength = 1.0;
        float lod = 1.0;
    };

    struct MetaData {
        glm::vec3 origin;
        glm::vec3 startCorner;
        glm::vec3 stepW;
        glm::vec3 stepH;
        glm::uint blockIndex;
    };

    RenderLayerOctree(Vulkan::Device* device, Object* parent = nullptr);
    ~RenderLayerOctree();

    void write(Vulkan::CommandBuffer* commandBuffer, Vulkan::Framebuffer* framebuffer) override;
    Vulkan::GpuBuffer* getVertexBuffer() const { return vertexBuffer.get(); }

    void writeUBO(const UBO& ubo);
    void writeBlocks(uint32_t offset, void* data, uint32_t size);
    void writeMeta(uint32_t offset, void* data, uint32_t size);

private:
    std::unique_ptr<Vulkan::GpuBuffer> vertexBuffer;
    std::unique_ptr<Vulkan::GpuBuffer> uboBuffer;
    std::unique_ptr<Vulkan::GpuBuffer> blocksBuffer;
    std::unique_ptr<Vulkan::GpuBuffer> metaBuffer;
    std::unique_ptr<Vulkan::ShaderProgram> shaderProgram;
    std::unique_ptr<Vulkan::GraphicsPipeline> graphicsPipeline;
    uint32_t vertextCount = 0;
};

} // Origin
