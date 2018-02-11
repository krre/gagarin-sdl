#pragma once
#include "Screen.h"

namespace Origin {

class Button;
class LinearLayout;

class LoadWorldScreen : public Screen {

public:
    LoadWorldScreen();

private:
    void resizeImpl(int width, int height) override;

    Button* buttonBack;
    LinearLayout* layout;
};

} // Origin
