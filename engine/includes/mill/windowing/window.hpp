#pragma once

#include "mill/core/base.hpp"

#include <glm/ext/vector_int2.hpp>

#include <string>

namespace mill
{
    class WindowInterface
    {
    public:
        WindowInterface() = default;
        ~WindowInterface() = default;

        /* Commands */

        virtual void init(u32 width, u32 height, const std::string& title) = 0;
        virtual void shutdown() = 0;

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