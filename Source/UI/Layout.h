#pragma once
#include "Rectangle.h"
#include <vector>

class Layout : public Rectangle {

public:
    Layout();
    virtual void resize(int width, int height);
    virtual void draw(float dt) override;

    void addControl(Control *control);
    void removeControl(Control* control);
    void clearControls();

    void addLayout(std::shared_ptr<Layout> layout);
    void removeLayout(std::shared_ptr<Layout> layout);
    void clearLayouts();

    int getSpacing() const { return spacing; }
    void setSpacing(int spacing);

    void setParent(Control* parent);

protected:
    std::vector<Control*> controls;
    std::vector<std::shared_ptr<Layout>> layouts;
    int spacing = 10;
    Control* parent = nullptr;
};
