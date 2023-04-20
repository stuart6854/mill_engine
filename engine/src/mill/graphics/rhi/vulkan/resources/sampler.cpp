#include "sampler.hpp"

#include "mill/core/base.hpp"

namespace mill::rhi
{
    Sampler::Sampler(vk::Device device) : m_device(device) {}

    void Sampler::set_filter(vk::Filter filter)
    {
        m_filter = filter;
    }

    void Sampler::build()
    {
        vk::SamplerCreateInfo sampler_info{};
        sampler_info.setAddressModeU(vk::SamplerAddressMode::eRepeat);
        sampler_info.setAddressModeV(vk::SamplerAddressMode::eRepeat);
        sampler_info.setAddressModeW(vk::SamplerAddressMode::eRepeat);
        sampler_info.setMinFilter(m_filter);
        sampler_info.setMagFilter(m_filter);
        sampler_info.setMipmapMode(vk::SamplerMipmapMode::eLinear);
        m_sampler = m_device.createSamplerUnique(sampler_info);

        m_hash = compute_hash();
    }

    auto Sampler::get_hash() const -> hasht
    {
        if (m_hash)
            return m_hash;

        return compute_hash();
    }

    auto Sampler::get_sampler() const -> const vk::Sampler&
    {
        return m_sampler.get();
    }

    auto Sampler::compute_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, m_filter);

        return hash;
    }

}
