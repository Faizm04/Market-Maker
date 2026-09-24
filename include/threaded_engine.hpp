#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "market_event.hpp"
#include "spsc_ring_buffer.hpp"
#include "timed_market_event.hpp"
#include "trading_engine.hpp"

enum class ThreadedRunMode
{
    Throughput,
    Latency
};

enum class ProducerMode
{
    Unpaced,
    Paced,
    Bursty
};

struct ProducerConfig
{
    ProducerMode mode{
        ProducerMode::Unpaced
    };

    std::uint64_t interval_ns{
        0
    };

    std::size_t burst_size{
        0
    };

    std::uint64_t burst_gap_ns{
        0
    };
};

class ThreadedEngine
{
public:
    explicit ThreadedEngine(
        TradingEngine& engine
    );

    void run(
        const std::vector<MarketEvent>& events,
        Timestamp starting_time,
        ThreadedRunMode mode = ThreadedRunMode::Throughput,
        ProducerConfig producer_config = {}
    );

    [[nodiscard]]
    std::size_t processed_events() const
    {
        return processed_events_;
    }

    [[nodiscard]]
    std::size_t producer_stalls() const
    {
        return producer_stalls_;
    }

    [[nodiscard]]
    std::size_t consumer_empty_spins() const
    {
        return consumer_empty_spins_;
    }

    [[nodiscard]]
    std::size_t queue_capacity() const
    {
        return market_event_queue_.capacity();
    }

    [[nodiscard]]
    std::size_t queue_high_water_mark() const
    {
        return queue_high_water_mark_;
    }

    [[nodiscard]]
    std::size_t overload_count() const
    {
        return overload_count_;
    }

    [[nodiscard]]
    bool market_data_overloaded() const
    {
        return market_data_overloaded_.load(
            std::memory_order_acquire
        );
    }

    [[nodiscard]]
    const std::vector<std::uint64_t>& producer_wait_latencies() const
    {
        return producer_wait_latencies_;
    }

    [[nodiscard]]
    const std::vector<std::uint64_t>& queue_residence_latencies() const
    {
        return queue_residence_latencies_;
    }

    [[nodiscard]]
    const std::vector<std::uint64_t>& engine_latencies() const
    {
        return engine_latencies_;
    }

    [[nodiscard]]
    const std::vector<std::uint64_t>& end_to_end_latencies() const
    {
        return end_to_end_latencies_;
    }

private:
    void producer_loop(
        const std::vector<MarketEvent>& events
    );

    void consumer_loop(
        Timestamp starting_time
    );

    void publish_message(
        TimedMarketEvent& message,
        std::size_t& stalls,
        std::size_t& high_water_mark,
        std::size_t& overload_count,
        bool& overloaded
    );

    static constexpr std::size_t QUEUE_CAPACITY =
        4096;

    static constexpr std::size_t STALE_QUEUE_THRESHOLD =
        1024;

    static constexpr std::size_t OVERLOAD_RECOVERY_THRESHOLD =
        STALE_QUEUE_THRESHOLD / 2;

    TradingEngine& engine_;

    SpscRingBuffer<
        TimedMarketEvent,
        QUEUE_CAPACITY
    > market_event_queue_;

    std::atomic<bool> producer_done_{false};

    std::atomic<bool> market_data_overloaded_{false};

    ThreadedRunMode mode_{
        ThreadedRunMode::Throughput
    };

    ProducerConfig producer_config_{};

    std::size_t processed_events_{0};

    std::size_t producer_stalls_{0};

    std::size_t consumer_empty_spins_{0};

    std::size_t queue_high_water_mark_{0};

    std::size_t overload_count_{0};

    std::vector<std::uint64_t> producer_wait_latencies_;

    std::vector<std::uint64_t> queue_residence_latencies_;

    std::vector<std::uint64_t> engine_latencies_;

    std::vector<std::uint64_t> end_to_end_latencies_;
};