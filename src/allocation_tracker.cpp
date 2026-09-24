#include "allocation_tracker.hpp"

#include <atomic>
#include <cstdlib>
#include <new>

namespace
{

std::atomic<std::uint64_t> allocation_count{0};
std::atomic<std::uint64_t> deallocation_count{0};
std::atomic<std::uint64_t> allocated_bytes{0};

}

void AllocationTracker::reset()
{
    allocation_count.store(
        0,
        std::memory_order_relaxed
    );

    deallocation_count.store(
        0,
        std::memory_order_relaxed
    );

    allocated_bytes.store(
        0,
        std::memory_order_relaxed
    );
}

AllocationStats AllocationTracker::stats()
{
    return {
        allocation_count.load(
            std::memory_order_relaxed
        ),
        deallocation_count.load(
            std::memory_order_relaxed
        ),
        allocated_bytes.load(
            std::memory_order_relaxed
        )
    };
}

void* operator new(std::size_t size)
{
    void* memory =
        std::malloc(size);

    if (memory == nullptr)
    {
        throw std::bad_alloc{};
    }

    allocation_count.fetch_add(
        1,
        std::memory_order_relaxed
    );

    allocated_bytes.fetch_add(
        size,
        std::memory_order_relaxed
    );

    return memory;
}

void* operator new[](std::size_t size)
{
    void* memory =
        std::malloc(size);

    if (memory == nullptr)
    {
        throw std::bad_alloc{};
    }

    allocation_count.fetch_add(
        1,
        std::memory_order_relaxed
    );

    allocated_bytes.fetch_add(
        size,
        std::memory_order_relaxed
    );

    return memory;
}

void operator delete(void* memory) noexcept
{
    if (memory == nullptr)
    {
        return;
    }

    deallocation_count.fetch_add(
        1,
        std::memory_order_relaxed
    );

    std::free(memory);
}

void operator delete[](void* memory) noexcept
{
    if (memory == nullptr)
    {
        return;
    }

    deallocation_count.fetch_add(
        1,
        std::memory_order_relaxed
    );

    std::free(memory);
}

void operator delete(
    void* memory,
    std::size_t
) noexcept
{
    if (memory == nullptr)
    {
        return;
    }

    deallocation_count.fetch_add(
        1,
        std::memory_order_relaxed
    );

    std::free(memory);
}

void operator delete[](
    void* memory,
    std::size_t
) noexcept
{
    if (memory == nullptr)
    {
        return;
    }

    deallocation_count.fetch_add(
        1,
        std::memory_order_relaxed
    );

    std::free(memory);
}