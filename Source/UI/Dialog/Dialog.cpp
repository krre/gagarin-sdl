#include "Dialog.h"
#include "Scene/SceneManager.h"
#include "Event/Input.h"
#include "Core/Application.h"
#include "Graphics/Render/RenderWindow.h"

Dialog::Dialog() {
    SDL_SetRelativeMouseMode(SDL_FALSE);
    int centerX = Application::get()->getWindow()->getWidth() / 2;
    int centerY = Application::get()->getWindow()->getHeight() / 2;
    SDL_WarpMouseInWindow(Application::get()->getWindow()->getHandle(), centerX, centerY);
//    int posX = centerX - getSize().width / 2;
//    int posY = centerY - getSize().height / 2;
//    setPosition({ posX, posY });
}

void Dialog::close() {
    SceneManager::get()->popScene();
}

void Dialog::onKeyPressed(const SDL_KeyboardEvent& event) {
    if (event.keysym.sym == SDLK_ESCAPE) {
        Input::get()->isKeyAccepted = true;
        close();
    }
}
