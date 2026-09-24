#include "analytics_manager.hpp"

#include <algorithm>
#include <cstdint>

void AnalyticsManager::record_market_snapshot(Timestamp timestamp, double midpoint)
{
    snapshots_.push_back({timestamp, midpoint});
}

const MarketSnapshot* AnalyticsManager::find_snapshot_at_or_after(Timestamp timestamp) const
{
    const auto it = std::lower_bound(
        snapshots_.begin(),
        snapshots_.end(),
        timestamp,
        [](const MarketSnapshot& snapshot, Timestamp target)
        {
            return snapshot.timestamp < target;
        }
    );

    if (it == snapshots_.end())
    {
        return nullptr;
    }

    return &(*it);
}

std::vector<MarkoutResult> AnalyticsManager::calculate_markouts(
    const std::vector<FillRecord>& fills,
    Timestamp horizon
) const
{
    std::vector<MarkoutResult> results;

    results.reserve(fills.size());

    for (const FillRecord& fill : fills)
    {
        const Timestamp target_time = fill.local_time + horizon;

        const MarketSnapshot* snapshot =
            find_snapshot_at_or_after(target_time);

        if (snapshot == nullptr)
        {
            continue;
        }

        const double fill_price = static_cast<double>(fill.price);

        const double markout =
            fill.side == Side::Buy
            ? snapshot->midpoint - fill_price
            : fill_price - snapshot->midpoint;

        results.push_back({
            fill.order_id,
            fill.side,
            fill.price,
            fill.quantity,
            fill.local_time,
            horizon,
            snapshot->midpoint,
            markout
        });
    }

    return results;
}

double AnalyticsManager::average_markout(
    const std::vector<FillRecord>& fills,
    Timestamp horizon
) const
{
    const std::vector<MarkoutResult> results =
        calculate_markouts(fills, horizon);

    if (results.empty())
    {
        return 0.0;
    }

    double weighted_sum = 0.0;
    std::uint64_t total_quantity = 0;

    for (const MarkoutResult& result : results)
    {
        weighted_sum +=
            result.markout_per_share * static_cast<double>(result.quantity);

        total_quantity += result.quantity;
    }

    if (total_quantity == 0)
    {
        return 0.0;
    }

    return weighted_sum / static_cast<double>(total_quantity);
}

double AnalyticsManager::spread_capture(const FillRecord& fill) const
{
    const MarketSnapshot* snapshot =
        find_snapshot_at_or_after(fill.local_time);

    if (snapshot == nullptr)
    {
        return 0.0;
    }

    if (fill.side == Side::Buy)
    {
        return snapshot->midpoint - static_cast<double>(fill.price);
    }

    return static_cast<double>(fill.price) - snapshot->midpoint;
}