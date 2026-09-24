#include "benchmark.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

namespace
{

std::uint64_t percentile(
    const std::vector<std::uint64_t>& values,
    double percentile_value
)
{
    if (values.empty())
    {
        return 0;
    }

    const double position =
        percentile_value
        * static_cast<double>(values.size() - 1);

    const std::size_t index =
        static_cast<std::size_t>(position);

    return values[index];
}

BenchmarkStats build_stats(
    std::vector<std::uint64_t> latencies,
    double total_seconds
)
{
    BenchmarkStats stats;

    stats.event_count =
        latencies.size();

    stats.total_seconds =
        total_seconds;

    if (total_seconds > 0.0)
    {
        stats.events_per_second =
            static_cast<double>(latencies.size())
            / total_seconds;
    }

    if (latencies.empty())
    {
        return stats;
    }

    std::sort(
        latencies.begin(),
        latencies.end()
    );

    const std::uint64_t total_latency =
        std::accumulate(
            latencies.begin(),
            latencies.end(),
            std::uint64_t{0}
        );

    stats.min_ns =
        latencies.front();

    stats.p50_ns =
        percentile(
            latencies,
            0.50
        );

    stats.p90_ns =
        percentile(
            latencies,
            0.90
        );

    stats.p99_ns =
        percentile(
            latencies,
            0.99
        );

    stats.p999_ns =
        percentile(
            latencies,
            0.999
        );

    stats.max_ns =
        latencies.back();

    stats.mean_ns =
        static_cast<double>(total_latency)
        / static_cast<double>(latencies.size());

    return stats;
}

}

Benchmark::Benchmark(TradingEngine& engine)
    : engine_(engine)
{
}

BenchmarkStats Benchmark::run(
    const std::vector<MarketEvent>& events,
    Timestamp starting_time
)
{
    using Clock =
        std::chrono::steady_clock;

    std::vector<std::uint64_t> latencies;

    latencies.reserve(
        events.size()
    );

    Timestamp simulated_time =
        starting_time;

    const auto benchmark_start =
        Clock::now();

    for (const MarketEvent& event : events)
    {
        ++simulated_time;

        const auto start =
            Clock::now();

        engine_.on_market_event(
            event,
            simulated_time,
            true
        );

        const auto end =
            Clock::now();

        const auto latency =
            std::chrono::duration_cast<
                std::chrono::nanoseconds
            >(
                end - start
            ).count();

        latencies.push_back(
            static_cast<std::uint64_t>(
                latency
            )
        );
    }

    const auto benchmark_end =
        Clock::now();

    const double total_seconds =
        std::chrono::duration<double>(
            benchmark_end
            - benchmark_start
        ).count();

    return build_stats(
        std::move(latencies),
        total_seconds
    );
}

BenchmarkStats Benchmark::benchmark_order_book(
    OrderBook& book,
    const std::vector<MarketEvent>& events
)
{
    using Clock =
        std::chrono::steady_clock;

    std::vector<std::uint64_t> latencies;

    latencies.reserve(
        events.size()
    );

    const auto benchmark_start =
        Clock::now();

    for (const MarketEvent& event : events)
    {
        const auto start =
            Clock::now();

        book.on_event(
            event
        );

        const auto end =
            Clock::now();

        const auto latency =
            std::chrono::duration_cast<
                std::chrono::nanoseconds
            >(
                end - start
            ).count();

        latencies.push_back(
            static_cast<std::uint64_t>(
                latency
            )
        );
    }

    const auto benchmark_end =
        Clock::now();

    const double total_seconds =
        std::chrono::duration<double>(
            benchmark_end
            - benchmark_start
        ).count();

    return build_stats(
        std::move(latencies),
        total_seconds
    );
}

AllocationBenchmarkStats Benchmark::benchmark_order_book_allocations(
    OrderBook& book,
    const std::vector<MarketEvent>& events
)
{
    AllocationTracker::reset();

    const AllocationStats before =
        AllocationTracker::stats();

    const BenchmarkStats timing =
        benchmark_order_book(
            book,
            events
        );

    const AllocationStats after =
        AllocationTracker::stats();

    AllocationBenchmarkStats result;

    result.timing =
        timing;

    result.allocations.allocations =
        after.allocations
        - before.allocations;

    result.allocations.deallocations =
        after.deallocations
        - before.deallocations;

    result.allocations.bytes_allocated =
        after.bytes_allocated
        - before.bytes_allocated;

    return result;
}

BenchmarkStats Benchmark::benchmark_strategy(
    const MarketMaker& strategy,
    const OrderBook& book,
    std::size_t iterations
)
{
    using Clock =
        std::chrono::steady_clock;

    std::vector<std::uint64_t> latencies;

    latencies.reserve(
        iterations
    );

    std::int64_t inventory = 0;

    volatile Price quote_sink = 0;

    const auto benchmark_start =
        Clock::now();

    for (std::size_t i = 0; i < iterations; ++i)
    {
        inventory =
            static_cast<std::int64_t>(
                i % 2001
            ) - 1000;

        const auto start =
            Clock::now();

        const Quote quote =
            strategy.calculate_quote(
                book,
                inventory
            );

        const auto end =
            Clock::now();

        quote_sink =
            quote.bid_price;

        const auto latency =
            std::chrono::duration_cast<
                std::chrono::nanoseconds
            >(
                end - start
            ).count();

        latencies.push_back(
            static_cast<std::uint64_t>(
                latency
            )
        );
    }

    (void) quote_sink;

    const auto benchmark_end =
        Clock::now();

    const double total_seconds =
        std::chrono::duration<double>(
            benchmark_end
            - benchmark_start
        ).count();

    return build_stats(
        std::move(latencies),
        total_seconds
    );
}

BenchmarkStats Benchmark::benchmark_order_manager(
    OrderManager& order_manager,
    const OrderBook& book,
    const Quote& quote,
    std::size_t iterations,
    Timestamp starting_time
)
{
    using Clock =
        std::chrono::steady_clock;

    std::vector<std::uint64_t> latencies;

    latencies.reserve(
        iterations
    );

    Timestamp now =
        starting_time;

    const auto benchmark_start =
        Clock::now();

    for (std::size_t i = 0; i < iterations; ++i)
    {
        ++now;

        const auto start =
            Clock::now();

        order_manager.update_quote(
            quote,
            book,
            now
        );

        const auto end =
            Clock::now();

        const auto latency =
            std::chrono::duration_cast<
                std::chrono::nanoseconds
            >(
                end - start
            ).count();

        latencies.push_back(
            static_cast<std::uint64_t>(
                latency
            )
        );
    }

    const auto benchmark_end =
        Clock::now();

    const double total_seconds =
        std::chrono::duration<double>(
            benchmark_end
            - benchmark_start
        ).count();

    return build_stats(
        std::move(latencies),
        total_seconds
    );
}

BenchmarkStats Benchmark::benchmark_order_manager_requote(
    OrderManager& order_manager,
    TradingEngine& engine,
    const OrderBook& book,
    Quote first_quote,
    Quote second_quote,
    std::size_t iterations,
    Timestamp starting_time
)
{
    using Clock =
        std::chrono::steady_clock;

    std::vector<std::uint64_t> latencies;

    latencies.reserve(
        iterations
    );

    Timestamp now =
        starting_time;

    bool use_second_quote =
        true;

    const auto benchmark_start =
        Clock::now();

    for (std::size_t i = 0; i < iterations; ++i)
    {
        const Quote& desired =
            use_second_quote
            ? second_quote
            : first_quote;

        const auto start =
            Clock::now();

        order_manager.update_quote(
            desired,
            book,
            now
        );

        now += 250'000;

        engine.process_until(
            now
        );

        now += 250'000;

        engine.process_until(
            now
        );

        const auto end =
            Clock::now();

        const auto latency =
            std::chrono::duration_cast<
                std::chrono::nanoseconds
            >(
                end - start
            ).count();

        latencies.push_back(
            static_cast<std::uint64_t>(
                latency
            )
        );

        use_second_quote =
            !use_second_quote;
    }

    const auto benchmark_end =
        Clock::now();

    const double total_seconds =
        std::chrono::duration<double>(
            benchmark_end
            - benchmark_start
        ).count();

    return build_stats(
        std::move(latencies),
        total_seconds
    );
}

AllocationBenchmarkStats
Benchmark::benchmark_order_manager_requote_allocations(
    OrderManager& order_manager,
    TradingEngine& engine,
    const OrderBook& book,
    Quote first_quote,
    Quote second_quote,
    std::size_t iterations,
    Timestamp starting_time
)
{
    AllocationTracker::reset();

    const AllocationStats before =
        AllocationTracker::stats();

    const BenchmarkStats timing =
        benchmark_order_manager_requote(
            order_manager,
            engine,
            book,
            first_quote,
            second_quote,
            iterations,
            starting_time
        );

    const AllocationStats after =
        AllocationTracker::stats();

    AllocationBenchmarkStats result;

    result.timing =
        timing;

    result.allocations.allocations =
        after.allocations
        - before.allocations;

    result.allocations.deallocations =
        after.deallocations
        - before.deallocations;

    result.allocations.bytes_allocated =
        after.bytes_allocated
        - before.bytes_allocated;

    return result;
}

void Benchmark::run_profile_workload(
    TradingEngine& engine,
    const std::vector<MarketEvent>& events,
    Timestamp starting_time,
    std::size_t repetitions
)
{
    Timestamp simulated_time =
        starting_time;

    for (std::size_t repetition = 0;
         repetition < repetitions;
         ++repetition)
    {
        for (const MarketEvent& event : events)
        {
            ++simulated_time;

            engine.on_market_event(
                event,
                simulated_time,
                true
            );
        }
    }
}

double Benchmark::benchmark_throughput_only(
    TradingEngine& engine,
    const std::vector<MarketEvent>& events,
    Timestamp starting_time,
    std::size_t repetitions
)
{
    using Clock =
        std::chrono::steady_clock;

    Timestamp simulated_time =
        starting_time;

    const auto start =
        Clock::now();

    for (std::size_t repetition = 0;
         repetition < repetitions;
         ++repetition)
    {
        for (const MarketEvent& event : events)
        {
            ++simulated_time;

            engine.on_market_event(
                event,
                simulated_time,
                true
            );
        }
    }

    const auto end =
        Clock::now();

    const double seconds =
        std::chrono::duration<double>(
            end - start
        ).count();

    if (seconds <= 0.0)
    {
        return 0.0;
    }

    const std::uint64_t total_events =
        static_cast<std::uint64_t>(
            events.size()
        )
        * static_cast<std::uint64_t>(
            repetitions
        );

    return static_cast<double>(
        total_events
    ) / seconds;
}