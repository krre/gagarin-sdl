#include "VoxelShaderProgram.h"
#include "../../Graphics/Voxel/GPUMemoryManager.h"

VoxelShaderProgram::VoxelShaderProgram(const Vulkan::Device* device) : ShaderProgram(device) {
    addShader("Shader/Voxel.vert.spv");
    addShader("Shader/Voxel.frag.spv");

    linkBuffer("ubo", sizeof(ubo), &ubo);
    linkBuffer("octree", MEMORY_SIZE);
    linkBuffer("renderList", MAX_OCTREE_COUNT * sizeof(uint32_t));
    linkBuffer("pickResult", sizeof(pickResult), &pickResult);
    linkBuffer("debugOut", sizeof(debugOut), &debugOut);

    createDescriptors();
}