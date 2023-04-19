#include "mill/utility/random.hpp"

#include <random>

namespace mill::random
{
    static std::random_device s_randDevice{};
    static std::mt19937_64 s_gen32(s_randDevice());
    static std::mt19937_64 s_gen64(s_randDevice());

    auto random_u32() -> u32
    {
        std::uniform_int_distribution<u32> distrib{};
        return distrib(s_gen32);
    }

    auto random_u64() -> u64
    {
        std::uniform_int_distribution<u64> distrib{};
        return distrib(s_gen64);
    }

    auto random_i32_range(i32 min, i32 max) -> i32
    {
        std::uniform_int_distribution<i32> distrib(min, max);
        return distrib(s_gen32);
    }

    auto random_i64_range(i64 min, i64 max) -> i64
    {
        std::uniform_int_distribution<i64> distrib(min, max);
        return distrib(s_gen64);
    }

    auto random_f32() -> f32
    {
        std::uniform_real_distribution<f32> distrib(0.0f, 1.0f);
        return distrib(s_gen32);
    }

    auto random_f64() -> f64
    {
        std::uniform_real_distribution<f64> distrib(0.0f, 1.0f);
        return distrib(s_gen64);
    }

    auto random_f32_range(f32 min, f32 max) -> f32
    {
        std::uniform_real_distribution<f32> distrib(min, max);
        return distrib(s_gen32);
    }

    auto random_f64_range(f64 min, f64 max) -> f64
    {
        std::uniform_real_distribution<f64> distrib(min, max);
        return distrib(s_gen64);
    }
}
