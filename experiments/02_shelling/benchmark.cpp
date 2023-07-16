#include <benchmark/benchmark.h>
#include "src/model.hpp"

template<size_t Size> 
void BM_shelling(benchmark::State& state)
{
	constexpr float density = .95f;
	constexpr float similar_wanted = .30;

	auto world = opack::create_world();
	auto shelling = Shelling<Size, Size>(world, state.range(0), density, similar_wanted);
    for ([[maybe_unused]] auto _ : state)
    {
        opack::step(world);
    }
}
BENCHMARK(BM_shelling<201>)
	->MinTime(20)
	->Range(0, 1)
	->Unit(benchmark::kMillisecond)
;

BENCHMARK(BM_shelling<601>)
	->MinTime(20)
	->Range(0, 1)
	->Unit(benchmark::kSecond)
;

BENCHMARK(BM_shelling<1001>)
	->MinTime(20)
	->Range(0, 1)
	->Unit(benchmark::kSecond)
;

BENCHMARK_MAIN();
