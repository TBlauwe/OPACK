#pragma once

#include <benchmark/benchmark.h>
#include <opack/core.hpp>

template<typename T>
inline void spawn_n_agents(opack::World& world, size_t n)
{
    for (auto i = 0; i < n; i++)
    {
        opack::spawn<T>(world);
    }
}
