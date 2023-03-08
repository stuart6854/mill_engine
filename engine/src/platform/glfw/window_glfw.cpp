#include "window_glfw.hpp"

#include "mill/core/debug.hpp"

namespace mill
{
    void platform::WindowGLFW::init(const WindowInit& init)
    {
        if (!glfwInit())
        {
            LOG_ERROR("WindowGLFW - Failed to initialise GLFW!");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(init.width, init.height, init.title.c_str(), nullptr, nullptr);
        if (m_window == nullptr)
        {
            LOG_ERROR("WindowGLFW - Failed to create window!");
            return;
        }

        glfwSetWindowUserPointer(m_window, this);

        glfwSetWindowCloseCallback(m_window,
                                   [](GLFWwindow* window)
                                   {
                                       auto* window_data = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
                                       window_data->cb_on_window_close_requested.emit();
                                   });

        glfwSetWindowSizeCallback(m_window,
                                  [](GLFWwindow* window, i32 width, i32 height)
                                  {
                                      auto* window_data = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
                                      window_data->cb_on_window_size.emit({ width, height });
                                  });

        glfwSetKeyCallback(m_window,
                           [](GLFWwindow* window, i32 key, i32 /*scancode*/, i32 action, i32 /*mods*/)
                           {
                               if (action == GLFW_REPEAT)
                               {
                                   return;
                               }

                               auto* window_data = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
                               window_data->cb_on_input_keyboard_key.emit(key, action != GLFW_RELEASE);
                           });

        glfwSetMouseButtonCallback(m_window,
                                   [](GLFWwindow* window, i32 btn, i32 action, i32 /*mods*/)
                                   {
                                       if (action == GLFW_REPEAT)
                                       {
                                           return;
                                       }

                                       auto* window_data = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
                                       window_data->cb_on_input_mouse_btn.emit(btn, action != GLFW_RELEASE);
                                   });

        glfwSetCursorPosCallback(m_window,
                                 [](GLFWwindow* window, f64 x, f64 y)
                                 {
                                     auto* window_data = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));
                                     window_data->cb_on_input_cursor_pos.emit({ x, y });
                                 });
    }

    void platform::WindowGLFW::shutdown()
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;

        glfwTerminate();
    }

    void platform::WindowGLFW::poll_events()
    {
        glfwPollEvents();
    }

    void platform::WindowGLFW::set_title(const std::string& title)
    {
        glfwSetWindowTitle(m_window, title.c_str());
    }

    void platform::WindowGLFW::set_size() {}

    void platform::WindowGLFW::set_position() {}

    auto platform::WindowGLFW::get_size() const -> glm::ivec2
    {
        i32 w, h;
        glfwGetWindowSize(m_window, &w, &h);
        return { w, h };
    }

    auto platform::WindowGLFW::get_resolution() const -> glm::ivec2
    {
        i32 w, h;
        glfwGetFramebufferSize(m_window, &w, &h);
        return { w, h };
    }

    auto platform::WindowGLFW::get_position() const -> glm::ivec2
    {
        i32 x, y;
        glfwGetWindowPos(m_window, &x, &y);
        return { x, y };
    }

    auto platform::WindowGLFW::get_handle() const -> void*
    {
        return m_window;
    }

    auto platform::create_window() -> Owned<WindowInterface>
    {
        return CreateOwned<platform::WindowGLFW>();
    }
}
