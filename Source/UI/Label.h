#pragma once
#include "Control.h"
#include "Graphics/Color.h"

namespace Origin {

class Font;

class Label : public Control {

public:
    Label(const std::string& text = "", Control* parent = nullptr);
    void setText(const std::string& text);
    const std::string& getText() const { return text; }

    void setFont(Font* font);
    Font* getFont() const { return font; }

    void setColor(const Color& color);
    const Color& getColor() const { return color; }

private:
    void drawImpl() override;

    void updateTextData();
    std::string text;
    Font* font;
    Color color = Color::BLACK;
};

} // Origin
