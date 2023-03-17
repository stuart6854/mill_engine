#include "mill/input/input.hpp"

namespace mill
{
    void InputDefault::init() {}

    void InputDefault::shutdown() {}

    void InputDefault::new_frame()
    {
        m_lastState = m_currentState;
    }

    void InputDefault::set_key(KeyCodes key, bool is_down)
    {
        m_currentState.keys[static_cast<u16>(key)] = is_down;
    }

    void InputDefault::set_mouse_btn(MouseButtonCodes btn, bool is_down)
    {
        m_currentState.btns[static_cast<u8>(btn)] = is_down;
    }

    void InputDefault::set_cursor_pos(const glm::vec2& pos)
    {
        m_currentState.cursorPos = pos;
    }

    bool InputDefault::on_key_down(KeyCodes key)
    {
        const auto current = m_currentState.keys[static_cast<u16>(key)];
        const auto last = m_lastState.keys[static_cast<u16>(key)];
        return !last && current;
    }

    bool InputDefault::on_key_up(KeyCodes key)
    {
        const auto current = m_currentState.keys[static_cast<u16>(key)];
        const auto last = m_lastState.keys[static_cast<u16>(key)];
        return last && !current;
    }

    bool InputDefault::on_key_held(KeyCodes key)
    {
        const auto current = m_currentState.keys[static_cast<u16>(key)];
        const auto last = m_lastState.keys[static_cast<u16>(key)];
        return last && current;
    }

    bool InputDefault::on_mouse_btn_down(MouseButtonCodes btn)
    {
        const auto current = m_currentState.btns[static_cast<u16>(btn)];
        const auto last = m_lastState.btns[static_cast<u16>(btn)];
        return !last && current;
    }

    bool InputDefault::on_mouse_btn_up(MouseButtonCodes btn)
    {
        const auto current = m_currentState.btns[static_cast<u16>(btn)];
        const auto last = m_lastState.btns[static_cast<u16>(btn)];
        return last && !current;
    }

    bool InputDefault::on_mouse_btn_held(MouseButtonCodes btn)
    {
        const auto current = m_currentState.btns[static_cast<u16>(btn)];
        const auto last = m_lastState.btns[static_cast<u16>(btn)];
        return last && current;
    }

    auto InputDefault::get_cursor_pos() -> const glm::vec2&
    {
        return m_currentState.cursorPos;
    }

    auto InputDefault::get_cursor_delta() -> glm::vec2
    {
        return m_currentState.cursorPos - m_lastState.cursorPos;
    }

}
