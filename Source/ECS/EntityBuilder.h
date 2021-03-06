#pragma once
#include "Entity.h"

namespace Origin {

class EntityManager;

class EntityBuilder {

public:
    EntityBuilder(EntityManager* entityManager);
    // Geometry
    std::shared_ptr<Entity> geometry();
    std::shared_ptr<Entity> cube();

    // Camera
    std::shared_ptr<Entity> camera();
    std::shared_ptr<Entity> freeCamera();

    // Light
    std::shared_ptr<Entity> light();

    std::shared_ptr<Entity> avatar();

private:
    EntityManager* entityManager;
};

} // Origin
