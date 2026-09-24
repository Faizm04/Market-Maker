#pragma once

#include <array>
#include <atomic>
#include <cstddef>

template <typename T, std::size_t Capacity>
class SpscRingBuffer
{
    static_assert(
        Capacity >= 2,
        "SPSC capacity must be at least 2"
    );

    static_assert(
        (Capacity & (Capacity - 1)) == 0,
        "SPSC capacity must be a power of two"
    );

public:
    [[nodiscard]]
    bool push(const T& value)
    {
        const std::size_t tail =
            producer_.tail.load(
                std::memory_order_relaxed
            );

        if (
            tail - producer_.cached_head
            >= Capacity
        )
        {
            producer_.cached_head =
                consumer_.head.load(
                    std::memory_order_acquire
                );

            if (
                tail - producer_.cached_head
                >= Capacity
            )
            {
                return false;
            }
        }

        buffer_[tail & MASK] =
            value;

        producer_.tail.store(
            tail + 1,
            std::memory_order_release
        );

        return true;
    }

    [[nodiscard]]
    bool pop(T& value)
    {
        const std::size_t head =
            consumer_.head.load(
                std::memory_order_relaxed
            );

        if (
            head == consumer_.cached_tail
        )
        {
            consumer_.cached_tail =
                producer_.tail.load(
                    std::memory_order_acquire
                );
            if (
                head == consumer_.cached_tail
            )
            {
                return false;
            }
        }

        value =
            buffer_[head & MASK];

        consumer_.head.store(
            head + 1,
            std::memory_order_release
        );

        return true;
    }

    [[nodiscard]]
    bool empty() const
    {
        const std::size_t head =
            consumer_.head.load(
                std::memory_order_acquire
            );

        const std::size_t tail =
            producer_.tail.load(
                std::memory_order_acquire
            );

        return head == tail;
    }

    [[nodiscard]]
    std::size_t size() const
    {
        const std::size_t head =
            consumer_.head.load(
                std::memory_order_acquire
            );

        const std::size_t tail =
            producer_.tail.load(
                std::memory_order_acquire
            );

        return tail - head;
    }

    [[nodiscard]]
    constexpr std::size_t capacity() const
    {
        return Capacity;
    }

private:
    static constexpr std::size_t MASK =
        Capacity - 1;

    struct alignas(64) ProducerState
    {
        std::atomic<std::size_t> tail{0};
        std::size_t cached_head{0};
    };

    struct alignas(64) ConsumerState
    {
        std::atomic<std::size_t> head{0};
        std::size_t cached_tail{0};
    };

    ProducerState producer_;
    ConsumerState consumer_;

    std::array<T, Capacity> buffer_{};
};