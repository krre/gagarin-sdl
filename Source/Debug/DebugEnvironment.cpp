#include "DebugEnvironment.h"
#include "Core/Application.h"
#include "Core/Game.h"
#include "Core/Defines.h"
#include "Core/Utils.h"
#include "Debug/DebugHUD.h"
#include "Screen/MenuScreen.h"

DebugEnvironment::DebugEnvironment() {
#ifdef DEVELOP_MODE
    loadValues();
#endif
}

void DebugEnvironment::setValue(const std::string& key, const std::string& value) {
    mainStorage[key] = value;
}

std::string DebugEnvironment::getValue(const std::string& key) const {
    if (mainStorage.find(key) != mainStorage.end()) {
        return mainStorage[key];
    } else {
        return std::string();
    }
}

void DebugEnvironment::setDebugScreen() {
    const std::string& screenName = getValue("screen");
    if (!screenName.empty()) {
        if (screenName == "menu") {
            Game::get()->setScreen(std::make_shared<MenuScreen>());
        } else if (screenName == "settings") {
//            Game::get()->setScreen(std::make_shared<SettingsScreen>());
        } else if (screenName == "world") {
//            Game::get()->setScreen(std::make_shared<WorldScreen>());
        }
    } else {
        Game::get()->setScreen(std::make_shared<MenuScreen>());
    }

    if (getValue("debugHUD") == "true") {
//        DebugHUD::get()->setVisible(true);
    }
}

void DebugEnvironment::loadValues() {
    std::string mainPath = Application::getCurrentPath() + "/Debug/main.json";
    std::string mainText = Utils::readTextFile(mainPath);
    mainStorage = json::parse(mainText);

    enable = getValue("enable") == "true";
}
