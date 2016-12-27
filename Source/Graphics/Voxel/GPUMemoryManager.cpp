#include "GPUMemoryManager.h"
#include "../../Resource/ResourceManager.h"
#include "../../ECS/Components/Components.h"

GPUMemoryManager::GPUMemoryManager() {
//    voxelShaderGroup = ResourceManager::get()->getResource<ShaderGroup>("VoxelShaderGroup");
//    program = voxelShaderGroup->getProgram();
//    voxelShaderGroup->bind();

//    glGenBuffers(1, &octreesSsbo);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreesSsbo);
//    glBufferData(GL_SHADER_STORAGE_BUFFER, MEMORY_SIZE, nullptr, GL_DYNAMIC_COPY);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, octreesSsbo);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

//    glGenBuffers(1, &renderListSsbo);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderListSsbo);
//    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_OCTREE_COUNT, nullptr, GL_DYNAMIC_COPY);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, renderListSsbo);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GPUMemoryManager::addEntity(Entity* entity, Vulkan::Descriptor* descriptor) {
    OctreeComponent* oc = static_cast<OctreeComponent*>(entity->components[ComponentType::Octree].get());
    int size = sizeof(uint32_t) * oc->data.get()->size();
    descriptor->update(endOffset, size, oc->data->data());

    octreeOffsets[entity->getId()] = endOffset;
    renderOffsets.push_back(endOffset);
    endOffset += PAGE_BYTES;
}

void GPUMemoryManager::updateEntityOctree(Entity* entity) {
    OctreeComponent* oc = static_cast<OctreeComponent*>(entity->components[ComponentType::Octree].get());
    int offset = octreeOffsets[entity->getId()];
    int size = sizeof(uint32_t) * oc->data.get()->size();
//    GLvoid* data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, GL_MAP_WRITE_BIT);
//    memcpy(data, oc->data.get()->data(), size);
//    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void GPUMemoryManager::updateEntityTransform(Entity* entity, const std::vector<glm::vec4>& transform) {
    int size = sizeof(glm::vec4) * transform.size();
    int offset = octreeOffsets[entity->getId()] + PAGE_BYTES - size;
//    GLvoid* data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, GL_MAP_WRITE_BIT);
//    memcpy(data, transform.data(), size);
//    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

}

void GPUMemoryManager::removeEntity(const Entity* entity) {

}

void GPUMemoryManager::updateRenderList() {
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderListSsbo);
//    // Count
//    GLvoid* data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), GL_MAP_WRITE_BIT);
//    int size = renderOffsets.size();
//    memcpy(data, &size, sizeof(int));
//    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//    // Render list
//    size = sizeof(uint32_t) * renderOffsets.size();
//    data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, sizeof(int), size, GL_MAP_WRITE_BIT);
//    memcpy(data, renderOffsets.data(), size);
//    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
