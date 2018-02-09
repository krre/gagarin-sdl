#include "LinearLayout.h"
#include "Control.h"

namespace Origin {

LinearLayout::LinearLayout(Direction direction, Control* parent) :
    Layout(parent),
    direction(direction) {

}

void LinearLayout::updateContentPostion() {
    int i = 0;
    for (const auto& control : controls) {
        if (direction == Direction::Vertical) {
//            control->setPosition({ position.x, position.y + i * ((int)control->getSize().width + spacing) });
            i++;
        } else {
//            control->setPosition({ position.x + i * ((int)control->getSize().height + spacing), position.y });
            i++;
        }
    }
}

} // Origin
