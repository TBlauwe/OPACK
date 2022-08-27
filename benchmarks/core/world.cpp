#include "../utils.hpp"
	
void BM_create_empty_world(benchmark::State& state) {
    for ([[maybe_unused]] auto _ : state) {
        auto world = opack::create_world();
        benchmark::DoNotOptimize(world);
    }
}
BENCHMARK(BM_create_empty_world)
    ->Unit(benchmark::kMillisecond)
;

void BM_spawn_n_agent(benchmark::State& state) {
    auto world = opack::create_world();
    for ([[maybe_unused]] auto _ : state) {
        spawn_n<opack::Agent>(world, static_cast<size_t>(state.range(0)));
    }
}
BENCHMARK(BM_spawn_n_agent)
    ->Unit(benchmark::kNanosecond)
    ->Arg(1 << 0)
;

BENCHMARK(BM_spawn_n_agent)
    ->Unit(benchmark::kMillisecond)
    ->Arg(1<<10)->Arg(1<<20)
;

BENCHMARK_MAIN();
