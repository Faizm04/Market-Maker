#pragma once

#include <cstdint>
#include <vector>

#include "types.hpp"

struct FillRecord
{
    OrderId order_id{};
    Side side{};
    Price price{};
    Quantity quantity{};
    Timestamp exchange_time{};
    Timestamp local_time{};
};

class PositionManager
{
public:
    void on_fill(
        OrderId order_id,
        Side side,
        Price price,
        Quantity quantity,
        Timestamp exchange_time,
        Timestamp local_time
    );

    [[nodiscard]]
    std::int64_t inventory() const
    {
        return inventory_;
    }

    [[nodiscard]]
    std::int64_t cash() const
    {
        return cash_;
    }

    [[nodiscard]]
    double average_entry_price() const
    {
        return average_entry_price_;
    }

    [[nodiscard]]
    std::int64_t realized_pnl() const
    {
        return realized_pnl_;
    }

    [[nodiscard]]
    std::int64_t unrealized_pnl(Price mark_price) const;

    [[nodiscard]]
    std::int64_t total_pnl(Price mark_price) const;

    [[nodiscard]]
    const std::vector<FillRecord>& fills() const
    {
        return fills_;
    }

private:
    std::int64_t inventory_{0};
    std::int64_t cash_{0};
    double average_entry_price_{0.0};
    std::int64_t realized_pnl_{0};
    std::vector<FillRecord> fills_;
};