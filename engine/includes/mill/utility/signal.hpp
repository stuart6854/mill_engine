#pragma once

#include <map>
#include <functional>

namespace mill
{
    /**
     * @brief A signal object may call multiple slots with the same signature. You can connect functions to the signal
     * which will be called when the emit() method on the signal object is invoked. Any arguments passed to emit()
     * will be passed to the given function.
     */
    template <typename... Args>
    class Signal
    {
    public:
        Signal() = default;
        Signal(const Signal&) {}
        Signal(Signal&& other) noexcept : m_slots(std::move(other.m_slots)), m_currentId(other.m_currentId) {}
        ~Signal() = default;

        /* Commands */

        /**
         * @brief Connects a std::function to the signal. The returned valued can be used to disconnect the function again.
         * @param slot
         * @return
         */
        auto connect(const std::function<void(Args...)>& slot) const -> int
        {
            m_slots.insert(std::make_pair(++m_currentId, slot));
            return m_currentId;
        }

        /**
         * @brief Convenience method to connect a member function of an object to this signal.
         * @tparam T
         * @param inst
         * @param func
         * @return
         */
        template <typename T>
        auto connect_member(T* inst, void (T::*func)(Args...)) -> int
        {
            return connect([=](Args... args) { (inst->*func)(args...); });
        }

        /**
         * @brief Convenience method to connect a const member function of an object to this signal.
         * @tparam T
         * @param inst
         * @param func
         * @return
         */
        template <typename T>
        auto connect_member(T* inst, void (T::*func)(Args...) const) -> int
        {
            return connect([=](Args... args) { (inst->*func)(args...); });
        }

        /**
         * @brief Disconnects a previously connected function.
         * @param id
         */
        void disconnect(int id) const
        {
            m_slots.erase(id);
        }

        /**
         * @brief Disconnects all previously connected functions.
         */
        void disconnect_all() const
        {
            m_slots.clear();
        }

        /**
         * @brief Calls all connected functions.
         * @tparam ...Args
         */
        void emit(Args... p)
        {
            for (const auto& it : m_slots)
            {
                it.second(p...);
            }
        }

        /**
         * @brief Calls all connected functions except for one.
         * @param excluded_connection_id
         * @param ...p
         */
        void emit_for_all_but_one(int excluded_connection_id, Args... p)
        {
            for (const auto& it : m_slots)
            {
                if (it.first != excluded_connection_id)
                {
                    it.second(p...);
                }
            }
        }

        /**
         * @brief Calls only one connected function.
         * @param excluded_connection_id
         * @param ...p
         */
        void emit_for(int connection_id, Args... p)
        {
            const auto& it = m_slots.find(connection_id);
            if (it != m_slots.end())
            {
                it.second(p...);
            }
        }

        /* Operators */

        auto operator=(const Signal& other) -> Signal&
        {
            if (this != &other)
            {
                disconnect_all();
            }
            return *this;
        }

        auto operator=(Signal&& other) noexcept -> Signal&
        {
            if (this != &other)
            {
                m_slots = std::move(other.m_slots);
                m_currentId = other.m_currentId;
            }
            return *this;
        }

    private:
        mutable std::map<int, std::function<void(Args...)>> m_slots{};
        mutable int m_currentId{ 0 };
    };
}