#include "mill/events/events.hpp"

namespace mill
{
    void Events::flush_queue()
    {
        while (!m_queue.empty())
        {
            const auto& event = m_queue.front();
            post_immediate(event);
            m_queue.pop();
        }
    }

    auto Events::subscribe(const EventCallbackFn& callback) -> u32
    {
        const auto id = m_nextListenerId++;
        m_listeners[id] = callback;
        return id;
    }

    void Events::post_queue(const Event& event)
    {
        m_queue.push(event);
    }

    void Events::post_immediate(const Event& event)
    {
        for (const auto& [_, callback] : m_listeners)
        {
            callback(event);
        }
    }

}
