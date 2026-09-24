#pragma once

enum class ThreadRole
{
    MarketData,
    Trading
};

void configure_current_thread(ThreadRole role);