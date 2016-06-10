#include "Node.h"
#include <glm/gtc/matrix_transform.hpp>

Node::Node() :
    translationMatrix(1.0f),
    rotationMatrix(1.0f),
    scaleMatrix(1.0f),
    modelMatrix(1.0f) {

}

void Node::addChild(shared_ptr<Node> child) {
    nodes.push_back(child);
}

void Node::removeChild(shared_ptr<Node> child) {
    nodes.remove(child);
}

void Node::setPosition(const glm::vec3& position) {

}

void Node::setRotation(float angle) {

}

void Node::setScale(float scale) {

}

void Node::translate(const glm::vec3& vector) {
    translationMatrix = glm::translate(translationMatrix, vector);
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}
