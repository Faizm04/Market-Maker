#include "position_manager.hpp"

void PositionManager::on_fill(
    OrderId order_id,
    Side side,
    Price price,
    Quantity quantity,
    Timestamp exchange_time,
    Timestamp local_time
)
{
    fills_.push_back({order_id, side, price, quantity, exchange_time, local_time});

    const std::int64_t signed_quantity =
        side == Side::Buy
        ? static_cast<std::int64_t>(quantity)
        : -static_cast<std::int64_t>(quantity);

    const std::int64_t notional =
        static_cast<std::int64_t>(price) * static_cast<std::int64_t>(quantity);

    if (side == Side::Buy)
    {
        cash_ -= notional;
    }
    else
    {
        cash_ += notional;
    }

    const std::int64_t old_inventory = inventory_;

    const bool same_direction =
        old_inventory == 0
        || (old_inventory > 0 && signed_quantity > 0)
        || (old_inventory < 0 && signed_quantity < 0);

    if (same_direction)
    {
        const std::int64_t old_abs = old_inventory >= 0 ? old_inventory : -old_inventory;
        const std::int64_t fill_abs = signed_quantity >= 0 ? signed_quantity : -signed_quantity;
        const std::int64_t new_abs = old_abs + fill_abs;

        if (new_abs > 0)
        {
            average_entry_price_ =
                (
                    average_entry_price_ * static_cast<double>(old_abs)
                    + static_cast<double>(price) * static_cast<double>(fill_abs)
                )
                / static_cast<double>(new_abs);
        }

        inventory_ += signed_quantity;
        return;
    }

    const std::int64_t old_abs = old_inventory >= 0 ? old_inventory : -old_inventory;
    const std::int64_t fill_abs = signed_quantity >= 0 ? signed_quantity : -signed_quantity;
    const std::int64_t closed_quantity = old_abs < fill_abs ? old_abs : fill_abs;

    if (old_inventory > 0)
    {
        realized_pnl_ += static_cast<std::int64_t>(
            (static_cast<double>(price) - average_entry_price_)
            * static_cast<double>(closed_quantity)
        );
    }
    else
    {
        realized_pnl_ += static_cast<std::int64_t>(
            (average_entry_price_ - static_cast<double>(price))
            * static_cast<double>(closed_quantity)
        );
    }

    inventory_ += signed_quantity;

    if (inventory_ == 0)
    {
        average_entry_price_ = 0.0;
        return;
    }

    const bool flipped_direction =
        (old_inventory > 0 && inventory_ < 0)
        || (old_inventory < 0 && inventory_ > 0);

    if (flipped_direction)
    {
        average_entry_price_ = static_cast<double>(price);
    }
}

std::int64_t PositionManager::unrealized_pnl(Price mark_price) const
{
    if (inventory_ == 0)
    {
        return 0;
    }

    if (inventory_ > 0)
    {
        return static_cast<std::int64_t>(
            (static_cast<double>(mark_price) - average_entry_price_)
            * static_cast<double>(inventory_)
        );
    }

    const std::int64_t short_quantity = -inventory_;

    return static_cast<std::int64_t>(
        (average_entry_price_ - static_cast<double>(mark_price))
        * static_cast<double>(short_quantity)
    );
}

std::int64_t PositionManager::total_pnl(Price mark_price) const
{
    return realized_pnl_ + unrealized_pnl(mark_price);
}