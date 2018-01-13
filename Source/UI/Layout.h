#pragma once
#include "Control.h"
#include <vector>

namespace Origin {

class Layout : public Control {

public:
    Layout();
    virtual void resize(int width, int height);

    void addControl(Control *control);
    void removeControl(Control* control);
    void clearControls();

    void addLayout(const std::shared_ptr<Layout>& layout);
    void removeLayout(const std::shared_ptr<Layout>& layout);
    void clearLayouts();

    int getSpacing() const { return spacing; }
    void setSpacing(int spacing);

    void setParent(Control* parent);
    void update();

    void prepareBatch(std::vector<Batch2D>& batches, std::vector<Batch2D::Vertex>& vertices) override;

protected:
    virtual void updateContentPostion() = 0;

    std::vector<Control*> controls;
    std::vector<std::shared_ptr<Layout>> layouts;
    int spacing = 5;
    Control* parent = nullptr;
};

} // Origin
