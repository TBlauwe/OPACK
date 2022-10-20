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

void BM_loop_n(benchmark::State& state)
{
	auto world = opack::create_world();
	benchmark::DoNotOptimize(world);
    for ([[maybe_unused]] auto _ : state)
    {
        opack::step_n(world, static_cast<size_t>(state.range(0)));
    }
}
BENCHMARK(BM_loop_n)
    ->Unit(benchmark::kNanosecond)
	->Range(1<<0, 1 << 10);
BENCHMARK(BM_loop_n)
    ->Unit(benchmark::kMillisecond)
	->Range(1<<15, 1 << 20);
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

void BM_loop_n_with_m_agents(benchmark::State& state)
{
	auto world = opack::create_world();
	benchmark::DoNotOptimize(world);
	spawn_n<opack::Agent>(world, static_cast<size_t>(state.range(1)));
    for ([[maybe_unused]] auto _ : state)
    {
        opack::step_n(world, static_cast<size_t>(state.range(0)));
    }
}
BENCHMARK(BM_loop_n_with_m_agents)
    ->Unit(benchmark::kNanosecond)
	->Ranges({ { 1 << 0, 1 << 20}, {1 << 0, 1 << 20} });
;

BENCHMARK_MAIN();
