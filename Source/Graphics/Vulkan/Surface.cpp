#include "Surface.h"
#include "../../Core/App.h"
#include <SDL_syswm.h>
#ifdef __linux__
    #include <X11/Xlib-xcb.h>
#elif _WIN32

#endif


using namespace Vulkan;

Surface::Surface(const Instance* instance) : instance(instance) {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    SDL_GetWindowWMInfo(App::get()->getWindow(), &wminfo);

    switch (wminfo.subsystem) {
#ifdef __linux__
    case SDL_SYSWM_X11: {
        VkXcbSurfaceCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        createInfo.flags = 0;
        createInfo.connection = XGetXCBConnection(wminfo.info.x11.display);
        createInfo.window = wminfo.info.x11.window;

        result = vkCreateXcbSurfaceKHR(instance->getHandle(), &createInfo, nullptr, &handle);
        print("X11: " << handle)
        break;
    }

#elif _WIN32
    case SDL_SYSWM_WINDOWS: {
//        TCHAR* className;
//        GetClassName(wminfo.info.win.window, className, 256);
//        WNDCLASS wce;
//        GetClassInfo(GetModuleHandle(NULL), className, &wce);

        VkWin32SurfaceCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.flags = 0;
        createInfo.hwnd = wminfo.info.win.window;
//        createInfo.hinstance = wce.hInstance;
        createInfo.hinstance = GetModuleHandle(nullptr);

        result = vkCreateWin32SurfaceKHR(instance->getHandle(), &createInfo, nullptr, &handle);

        print("Windows: " << handle)
        break;
    }
#endif

    }
}

Surface::~Surface() {
    if (handle) {
        vkDestroySurfaceKHR(instance->getHandle(), handle, nullptr);
    }
}