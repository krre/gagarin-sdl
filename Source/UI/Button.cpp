#include "Button.h"
#include "../Event/Event.h"

Button::Button() : Rectangle(100, 20) {
    setColor(glm::vec3(0.6, 0.6, 0.6));
    Event::get()->mouseButtonAction.connect<Button, &Button::onMouseButtonAction>(this);
}

Button::~Button() {
   Event::get()->mouseButtonAction.disconnect<Button, &Button::onMouseButtonAction>(this);
}

void Button::setText(const std::string &text) {
    this->text = text;
    labelText.setText(text);
    labelText.setZ(1.0f);
}

void Button::draw(float dt) {
    Rectangle::draw(dt);
    labelText.setPosition(glm::vec2(position.x + 10, position.y + 15));
    labelText.draw(dt);
}

void Button::onMouseButtonAction(const SDL_MouseButtonEvent& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = event.x;
        int mouseY = event.y;

        if (mouseX > position.x && mouseX < (position.x + width) && mouseY > position.y && mouseY < (position.y + height)) {
            clicked.emit();
        }
    }
}
