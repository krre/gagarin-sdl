#pragma once
#include "Renderer.h"
#include "ECS/Scenes/Scene.h"
#include "Octree/Octree.h"

namespace Origin {

class RenderLayerOctree;

class SceneRenderer : public Renderer {

public:
    SceneRenderer(Object* parent = nullptr);
    ~SceneRenderer();

    static SceneRenderer* get();

    void draw() override;
    RenderLayer* getRenderLayer() const override;

    void addScene(Scene* scene);

private:
    std::vector<Scene*> scenes;
    RenderLayerOctree* renderLayerOctree;
};

} // Origin
