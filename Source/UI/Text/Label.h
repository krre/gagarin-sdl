#pragma once
#include "../Control.h"

class Font;

class Label : public Control {

public:
    Label();
    void setText(const std::string& text);
    const std::string& getText() const { return text; }

    void setFont(Font* font);
    Font* getFont() const { return font; }

    void setColor(const glm::vec3& color);
    glm::vec3 getColor() const { return color; }

    void draw(float dt) override;
    void update(float dt) override;

private:
    void updateTextData();
    std::string text;
    Font* font;
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
};
