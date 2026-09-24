#pragma once

#include <cstddef>
#include <vector>

#include "market_event.hpp"
#include "threaded_engine.hpp"
#include "trading_engine.hpp"
#include "types.hpp"

struct ThreadBenchmarkStats
{
    std::size_t event_count{};
    double total_seconds{};
    double events_per_second{};
};

class ThreadBenchmark
{
public:
    [[nodiscard]]
    static ThreadBenchmarkStats benchmark_spsc(
        const std::vector<MarketEvent>& events
    );

    [[nodiscard]]
    static ThreadBenchmarkStats benchmark_single_thread_engine(
        TradingEngine& engine,
        const std::vector<MarketEvent>& events,
        Timestamp starting_time
    );

    [[nodiscard]]
    static ThreadBenchmarkStats benchmark_threaded_engine(
        ThreadedEngine& threaded_engine,
        const std::vector<MarketEvent>& events,
        Timestamp starting_time
    );
};