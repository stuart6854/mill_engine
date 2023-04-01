#pragma once

namespace mill
{
    /*
        To be used with enum class to define binary bit flags.
        Note: This is inspired by Vulkan-HPP Flags.
     */
    template <typename BitType>
    class Flags
    {
    public:
        using MaskType = typename std::underlying_type<BitType>::type;

        constexpr Flags() = default;
        constexpr Flags(BitType bit) : m_mask(static_cast<MaskType>(bit)) {}
        constexpr Flags(const Flags<BitType>& rhs) = default;
        constexpr explicit Flags(MaskType flags) noexcept : m_mask(flags) {}
        ~Flags() = default;

        /* Operators */

        constexpr bool operator!() const noexcept
        {
            return !m_mask;
        }

        constexpr auto operator&(const Flags<BitType>& rhs) const noexcept -> Flags<BitType>
        {
            return Flags<BitType>(m_mask & rhs.m_mask);
        }

        constexpr auto operator|(const Flags<BitType>& rhs) const noexcept -> Flags<BitType>
        {
            return { m_mask | rhs.m_mask };
        }

        constexpr auto operator^(const Flags<BitType>& rhs) const noexcept -> Flags<BitType>
        {
            return { m_mask ^ rhs.m_mask };
        }

        constexpr auto operator=(const Flags<BitType>& rhs) noexcept -> Flags<BitType>& = default;

        constexpr auto operator|=(const Flags<BitType>& rhs) -> Flags<BitType>
        {
            m_mask |= rhs.m_mask;
            return *this;
        }

        constexpr auto operator&=(const Flags<BitType>& rhs) -> Flags<BitType>
        {
            m_mask &= rhs.m_mask;
            return *this;
        }

        constexpr auto operator^=(const Flags<BitType>& rhs) -> Flags<BitType>
        {
            m_mask ^= rhs.m_mask;
            return *this;
        }

        explicit constexpr operator bool() const noexcept
        {
            return !!m_mask;
        }

        explicit constexpr operator MaskType() const noexcept
        {
            return m_mask;
        }

    private:
        MaskType m_mask{ 0 };
    };
}