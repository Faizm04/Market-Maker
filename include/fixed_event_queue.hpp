#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>

#include "execution_event.hpp"

template <std::size_t Capacity>
class FixedEventQueue
{
public:
    void push(const ScheduledExecution& event)
    {
        if (size_ >= Capacity)
        {
            throw std::runtime_error(
                "FixedEventQueue capacity exceeded"
            );
        }

        events_[size_] = event;
        ++size_;
    }

    [[nodiscard]]
    bool empty() const
    {
        return size_ == 0;
    }

    [[nodiscard]]
    std::size_t size() const
    {
        return size_;
    }

    [[nodiscard]]
    const ScheduledExecution& top() const
    {
        if (empty())
        {
            throw std::runtime_error(
                "FixedEventQueue is empty"
            );
        }

        return events_[find_earliest_index()];
    }

    ScheduledExecution pop()
    {
        if (empty())
        {
            throw std::runtime_error(
                "FixedEventQueue is empty"
            );
        }

        const std::size_t index =
            find_earliest_index();

        const ScheduledExecution result =
            events_[index];

        events_[index] =
            events_[size_ - 1];

        --size_;

        return result;
    }

private:
    [[nodiscard]]
    std::size_t find_earliest_index() const
    {
        std::size_t earliest = 0;

        for (std::size_t i = 1; i < size_; ++i)
        {
            if (
                events_[i].delivery_time
                < events_[earliest].delivery_time
            )
            {
                earliest = i;
            }
        }

        return earliest;
    }

    std::array<ScheduledExecution, Capacity> events_{};
    std::size_t size_{0};
};