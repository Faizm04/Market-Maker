#include "pipeline_latency.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{

std::uint64_t percentile(
    const std::vector<std::uint64_t>& values,
    double percentile_value
)
{
    if (values.empty())
    {
        return 0;
    }

    const double position =
        percentile_value
        * static_cast<double>(values.size() - 1);

    const std::size_t index =
        static_cast<std::size_t>(position);

    return values[index];
}

}

PipelineLatencyStats calculate_pipeline_latency_stats(
    std::vector<std::uint64_t> producer_wait_latencies,
    std::vector<std::uint64_t> queue_residence_latencies,
    std::vector<std::uint64_t> engine_latencies,
    std::vector<std::uint64_t> end_to_end_latencies
)
{
    PipelineLatencyStats stats;

    if (
        producer_wait_latencies.empty()
        || queue_residence_latencies.empty()
        || engine_latencies.empty()
        || end_to_end_latencies.empty()
    )
    {
        return stats;
    }

    std::sort(
        producer_wait_latencies.begin(),
        producer_wait_latencies.end()
    );

    std::sort(
        queue_residence_latencies.begin(),
        queue_residence_latencies.end()
    );

    std::sort(
        engine_latencies.begin(),
        engine_latencies.end()
    );

    std::sort(
        end_to_end_latencies.begin(),
        end_to_end_latencies.end()
    );

    stats.samples =
        static_cast<std::uint64_t>(
            end_to_end_latencies.size()
        );

    stats.producer_wait_p50_ns =
        percentile(
            producer_wait_latencies,
            0.50
        );

    stats.producer_wait_p99_ns =
        percentile(
            producer_wait_latencies,
            0.99
        );

    stats.producer_wait_p999_ns =
        percentile(
            producer_wait_latencies,
            0.999
        );

    stats.queue_residence_p50_ns =
        percentile(
            queue_residence_latencies,
            0.50
        );

    stats.queue_residence_p99_ns =
        percentile(
            queue_residence_latencies,
            0.99
        );

    stats.queue_residence_p999_ns =
        percentile(
            queue_residence_latencies,
            0.999
        );

    stats.engine_p50_ns =
        percentile(
            engine_latencies,
            0.50
        );

    stats.engine_p99_ns =
        percentile(
            engine_latencies,
            0.99
        );

    stats.engine_p999_ns =
        percentile(
            engine_latencies,
            0.999
        );

    stats.end_to_end_p50_ns =
        percentile(
            end_to_end_latencies,
            0.50
        );

    stats.end_to_end_p99_ns =
        percentile(
            end_to_end_latencies,
            0.99
        );

    stats.end_to_end_p999_ns =
        percentile(
            end_to_end_latencies,
            0.999
        );

    return stats;
}