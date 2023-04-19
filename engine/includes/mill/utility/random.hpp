#pragma once

#include "mill/core/base.hpp"

namespace mill::random
{
    /* Returns a 32-bit unsigned integer between 0 and 4294967295 (inclusive). */
    auto random_u32() -> u32;
    /* Returns a 32-bit unsigned integer between 0 and 18446744073709551615 (inclusive). */
    auto random_u64() -> u64;

    /* Returns a 32-bit signed integer between `min` and `max` (inclusive). */
    auto random_i32_range(i32 min, i32 max) -> i32;
    /* Returns a 64-bit signed integer between `min` and `max` (inclusive). */
    auto random_i64_range(i64 min, i64 max) -> i64;

    /* Returns a 32-bit signed float between 0.0 and 1.0 (inclusive). */
    auto random_f32() -> f32;
    /* Returns a 32-bit signed float between 0.0 and 1.0 (inclusive). */
    auto random_f64() -> f64;

    /* Returns a 32-bit signed float between `min` and `max` (inclusive). */
    auto random_f32_range(f32 min, f32 max) -> f32;
    /* Returns a 32-bit signed float between `min` and `max` (inclusive). */
    auto random_f64_range(f64 min, f64 max) -> f64;
}