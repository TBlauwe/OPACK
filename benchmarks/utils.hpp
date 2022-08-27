#pragma once

#include <benchmark/benchmark.h>
#include <opack/core.hpp>

template<typename T>
void spawn_n(opack::World& world, size_t n)
{
    for (auto i = 0; i < n; i++)
    {
        opack::spawn<T>(world);
    }
}
