#pragma once

#include "mill/windowing/window.hpp"

#include <GLFW/glfw3.h>

namespace mill::platform
{
    class WindowGLFW : public WindowInterface
    {
    public:
        WindowGLFW() = default;
        ~WindowGLFW() = default;

        /* Commands */

        void initialise(const WindowInit& init) override;
        void shutdown() override;

        void poll_events() override;

        void set_title(const std::string& title) override;
        void set_size() override;
        void set_position() override;

        /* Getters */

        auto get_size() const -> glm::ivec2 override;
        auto get_resolution() const -> glm::ivec2 override;
        auto get_position() const -> glm::ivec2 override;

        auto get_handle() const -> void* override;
        auto get_platform_handle() const -> void* override;

    private:
        GLFWwindow* m_window = nullptr;
    };
}