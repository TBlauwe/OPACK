#include "../utils.hpp"
	
void BM_create_empty_world(benchmark::State& state) {
    for ([[maybe_unused]] auto _ : state) {
        auto world = opack::World ();
        benchmark::DoNotOptimize(world);
    }
}
BENCHMARK(BM_create_empty_world)
    ->Unit(benchmark::kMillisecond)
;

void BM_spawn_n_entity(benchmark::State& state) {
    auto world = opack::World ();
    for ([[maybe_unused]] auto _ : state) {
        spawn_n_agents<opack::Agent>(world, static_cast<size_t>(state.range(0)));
    }
}
BENCHMARK(BM_spawn_n_entity)
    ->Unit(benchmark::kMillisecond)
    ->Arg(1<<10)->Arg(1<<20)
;

BENCHMARK_MAIN();
