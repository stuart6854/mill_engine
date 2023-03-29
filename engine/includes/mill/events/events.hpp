#pragma once

#include "mill/core/base.hpp"

#include <map>
#include <queue>

namespace mill
{
    enum class EventType : u32
    {
        eWindowClose,
        eWindowSize,
        eInputKey,
        eInputMouseBtn,
        eInputMouseMove,
        eInputMouseWheel,
    };

    struct Event
    {
        EventType type{};
        void* context{ nullptr };

        // 128 bytes
        union
        {
            char c[16];
            i8 i8[16];
            u8 u8[16];

            i16 i16[8];
            u16 u16[8];

            i32 i32[4];
            u32 u32[4];
            f32 f32[4];

            i64 i64[2];
            u64 u64[2];
            f64 f64[2];
        } data{};
    };

    using EventCallbackFn = std::function<void(const Event&)>;

    class Events
    {
    public:
        void flush_queue();

        auto subscribe(const EventCallbackFn& callback) -> u32;

        void post_queue(const Event& event);
        void post_immediate(const Event& event);

    private:
        std::map<int, EventCallbackFn> m_listeners{};
        u32 m_nextListenerId{ 0 };

        std::queue<Event> m_queue{};
    };

}