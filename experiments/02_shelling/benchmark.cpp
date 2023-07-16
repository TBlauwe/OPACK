#include <benchmark/benchmark.h>
#include "src/model.hpp"

void BM_shelling(benchmark::State& state)
{
	constexpr size_t height = 100;
	constexpr size_t width = 100;
	constexpr float density = .95f;
	constexpr float similar_wanted = .30;

	auto world = opack::create_world();
	auto shelling = Shelling<height, width>(world, density, similar_wanted);
    for ([[maybe_unused]] auto _ : state)
    {
        opack::run(world);
    }
}
BENCHMARK(BM_shelling)
	->Unit(benchmark::kMillisecond)
;

BENCHMARK_MAIN();
