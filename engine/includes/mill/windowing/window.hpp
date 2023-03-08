#pragma once

#include "mill/core/base.hpp"
#include "mill/utility/signal.hpp"

#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_float2.hpp>

#include <string>

namespace mill
{
    class WindowInterface
    {
    public:
        Signal<> cb_on_window_close_requested{};
        Signal<glm::ivec2> cb_on_window_size{};
        Signal<i32, bool> cb_on_input_keyboard_key{};
        Signal<i32, bool> cb_on_input_mouse_btn{};
        Signal<glm::vec2> cb_on_input_cursor_pos{};

        WindowInterface() = default;
        ~WindowInterface() = default;

        /* Commands */

        virtual void init(u32 width, u32 height, const std::string& title) = 0;
        virtual void shutdown() = 0;

        virtual void poll_events() = 0;

        virtual void set_title(const std::string& title) = 0;
        virtual void set_size() = 0;
        virtual void set_position() = 0;

        /* Getters */

        virtual auto get_size() const -> glm::ivec2 = 0;
        virtual auto get_resolution() const -> glm::ivec2 = 0;
        virtual auto get_position() const -> glm::ivec2 = 0;

        virtual auto get_handle() const -> void* = 0;
    };

    namespace platform
    {
        auto create_window() -> Owned<WindowInterface>;
    }

}