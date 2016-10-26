#include "GPUMemoryManager.h"
#include "../../Resource/ResourceManager.h"
#include "../../ECS/Components/Components.h"

GPUMemoryManager::GPUMemoryManager() {
    voxelShaderGroup = ResourceManager::getInstance()->getShaderGroup("VoxelShaderGroup");
    program = voxelShaderGroup->getProgram();
    voxelShaderGroup->bind();

    glGenBuffers(1, &octreesSsbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreesSsbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MEMORY_SIZE, NULL, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, octreesSsbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &renderListSsbo);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderListSsbo);
//    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_OCTREE_COUNT, NULL, GL_DYNAMIC_COPY);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, renderListSsbo);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GPUMemoryManager::addEntity(Entity* entity) {
    if (!batch) {
        bind();
    }

    OctreeComponent* oc = static_cast<OctreeComponent*>(entity->components[ComponentType::Octree].get());
    int size = sizeof(uint32_t) * oc->data.get()->size();
    GLvoid* data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, endOffset, size, GL_MAP_WRITE_BIT);
    memcpy(data, oc->data.get()->data(), size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    octreeOffsets[entity->getId()] = endOffset;
    renderList.offsets.push_back(endOffset);
    print(endOffset)
    endOffset += pageBytes;
}

void GPUMemoryManager::updateEntityOctree(Entity* entity) {
    if (!batch) {
        bind();
    }

    OctreeComponent* oc = static_cast<OctreeComponent*>(entity->components[ComponentType::Octree].get());
    int offset = octreeOffsets[entity->getId()];
    int size = sizeof(uint32_t) * oc->data.get()->size();
    GLvoid* data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, GL_MAP_WRITE_BIT);
    memcpy(data, oc->data.get()->data(), size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void GPUMemoryManager::updateEntityTransform(Entity* entity, const std::vector<glm::vec4>& transform) {
    if (!batch) {
        bind();
    }

    int size = sizeof(glm::vec4) * transform.size();
    int offset = octreeOffsets[entity->getId()] + pageBytes - size;
    GLvoid* data = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, size, GL_MAP_WRITE_BIT);
    memcpy(data, transform.data(), size);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

}

void GPUMemoryManager::removeEntity(const Entity* entity) {
    if (!batch) {
        bind();
    }
}

void GPUMemoryManager::updateRenderList() {
    renderList.count = renderList.offsets.size();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderListSsbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(renderList), &renderList, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, renderListSsbo);
    print(renderList.offsets.size() << " " << sizeof(renderList))
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GPUMemoryManager::beginBatch() {
    bind();
    batch = true;
}

void GPUMemoryManager::endBatch() {
    release();
    batch = false;
}

void GPUMemoryManager::bind() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, octreesSsbo);
}

void GPUMemoryManager::release() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
