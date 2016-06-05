#pragma once
#include "../../Core/Object.h"
#include "Shader.h"
#include <GL/glew.h>
#include <list>

class Program : public Object {

public:
    Program();
    void addShader(const Shader& shader);
    bool link();
    void use(bool value);
    GLuint getId() const { return id; }

private:
    GLuint id;
    list<GLuint> shaders;
};
