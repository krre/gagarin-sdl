#pragma once
#include "ECS/Component.h"
#include "Core/App.h"
#include "Core/Window.h"
#include <glm/ext.hpp>

class CameraComponent : public Component {

public:
    CameraComponent() {
        type = ComponentType::Camera;
        int width = App::get()->getWindow()->getWidth();
        int height = App::get()->getWindow()->getHeight();
        aspect = width * 1.0f / height;
    }
    float near = 0.01f;
    float far = 100.0f;
    float fov = glm::radians(50.0f);
    float aspect;
    const float distance = 0.5f; // distance from camera to screen
    const glm::vec4 up = glm::vec4(0.0, -1.0, 0.0, 0.0); // Flip Vulkan coordinate system by setting Y to -1
    const glm::vec4 look = glm::vec4(0.0, 0.0, -1.0, 0.0);
    const glm::vec4 right = glm::vec4(1.0, 0.0, 0.0, 0.0);
    glm::mat4 projection = glm::perspective(fov, aspect, near, far);
};
