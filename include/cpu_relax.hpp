#pragma once

#if defined(__x86_64__) || defined(_M_X64)

#include <immintrin.h>

inline void cpu_relax()
{
    _mm_pause();
}

#elif defined(__aarch64__) || defined(_M_ARM64)

inline void cpu_relax()
{
    asm volatile(
        "yield"
        ::: "memory"
    );
}

#else

inline void cpu_relax()
{
    asm volatile(
        ""
        ::: "memory"
    );
}

#endif