#pragma once

#include <cstdint>

#include "market_event.hpp"

struct TimedMarketEvent
{
    MarketEvent event{};

    std::uint64_t ready_timestamp_ns{};
    std::uint64_t publish_timestamp_ns{};
};