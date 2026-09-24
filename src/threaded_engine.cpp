#include "threaded_engine.hpp"

#include <functional>
#include <thread>

#include "cpu_relax.hpp"
#include "latency_clock.hpp"
#include "thread_config.hpp"

ThreadedEngine::ThreadedEngine(
    TradingEngine& engine
)
    : engine_(engine)
{
}

void ThreadedEngine::publish_message(
    TimedMarketEvent& message,
    std::size_t& stalls,
    std::size_t& high_water_mark,
    std::size_t& overload_count,
    bool& overloaded
)
{
    while (true)
    {
        if (mode_ == ThreadedRunMode::Latency)
        {
            message.publish_timestamp_ns =
                latency_now_ns();
        }

        if (market_event_queue_.push(message))
        {
            break;
        }

        ++stalls;

        cpu_relax();
    }

    const std::size_t depth =
        market_event_queue_.size();

    if (depth > high_water_mark)
    {
        high_water_mark =
            depth;
    }

    if (
        depth >= STALE_QUEUE_THRESHOLD
        && !overloaded
    )
    {
        overloaded =
            true;

        ++overload_count;

        market_data_overloaded_.store(
            true,
            std::memory_order_release
        );
    }
    else if (
        depth < OVERLOAD_RECOVERY_THRESHOLD
        && overloaded
    )
    {
        overloaded =
            false;

        market_data_overloaded_.store(
            false,
            std::memory_order_release
        );
    }
}

void ThreadedEngine::producer_loop(
    const std::vector<MarketEvent>& events
)
{
    configure_current_thread(
        ThreadRole::MarketData
    );

    std::size_t stalls =
        0;

    std::size_t high_water_mark =
        0;

    std::size_t overload_count =
        0;

    bool overloaded =
        false;

    std::uint64_t next_event_time =
        latency_now_ns();

    std::uint64_t next_burst_time =
        next_event_time;

    std::size_t events_in_burst =
        0;

    for (const MarketEvent& event : events)
    {
        if (
            producer_config_.mode
            == ProducerMode::Paced
        )
        {
            spin_until_ns(
                next_event_time
            );
        }

        TimedMarketEvent message;

        message.event =
            event;

        if (mode_ == ThreadedRunMode::Latency)
        {
            message.ready_timestamp_ns =
                latency_now_ns();

            message.publish_timestamp_ns =
                message.ready_timestamp_ns;
        }

        publish_message(
            message,
            stalls,
            high_water_mark,
            overload_count,
            overloaded
        );

        if (
            producer_config_.mode
            == ProducerMode::Paced
        )
        {
            next_event_time +=
                producer_config_.interval_ns;
        }
        else if (
            producer_config_.mode
            == ProducerMode::Bursty
        )
        {
            ++events_in_burst;

            if (
                producer_config_.burst_size > 0
                && events_in_burst
                    >= producer_config_.burst_size
            )
            {
                next_burst_time =
                    latency_now_ns()
                    + producer_config_.burst_gap_ns;

                spin_until_ns(
                    next_burst_time
                );

                events_in_burst =
                    0;
            }
        }
    }

    producer_stalls_ =
        stalls;

    queue_high_water_mark_ =
        high_water_mark;

    overload_count_ =
        overload_count;

    producer_done_.store(
        true,
        std::memory_order_release
    );
}

void ThreadedEngine::consumer_loop(
    Timestamp starting_time
)
{
    configure_current_thread(
        ThreadRole::Trading
    );

    Timestamp local_time =
        starting_time;

    std::size_t processed =
        0;

    std::size_t empty_spins =
        0;

    TimedMarketEvent message;

    if (mode_ == ThreadedRunMode::Latency)
    {
        while (true)
        {
            if (
                market_event_queue_.pop(
                    message
                )
            )
            {
                ++local_time;

                const std::uint64_t consumer_start =
                    latency_now_ns();

                engine_.on_market_event(
                    message.event,
                    local_time,
                    true
                );

                const std::uint64_t consumer_end =
                    latency_now_ns();

                producer_wait_latencies_.push_back(
                    message.publish_timestamp_ns
                    - message.ready_timestamp_ns
                );

                queue_residence_latencies_.push_back(
                    consumer_start
                    - message.publish_timestamp_ns
                );

                engine_latencies_.push_back(
                    consumer_end
                    - consumer_start
                );

                end_to_end_latencies_.push_back(
                    consumer_end
                    - message.ready_timestamp_ns
                );

                ++processed;

                continue;
            }

            const bool done =
                producer_done_.load(
                    std::memory_order_acquire
                );

            if (
                done
                && market_event_queue_.empty()
            )
            {
                break;
            }

            ++empty_spins;

            cpu_relax();
        }
    }
    else
    {
        while (true)
        {
            if (
                market_event_queue_.pop(
                    message
                )
            )
            {
                ++local_time;

                engine_.on_market_event(
                    message.event,
                    local_time,
                    true
                );

                ++processed;

                continue;
            }

            const bool done =
                producer_done_.load(
                    std::memory_order_acquire
                );

            if (
                done
                && market_event_queue_.empty()
            )
            {
                break;
            }

            ++empty_spins;

            cpu_relax();
        }
    }

    processed_events_ =
        processed;

    consumer_empty_spins_ =
        empty_spins;
}

void ThreadedEngine::run(
    const std::vector<MarketEvent>& events,
    Timestamp starting_time,
    ThreadedRunMode mode,
    ProducerConfig producer_config
)
{
    mode_ =
        mode;

    producer_config_ =
        producer_config;

    producer_done_.store(
        false,
        std::memory_order_relaxed
    );

    market_data_overloaded_.store(
        false,
        std::memory_order_relaxed
    );

    processed_events_ =
        0;

    producer_stalls_ =
        0;

    consumer_empty_spins_ =
        0;

    queue_high_water_mark_ =
        0;

    overload_count_ =
        0;

    producer_wait_latencies_.clear();

    queue_residence_latencies_.clear();

    engine_latencies_.clear();

    end_to_end_latencies_.clear();

    if (mode_ == ThreadedRunMode::Latency)
    {
        producer_wait_latencies_.reserve(
            events.size()
        );

        queue_residence_latencies_.reserve(
            events.size()
        );

        engine_latencies_.reserve(
            events.size()
        );

        end_to_end_latencies_.reserve(
            events.size()
        );
    }

    std::thread producer(
        &ThreadedEngine::producer_loop,
        this,
        std::cref(events)
    );

    std::thread consumer(
        &ThreadedEngine::consumer_loop,
        this,
        starting_time
    );

    producer.join();
    consumer.join();
}