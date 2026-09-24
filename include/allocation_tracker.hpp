#pragma once

#include <cstdint>

struct AllocationStats
{
    std::uint64_t allocations{};
    std::uint64_t deallocations{};
    std::uint64_t bytes_allocated{};
};

class AllocationTracker
{
public:
    static void reset();

    [[nodiscard]]
    static AllocationStats stats();
};