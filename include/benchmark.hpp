#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "allocation_tracker.hpp"
#include "market_event.hpp"
#include "order_book.hpp"
#include "order_manager.hpp"
#include "strategy.hpp"
#include "trading_engine.hpp"

struct BenchmarkStats
{
    std::uint64_t event_count{};
    double total_seconds{};
    double events_per_second{};
    std::uint64_t min_ns{};
    std::uint64_t p50_ns{};
    std::uint64_t p90_ns{};
    std::uint64_t p99_ns{};
    std::uint64_t p999_ns{};
    std::uint64_t max_ns{};
    double mean_ns{};
};

struct AllocationBenchmarkStats
{
    BenchmarkStats timing;
    AllocationStats allocations;
};

class Benchmark
{
public:
    explicit Benchmark(TradingEngine& engine);

    [[nodiscard]]
    BenchmarkStats run(
        const std::vector<MarketEvent>& events,
        Timestamp starting_time
    );

    [[nodiscard]]
    static BenchmarkStats benchmark_order_book(
        OrderBook& book,
        const std::vector<MarketEvent>& events
    );

    [[nodiscard]]
    static AllocationBenchmarkStats benchmark_order_book_allocations(
        OrderBook& book,
        const std::vector<MarketEvent>& events
    );

    [[nodiscard]]
    static BenchmarkStats benchmark_strategy(
        const MarketMaker& strategy,
        const OrderBook& book,
        std::size_t iterations
    );

    [[nodiscard]]
    static BenchmarkStats benchmark_order_manager(
        OrderManager& order_manager,
        const OrderBook& book,
        const Quote& quote,
        std::size_t iterations,
        Timestamp starting_time
    );

    [[nodiscard]]
    static BenchmarkStats benchmark_order_manager_requote(
        OrderManager& order_manager,
        TradingEngine& engine,
        const OrderBook& book,
        Quote first_quote,
        Quote second_quote,
        std::size_t iterations,
        Timestamp starting_time
    );

    [[nodiscard]]
    static AllocationBenchmarkStats benchmark_order_manager_requote_allocations(
        OrderManager& order_manager,
        TradingEngine& engine,
        const OrderBook& book,
        Quote first_quote,
        Quote second_quote,
        std::size_t iterations,
        Timestamp starting_time
    );

    static void run_profile_workload(
        TradingEngine& engine,
        const std::vector<MarketEvent>& events,
        Timestamp starting_time,
        std::size_t repetitions
    );

    [[nodiscard]]
    static double benchmark_throughput_only(
        TradingEngine& engine,
        const std::vector<MarketEvent>& events,
        Timestamp starting_time,
        std::size_t repetitions
    );

private:
    TradingEngine& engine_;
};