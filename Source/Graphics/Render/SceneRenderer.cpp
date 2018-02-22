#include "SceneRenderer.h"
#include "Vulkan/GpuBuffer.h"
#include "Resource/RenderPass/RenderPassVoxel.h"
#include "Graphics/Render/RenderManager.h"
#include "Base/Game.h"

namespace Origin {

SceneRenderer::SceneRenderer(Object* parent) : Object(parent) {
    renderPassVoxel = new RenderPassVoxel(Game::getRenderManager()->getGraphicsDevice(), this);

    glm::vec4 color = { 1.0f, 0.0f, 0.0f, 1.0f };
    Scene::Vertex vertex = {};

    vertex.pos = { -1.0, 1.0, 1.0, 1.0 };
    vertex.color = color;
    vertices.push_back(vertex);

    vertex.pos = { 1.0, 1.0, 1.0, 1.0 };
    vertex.color = color;
    vertices.push_back(vertex);

    vertex.pos = { -1.0, -1.0, 1.0, 1.0 };
    vertex.color = color;
    vertices.push_back(vertex);
}

SceneRenderer::~SceneRenderer() {

}

void SceneRenderer::addScene(Scene* scene) {
    scenes.push_back(scene);
}

void SceneRenderer::drawScenes() {
    for (Scene* scene : scenes) {


    }

    uint32_t size = vertices.size() * sizeof(Scene::Vertex);

    renderPassVoxel->setVertexCount(vertices.size());

    if (size > renderPassVoxel->getVertexBuffer()->getSize()) {
        renderPassVoxel->resizeVertexBuffer(size);
    }

    if (size) {
        renderPassVoxel->getVertexBuffer()->write(vertices.data(), size);
    }

    scenes.clear();
}

} // Origin
