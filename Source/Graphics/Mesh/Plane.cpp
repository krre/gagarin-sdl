#include "Plane.h"

Plane::Plane(float width, float height) : width(width), height(height) {
    static GLfloat vertexData[] = {
        -1.0f * width, 0.0f, -1.0f * height,
        1.0f * width, 0.0f, 1.0f * height,
        1.0f * width, 0.0f, -1.0f * height,

        -1.0f * width,  0.0f, -1.0f * height,
        -1.0f * width,  0.0f, 1.0f * height,
        1.0f * width,  0.0f, 1.0f * height,
    };

    verticles = std::vector<GLfloat>(vertexData, vertexData + sizeof vertexData / sizeof vertexData[0]);
    setup();
}
