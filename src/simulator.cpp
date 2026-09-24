#include "simulator.hpp"

Simulator::Simulator(TradingEngine& engine)
    : engine_(engine)
{
}

void Simulator::schedule_market_event(
    const MarketEvent& event,
    Timestamp exchange_time
)
{
    const Timestamp delivery_time =
        exchange_time + MARKET_DATA_LATENCY_NS;

    events_.push({
        exchange_time,
        delivery_time,
        event
    });
}

void Simulator::run()
{
    while (!events_.empty())
    {
        const SimulationEvent event = events_.top();

        events_.pop();

        engine_.on_exchange_market_execution(
            event.market_event,
            event.exchange_time
        );

        now_ = event.delivery_time;

        engine_.process_until(now_);

        engine_.on_market_event(
            event.market_event,
            now_
        );
    }

    while (engine_.has_pending_exchange_events())
    {
        const Timestamp next_time =
            engine_.next_exchange_event_time();

        if (next_time < now_)
        {
            engine_.process_until(now_);
            continue;
        }

        now_ = next_time;

        engine_.process_until(now_);
    }
}