#pragma once
#include "Dialog.h"

class PauseDialog : public Dialog {

public:
    PauseDialog();
    void init() override;
    void update(float dt);

private:
//    void writeCommands(Vulkan::CommandBuffer* commandBuffer) override;
};
