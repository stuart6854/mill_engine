#include "mill/platform/platform_interface.hpp"

#include "mill/core/debug.hpp"
#include "mill/core/engine.hpp"
#include "mill/events/events.hpp"
#include "mill/input/input_codes.hpp"

#define GLFW_INCLUDE_NONE
#if MILL_WINDOWS
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace mill::platform
{
    void close_callback(GLFWwindow* window);
    void size_callback(GLFWwindow* window, i32 width, i32 height);
    void key_callback(GLFWwindow* window, i32 key, int scancode, int action, int mods);
    void mouse_btn_callback(GLFWwindow* window, i32 btn, int action, int mods);
    void cursor_pos_callback(GLFWwindow* window, f64 x, f64 y);

    bool platform_initialise()
    {
        if (!glfwInit())
        {
            LOG_ERROR("Platform - GLFW - Failed to initialise!");
            return false;
        }
        return true;
    }

    void platform_shutdown()
    {
        glfwTerminate();
    }

    bool platform_pump_messages()
    {
        glfwPollEvents();
        return true;
    }

    auto create_window(const WindowInfo& info) -> HandleWindow
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        auto handle = glfwCreateWindow(info.width, info.height, info.title.c_str(), nullptr, nullptr);
        ASSERT(handle);

        auto primary_monitor = glfwGetPrimaryMonitor();
        auto video_mode = glfwGetVideoMode(primary_monitor);

        i32 client_x = info.pos_x == -1 ? (video_mode->width - info.width) / 2 : info.pos_x;
        i32 client_y = info.pos_y == -1 ? (video_mode->height - info.height) / 2 : info.pos_y;
        glfwSetWindowPos(handle, client_x, client_y);

        const auto* monitor_name = glfwGetMonitorName(primary_monitor);
        LOG_INFO("Platform - GLFW - Window created.");
        LOG_INFO("  position = {}, {}", client_x, client_y);
        LOG_INFO("      size = {}, {}", info.pos_x, info.pos_y);
        LOG_INFO("   monitor = {}", monitor_name);

        glfwSetWindowCloseCallback(handle, close_callback);
        glfwSetWindowSizeCallback(handle, size_callback);
        glfwSetKeyCallback(handle, key_callback);
        glfwSetMouseButtonCallback(handle, mouse_btn_callback);
        glfwSetCursorPosCallback(handle, cursor_pos_callback);

        return handle;
    }

    void destroy_window(HandleWindow window_handle)
    {
        glfwDestroyWindow(static_cast<GLFWwindow*>(window_handle.ptr));
    }

    auto get_raw_window_handle(HandleWindow window_handle) -> void*
    {
#if MILL_WINDOWS
        return glfwGetWin32Window(static_cast<GLFWwindow*>(window_handle.ptr));
#endif
    }

#if 0
    auto platform_allocate(u64 size, bool aligned) -> void*
    {
        return malloc(size);
    }

    void platform_free(void* block, bool aligned)
    {
        free(block);
    }
#endif

    auto platform_get_absolute_time() -> f64
    {
        return glfwGetTime();
    }

#pragma region GLFW Callbacks

    void close_callback(GLFWwindow* window)
    {
        Event event{};
        event.type = EventType::eWindowClose;
        event.context = window;

        Engine::get()->get_events().post_immediate(event);
    }

    void size_callback(GLFWwindow* window, i32 width, i32 height)
    {
        Event event{};
        event.type = EventType::eWindowSize;
        event.context = window;
        event.data.u32[0] = width;
        event.data.u32[1] = height;

        Engine::get()->get_events().post_immediate(event);
    }

    void key_callback(GLFWwindow* window, i32 key, int scancode, int action, int mods)
    {
        UNUSED(scancode);
        UNUSED(mods);

        Event event{};
        event.type = EventType::eInputKey;
        event.context = window;
        event.data.u32[0] = key;
        event.data.u32[1] = action != GLFW_RELEASE;

        Engine::get()->get_events().post_immediate(event);
    }

    void mouse_btn_callback(GLFWwindow* window, i32 btn, int action, int mods)
    {
        UNUSED(mods);

        Event event{};
        event.type = EventType::eInputMouseBtn;
        event.context = window;
        event.data.u32[0] = btn;
        event.data.u32[1] = action != GLFW_RELEASE;

        Engine::get()->get_events().post_immediate(event);
    }

    void cursor_pos_callback(GLFWwindow* window, f64 x, f64 y)
    {
        Event event{};
        event.type = EventType::eInputMouseMove;
        event.context = window;
        event.data.i32[0] = CAST_I32(x);
        event.data.i32[1] = CAST_I32(y);

        Engine::get()->get_events().post_immediate(event);
    }

#pragma endregion
}
