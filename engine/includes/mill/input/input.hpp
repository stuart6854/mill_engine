#pragma once

#include "input_codes.hpp"

#include <glm/ext/vector_int2.hpp>
#include <glm/ext/vector_float2.hpp>

#include <array>

namespace mill
{
    class InputInterface
    {
    public:
        InputInterface() = default;
        ~InputInterface() = default;

        /* Commands */

        virtual void initialise() = 0;
        virtual void shutdown() = 0;

        /* This sets the last state to the current state. This should be called *before* whatever sets input state is called. */
        virtual void new_frame() = 0;

        virtual void set_key(KeyCodes key, bool is_down) = 0;
        virtual void set_mouse_btn(MouseButtonCodes btn, bool is_down) = 0;

        virtual void set_cursor_pos(const glm::vec2& pos) = 0;

        /* Getters */

        virtual bool on_key_down(KeyCodes key) = 0;
        virtual bool on_key_up(KeyCodes key) = 0;
        virtual bool on_key_held(KeyCodes key) = 0;

        virtual bool on_mouse_btn_down(MouseButtonCodes btn) = 0;
        virtual bool on_mouse_btn_up(MouseButtonCodes btn) = 0;
        virtual bool on_mouse_btn_held(MouseButtonCodes btn) = 0;

        virtual auto get_cursor_pos() -> const glm::vec2& = 0;
        virtual auto get_cursor_delta() -> glm::vec2 = 0;
    };

    class InputDefault : public InputInterface
    {
    public:
        InputDefault() = default;
        ~InputDefault() = default;

        DISABLE_COPY_AND_MOVE(InputDefault);

        /* Commands */

        void initialise() override;
        void shutdown() override;

        void new_frame() override;

        void set_key(KeyCodes key, bool is_down) override;
        void set_mouse_btn(MouseButtonCodes btn, bool is_down) override;

        void set_cursor_pos(const glm::vec2& pos) override;

        /* Getters */

        bool on_key_down(KeyCodes key) override;
        bool on_key_up(KeyCodes key) override;
        bool on_key_held(KeyCodes key) override;

        bool on_mouse_btn_down(MouseButtonCodes btn) override;
        bool on_mouse_btn_up(MouseButtonCodes btn) override;
        bool on_mouse_btn_held(MouseButtonCodes btn) override;

        auto get_cursor_pos() -> const glm::vec2& override;
        auto get_cursor_delta() -> glm::vec2 override;

    private:
        struct State
        {
            std::array<bool, static_cast<u16>(KeyCodes::Last)> keys{};
            std::array<bool, static_cast<u8>(MouseButtonCodes::MouseLast)> btns{};
            glm::vec2 cursorPos{};
        };
        State m_currentState{};
        State m_lastState{};
    };
}