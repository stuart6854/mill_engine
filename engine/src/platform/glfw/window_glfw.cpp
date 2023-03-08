#include "window_glfw.hpp"

#include "mill/core/debug.hpp"

namespace mill
{
    void platform::WindowGLFW::init(u32 width, u32 height, const std::string& title)
    {
        if (!glfwInit())
        {
            LOG_ERROR("WindowGLFW - Failed to initialise GLFW!");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (m_window == nullptr)
        {
            LOG_ERROR("WindowGLFW - Failed to create window!");
            return;
        }
    }

    void platform::WindowGLFW::shutdown()
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;

        glfwTerminate();
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
