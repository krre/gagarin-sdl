#include "RenderManager.h"
#include "Origin.h"
#include "Core/Settings.h"
#include <algorithm>

RenderManager::RenderManager() {
#ifdef ENABLE_OPENGL
    backends.push_back(GraphicsBackend::OPENGL);
#endif

#ifdef ENABLE_VULKAN
    backends.push_back(GraphicsBackend::VULKAN);
#endif

    std::string backend = Settings::get()->getValue("gfx_backend");
    if (backend == "opengl") {
        currentBackend = GraphicsBackend::OPENGL;
    } else if (backend == "vulkan") {
        currentBackend = GraphicsBackend::VULKAN;
    }

    if (std::find(backends.begin(), backends.end(), currentBackend) == backends.end()) {
        throw std::runtime_error(std::string("Graphics backend ") + backend + " is not supported");
    }
}

GraphicsBackend RenderManager::getCurrentBackend() const {
    return currentBackend;
}

void RenderManager::setCurrentBackend(GraphicsBackend backend) {
    currentBackend = backend;
}

const std::vector<GraphicsBackend>&RenderManager::getBackends() const {
    return backends;
}
