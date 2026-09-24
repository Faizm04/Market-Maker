#pragma once

#include <queue>
#include <vector>

#include "simulation_event.hpp"
#include "trading_engine.hpp"

struct SimulationEventCompare
{
    bool operator()(
        const SimulationEvent& lhs,
        const SimulationEvent& rhs
    ) const
    {
        return lhs.delivery_time > rhs.delivery_time;
    }
};

class Simulator
{
public:
    explicit Simulator(TradingEngine& engine);

    void schedule_market_event(
        const MarketEvent& event,
        Timestamp exchange_time
    );

    void run();

    [[nodiscard]]
    Timestamp now() const
    {
        return now_;
    }

private:
    static constexpr Timestamp MARKET_DATA_LATENCY_NS = 100'000;

    Timestamp now_{0};

    TradingEngine& engine_;

    std::priority_queue<
        SimulationEvent,
        std::vector<SimulationEvent>,
        SimulationEventCompare
    > events_;
};