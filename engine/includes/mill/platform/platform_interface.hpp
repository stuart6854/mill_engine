#pragma once

#include "mill/core/base.hpp"

#include <glm/ext/vector_int2.hpp>

namespace mill::platform
{
    struct HandleWindow
    {
        // Pointer to the platform specific window handle.
        void* ptr{ nullptr };

        HandleWindow() = default;
        HandleWindow(void* window) : ptr(window) {}
        ~HandleWindow() = default;

        bool operator==(const HandleWindow& rhs) const
        {
            return ptr == rhs.ptr;
        }

        bool operator!=(const HandleWindow& rhs) const
        {
            return !(*this == rhs);
        }
    };

    struct PlatformInit
    {
    };

    struct WindowInfo
    {
        std::string title = "<Mill Engine>";
        i32 pos_x{ -1 };
        i32 pos_y{ -1 };
        i32 width{ 1280 };
        i32 height{ 720 };
    };

    /* Initialise the platform layer. */
    bool platform_initialise();
    /* Shutdown the platform layer. */
    void platform_shutdown();

    /* Called once every frame. */
    bool platform_pump_messages();

    auto create_window(const WindowInfo& info) -> HandleWindow;
    void destroy_window(HandleWindow window_handle);
    auto get_raw_window_handle(HandleWindow window_handle) -> void*;

#if 0
    auto platform_allocate(u64 size, bool aligned) -> void*;
    void platform_free(void* block, bool aligned);
#endif

    auto platform_get_absolute_time() -> f64;

}