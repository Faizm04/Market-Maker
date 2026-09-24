#pragma once

#include <cstdint>
#include <vector>

struct PipelineLatencyStats
{
    std::uint64_t samples{};

    std::uint64_t producer_wait_p50_ns{};
    std::uint64_t producer_wait_p99_ns{};
    std::uint64_t producer_wait_p999_ns{};

    std::uint64_t queue_residence_p50_ns{};
    std::uint64_t queue_residence_p99_ns{};
    std::uint64_t queue_residence_p999_ns{};

    std::uint64_t engine_p50_ns{};
    std::uint64_t engine_p99_ns{};
    std::uint64_t engine_p999_ns{};

    std::uint64_t end_to_end_p50_ns{};
    std::uint64_t end_to_end_p99_ns{};
    std::uint64_t end_to_end_p999_ns{};
};

[[nodiscard]]
PipelineLatencyStats calculate_pipeline_latency_stats(
    std::vector<std::uint64_t> producer_wait_latencies,
    std::vector<std::uint64_t> queue_residence_latencies,
    std::vector<std::uint64_t> engine_latencies,
    std::vector<std::uint64_t> end_to_end_latencies
);