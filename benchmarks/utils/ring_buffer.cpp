#include <benchmark/benchmark.h>
#include <opack/utils/ring_buffer.hpp>

static void BM_ring_buffer_creation(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        benchmark::DoNotOptimize(ring_buffer<int>(state.range(0)));
    }
}
BENCHMARK(BM_ring_buffer_creation)
        ->Unit(benchmark::kNanosecond)
        ->Range(1<<0, 1 << 20);

static void BM_ring_buffer_write(benchmark::State& state) {

	auto rg = ring_buffer<int>(state.range(0));
    for ([[maybe_unused]] auto _ : state)
    {
        for(auto i = 0; i < state.range(1); i++)
        {
            rg.push(i);
        }
    }
}
BENCHMARK(BM_ring_buffer_write)
        ->Unit(benchmark::kNanosecond)
		->Ranges({ { 1 << 0, 1 << 20}, {1 << 0, 1 << 10} });

static void BM_ring_buffer_read(benchmark::State& state) {

	auto rg = ring_buffer<int>(state.range(0));
	for(auto i = 0; i < state.range(0); i++)
	{
		rg.push(i);
	}
    for ([[maybe_unused]] auto _ : state)
    {
        for(auto i = 0; i < state.range(1); i++)
        {
            benchmark::DoNotOptimize(rg.peek(i));
        }
    }
}
BENCHMARK(BM_ring_buffer_read)
        ->Unit(benchmark::kNanosecond)
    ->Ranges({ { 1 << 0, 1 << 20}, {1 << 0, 1 << 10} });

