#pragma once

#include <vector>
#include <cstddef>
#include "position_manager.hpp"
#include "types.hpp"

struct MarketSnapshot
{
    Timestamp timestamp{};
    double midpoint{};
};

struct MarkoutResult
{
    OrderId order_id{};
    Side side{};
    Price fill_price{};
    Quantity quantity{};
    Timestamp fill_time{};
    Timestamp horizon{};
    double future_midpoint{};
    double markout_per_share{};
};

class AnalyticsManager
{
public:
    void record_market_snapshot(Timestamp timestamp, double midpoint);

    [[nodiscard]]
    std::vector<MarkoutResult> calculate_markouts(
        const std::vector<FillRecord>& fills,
        Timestamp horizon
    ) const;

    [[nodiscard]]
    double average_markout(
        const std::vector<FillRecord>& fills,
        Timestamp horizon
    ) const;

    [[nodiscard]]
    double spread_capture(const FillRecord& fill) const;

    [[nodiscard]]
    const std::vector<MarketSnapshot>& snapshots() const
    {
        return snapshots_;
    }

    void reserve_snapshots(std::size_t count) {
        snapshots_.reserve(count);
    }
private:
    [[nodiscard]]
    const MarketSnapshot* find_snapshot_at_or_after(Timestamp timestamp) const;

    std::vector<MarketSnapshot> snapshots_;
};