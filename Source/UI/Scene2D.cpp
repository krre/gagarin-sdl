#include "Scene2D.h"
#include "../Event/Event.h"
#include <algorithm>

Scene2D::Scene2D(int width, int height) : Control(width, height) {
    Event::get()->windowResize.connect<Scene2D, &Scene2D::onWindowResize>(this);
}

void Scene2D::draw(float dt) {
    if (layout != nullptr) {
        layout->draw(dt);
    }

    for (auto control : controls) {
        control->draw(dt);
    }
}

void Scene2D::addControl(std::shared_ptr<Control> control) {
    controls.push_back(control);
}

void Scene2D::removeControl(std::shared_ptr<Control> control) {
    controls.erase(std::remove(controls.begin(), controls.end(), control), controls.end());
}

void Scene2D::clearControls() {
    controls.clear();
}

void Scene2D::setLayout(std::shared_ptr<Layout> layout) {
    this->layout = layout;
    this->layout->setParent(this);
}

void Scene2D::onWindowResize(int width, int height) {
    if (layout != nullptr) {
        layout->resize(width, height);
    }
}
