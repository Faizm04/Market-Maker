#include "thread_benchmark.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <thread>

#include "cpu_relax.hpp"
#include "spsc_ring_buffer.hpp"

ThreadBenchmarkStats ThreadBenchmark::benchmark_spsc(
    const std::vector<MarketEvent>& events
)
{
    using Clock =
        std::chrono::steady_clock;

    static constexpr std::size_t QUEUE_CAPACITY =
        4096;

    SpscRingBuffer<
        MarketEvent,
        QUEUE_CAPACITY
    > queue;

    std::atomic<bool> producer_done{
        false
    };

    std::size_t consumed =
        0;

    const auto start =
        Clock::now();

    std::thread producer(
        [&]()
        {
            for (const MarketEvent& event : events)
            {
                while (!queue.push(event))
                {
                    cpu_relax();
                }
            }

            producer_done.store(
                true,
                std::memory_order_release
            );
        }
    );

    std::thread consumer(
        [&]()
        {
            MarketEvent event;

            std::size_t local_consumed =
                0;

            while (true)
            {
                if (queue.pop(event))
                {
                    ++local_consumed;
                    continue;
                }

                const bool done =
                    producer_done.load(
                        std::memory_order_acquire
                    );

                if (
                    done
                    && queue.empty()
                )
                {
                    break;
                }

                cpu_relax();
            }

            consumed =
                local_consumed;
        }
    );

    producer.join();
    consumer.join();

    const auto end =
        Clock::now();

    const double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();

    ThreadBenchmarkStats stats;

    stats.event_count =
        consumed;

    stats.total_seconds =
        seconds;

    if (seconds > 0.0)
    {
        stats.events_per_second =
            static_cast<double>(
                consumed
            ) / seconds;
    }

    return stats;
}

ThreadBenchmarkStats
ThreadBenchmark::benchmark_single_thread_engine(
    TradingEngine& engine,
    const std::vector<MarketEvent>& events,
    Timestamp starting_time
)
{
    using Clock =
        std::chrono::steady_clock;

    Timestamp now =
        starting_time;

    const auto start =
        Clock::now();

    for (const MarketEvent& event : events)
    {
        ++now;

        engine.on_market_event(
            event,
            now,
            true
        );
    }

    const auto end =
        Clock::now();

    const double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();

    ThreadBenchmarkStats stats;

    stats.event_count =
        events.size();

    stats.total_seconds =
        seconds;

    if (seconds > 0.0)
    {
        stats.events_per_second =
            static_cast<double>(
                events.size()
            ) / seconds;
    }

    return stats;
}

ThreadBenchmarkStats
ThreadBenchmark::benchmark_threaded_engine(
    ThreadedEngine& threaded_engine,
    const std::vector<MarketEvent>& events,
    Timestamp starting_time
)
{
    using Clock =
        std::chrono::steady_clock;

    const auto start =
        Clock::now();

    threaded_engine.run(
        events,
        starting_time
    );

    const auto end =
        Clock::now();

    const double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();

    ThreadBenchmarkStats stats;

    stats.event_count =
        threaded_engine.processed_events();

    stats.total_seconds =
        seconds;

    if (seconds > 0.0)
    {
        stats.events_per_second =
            static_cast<double>(
                stats.event_count
            ) / seconds;
    }

    return stats;
}