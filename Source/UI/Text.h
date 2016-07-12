#pragma once
#include "Control.h"
#include "Font.h"
#include "../Graphics/Effect.h"
#include <glm/glm.hpp>

class Text : public Control {

    struct Character {
        GLuint textureId;   // ID handle of the glyph texture
        glm::ivec2 size;    // Size of glyph
        glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
        GLuint advance;    // Horizontal offset to advance to next glyph
    };

public:
    Text();
    void setText(const std::string& text);
    std::string getText() const { return text; }

    void setFont(Font* font);
    Font* getFont() const { return font; }

    void setColor(const glm::vec4& color);
    glm::vec4 getColor() const { return color; }

    void setScale(float scale);
    float getScale() const { return scale; }

    void setFontSize(int fontSize);

    void render(float dt) override;

private:
    std::string text;
    Font* font;
    glm::vec4 color;
    float scale = 1.0;
    std::map<GLchar, Character> characters;
    GLuint vao, vbo;
    int fontSize = 14;
    Effect* fontEffect;
};
