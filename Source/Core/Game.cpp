#include "Game.h"
#include "Utils.h"
#include "Defines.h"
#include "Event/Event.h"
#include "Event/Input.h"
#include "Resource/ResourceManager.h"
#include "Debug/Logger.h"
#include "Debug/DebugEnvironment.h"
#include "UI/Overlay.h"
#include "UI/UIManager.h"
#include "ECS/EntityManager.h"
#include "Core/Settings.h"
#include "Graphics/Render/RenderEngine.h"
#include "Window.h"
#include "Screen/MenuScreen.h"
#include "SDLWrapper.h"
#include <string>
#include <SDL_timer.h>
#include <algorithm>
#include <memory>
#include <experimental/filesystem>

#if defined(OS_LINUX)
    #include <X11/Xlib-xcb.h>
#endif

namespace Origin {

namespace {
    std::vector<std::string> argvs;
    bool running = false;

    Settings* settings;
    Logger* logger;
    DebugEnvironment* debugEnvironment;
    Event* event;
    Window* window;
    RenderEngine* renderEngine;
    UIManager* uiManager;
    EntityManager* entityManager;
    ResourceManager* resourceManager;
    Overlay* overlay;
    Input* input;
}

namespace Game {

void init(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        argvs.push_back(argv[i]);
    }

    try {
        SDL::init();
        settings = new Settings;
        logger = new Logger;
        debugEnvironment = new DebugEnvironment;
        event = new Event;
        window = new Window;
        resourceManager = new ResourceManager;

        SDL_SysWMinfo wminfo = SDL::getSysWMinfo(window->getHandle());

#if defined(OS_WIN)
        renderEngine = new RenderEngine(GetModuleHandle(nullptr), (void*)wminfo.info.win.window);
#elif defined(OS_LINUX)
        renderEngine = new RenderEngine((void*)XGetXCBConnection(wminfo.info.x11.display), (void*)&wminfo.info.x11.window);
#endif

        if (debugEnvironment->getEnable()) {
            if (debugEnvironment->getSettings()["vulkan"]["layers"]["use"]) {
                renderEngine->setEnabledLayers(debugEnvironment->getSettings()["vulkan"]["layers"]["list"]);
            }

            if (debugEnvironment->getSettings()["vulkan"]["extensions"]["use"]) {
                renderEngine->setEnabledExtensions(debugEnvironment->getSettings()["vulkan"]["extensions"]["list"]);
            }

            renderEngine->setDeviceIndex(debugEnvironment->getVulkanDevice());
        }
        renderEngine->create();

        uiManager = new UIManager;
        entityManager = new EntityManager;
        overlay = new Overlay;
        input = new Input;
    } catch (const std::exception& ex) {
        if (SDL::isInited()) {
            SDL::showErrorMessageBox(ex.what());
        } else {
            PRINT(ex.what());
        }
    }

    if (debugEnvironment->getEnable()) {
        debugEnvironment->setDebugScreen();
    } else {
        window->setScreen(std::make_shared<MenuScreen>());
    }

    window->show();

    running = true;
}

void shutdown() {
    delete input;
    delete overlay;
    delete entityManager;
    delete uiManager;
    delete window;
    delete renderEngine;
    delete resourceManager;
    delete event;
    delete debugEnvironment;
    delete settings;
    delete logger;
    SDL::shutdown();
}

void run() {
    Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 currentTime = SDL_GetPerformanceCounter();

    while (running) {
        event->handleEvents();

        Uint64 newTime = SDL_GetPerformanceCounter();
        double frameTime = double(newTime - currentTime) / frequency;
        currentTime = newTime;

        window->update(frameTime);
        window->render();
//        PRINT(frameTime << " " << 1 / frameTime)
    }
}

void quit() {
    running = false;
}

std::string getCurrentDirectory() {
    return std::experimental::filesystem::current_path().string();
}

Window* getWindow() {
    return window;
}

UIManager* getUIManager() {
    return uiManager;
}

EntityManager* getEntityManager() {
    return entityManager;
}

Settings* getSettings() {
    return settings;
}

Logger* getLogger() {
    return logger;
}

DebugEnvironment* getDebugEnvironment() {
    return debugEnvironment;
}

Event* getEvent() {
    return event;
}

ResourceManager* getResourceManager() {
    return resourceManager;
}

Input* getInput() {
    return input;
}

Overlay* getOverlay() {
    return overlay;
}

RenderEngine* getRenderEngine() {
    return renderEngine;
}

} // Game

} // Origin
