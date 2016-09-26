#include "Camera.h"
#include <iostream>

Camera::Camera(QObject* parent) : QObject(parent) {

}

void Camera::setCameraToWorld(const glm::mat4 cameraToWorld) {
    m_cameraToWorld = cameraToWorld;
    m_worldToCamera = glm::inverse(m_cameraToWorld);
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    distance = glm::distance(m_position, target);
}

void Camera::setPosition(const glm::vec3& position) {
    m_position = position;
    setCameraToWorld(glm::translate(m_cameraToWorld, position));
}

void Camera::rotate(float yaw, float pitch) {
    glm::mat4 R = glm::yawPitchRoll(glm::radians(yaw), glm::radians(pitch), 0.0f);
    glm::vec3 T = glm::vec3(0, 0, distance);
    T = glm::vec3(R * glm::vec4(T, 0.0f));
    m_position = m_target + T;
    m_look = glm::normalize(m_target - m_position);
    m_up = glm::vec3(R * glm::vec4(UP, 0.0f));
    m_right = glm::cross(m_look, m_up);
    m_worldToCamera = glm::lookAt(m_position, m_target, m_up);
    m_cameraToWorld = glm::inverse(m_worldToCamera);
}
