#pragma once

#include <chrono>
#include <cstdint>

#include "cpu_relax.hpp"

inline std::uint64_t latency_now_ns()
{
    using Clock = std::chrono::steady_clock;

    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            Clock::now().time_since_epoch()
        ).count()
    );
}

inline void spin_until_ns(std::uint64_t target_ns)
{
    while (latency_now_ns() < target_ns)
    {
        cpu_relax();
    }
}