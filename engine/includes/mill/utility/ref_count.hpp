#pragma once

#include "mill/core/base.hpp"

namespace mill
{
    class RefCount
    {
    public:
        RefCount() : m_count(new u32(1)) {}
        RefCount(const RefCount& other) : m_count(other.m_count)
        {
            if (m_count != nullptr)
            {
                ++(*m_count);
            }
        }
        RefCount(RefCount&& other) noexcept
        {
            std::swap(m_count, other.m_count);
        }
        ~RefCount()
        {
            --(*m_count);
            if (*m_count == 0)
            {
                delete m_count;
            }
        }

        /* Getters */

        auto get_count() const -> u32
        {
            return *m_count;
        }

        /* Operators */

        auto operator=(const RefCount& rhs) -> RefCount&
        {
            if (m_count != nullptr)
            {
                --(*m_count);
                if (*m_count == 0)
                {
                    delete m_count;
                }
            }

            m_count = rhs.m_count;
            if (m_count != nullptr)
            {
                ++(*m_count);
            }
            return *this;
        }

        auto operator=(RefCount&& rhs) noexcept -> RefCount&
        {
            std::swap(m_count, rhs.m_count);
            return *this;
        }

    private:
        u32* m_count{ nullptr };
    };
}