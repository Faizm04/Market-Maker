#pragma once

#include "market_event.hpp"

struct SimulationEvent
{
    Timestamp exchange_time{};
    Timestamp delivery_time{};
    MarketEvent market_event{};
};