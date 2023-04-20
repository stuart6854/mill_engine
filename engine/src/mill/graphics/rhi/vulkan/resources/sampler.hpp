#pragma once

#include "mill/core/base.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class Sampler
    {
    public:
        Sampler(vk::Device device);
        ~Sampler() = default;

        void set_filter(vk::Filter filter);

        void build();

        /* Getters */

        auto get_hash() const -> hasht;
        auto get_sampler() const -> const vk::Sampler&;

    private:
        auto compute_hash() const -> hasht;

    private:
        vk::Device m_device{};

        hasht m_hash{};
        vk::UniqueSampler m_sampler{};

        vk::Filter m_filter{};
    };
}