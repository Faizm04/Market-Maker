#include "thread_config.hpp"

#if defined(__APPLE__)

#include <pthread.h>

#elif defined(__linux__)

#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>

#include <stdexcept>

#endif

void configure_current_thread(ThreadRole role)
{
#if defined(__APPLE__)

    qos_class_t qos = QOS_CLASS_USER_INTERACTIVE;

    int relative_priority = 0;

    pthread_set_qos_class_self_np(
        qos,
        relative_priority
    );

#elif defined(__linux__)

    int cpu = 0;

    switch (role)
    {
        case ThreadRole::MarketData:
            cpu = 2;
            break;

        case ThreadRole::Trading:
            cpu = 3;
            break;
    }

    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    const int result =
        pthread_setaffinity_np(
            pthread_self(),
            sizeof(cpu_set_t),
            &cpuset
        );

    if (result != 0)
    {
        throw std::runtime_error(
            "Failed to set thread CPU affinity"
        );
    }

#else

    (void) role;

#endif
}