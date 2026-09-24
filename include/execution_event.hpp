#pragma once

#include <cstdint>

#include "types.hpp"

enum class ExecutionType : std::uint8_t
{
    Accepted,
    Cancelled,
    PartiallyFilled,
    Filled,
    Rejected
};

struct ExecutionReport
{
    ExecutionType type{};
    OrderId order_id{};
    Side side{};
    Price price{};
    Quantity quantity{};
    Timestamp exchange_time{};
};

struct ScheduledExecution
{
    Timestamp delivery_time{};
    ExecutionReport report{};
};