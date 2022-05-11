#include <concepts>
#include <benchmark/benchmark.h>
#include <opack/core.hpp>
#include <opack/utils/simulation_template.hpp>
#include <opack/examples/empty_sim.hpp>
#include <opack/examples/simple_sim.hpp>

	
template<std::derived_from<opack::SimulationTemplate> Simulation = opack::SimulationTemplate> 
void BM_create_simulation_empty(benchmark::State& state) {
    for ([[maybe_unused]] auto _ : state) {
        auto sim = Simulation();
    }
}
BENCHMARK(BM_create_simulation_empty<EmptySim>)
    ->Unit(benchmark::kMillisecond)
;
BENCHMARK(BM_create_simulation_empty<SimpleSim>)
    ->Unit(benchmark::kMillisecond)
;

static void BM_create_agent(benchmark::State& state) {
    auto sim = opack::Simulation();
    for ([[maybe_unused]] auto _ : state) {
        opack::agent(sim);
    }
}
BENCHMARK(BM_create_agent)
        ->Unit(benchmark::kNanosecond)
;

static void BM_create_n_agent(benchmark::State& state) {
    auto sim = opack::Simulation();
    for ([[maybe_unused]] auto _ : state) {
        for(int i = 0; i<state.range(0); i++){
            opack::agent(sim);
        }
    }
}
BENCHMARK(BM_create_n_agent)
        ->Unit(benchmark::kMillisecond)
        ->Range(1<<10, 1<<20)
;

template<std::derived_from<opack::SimulationTemplate> Simulation = opack::SimulationTemplate>
void BM_run_simulation(benchmark::State& state) {
    auto sim = opack::Simulation();
    for ([[maybe_unused]] auto _ : state) {
        sim.step_n(state.range(0), 0.5f);
    }
}
BENCHMARK(BM_run_simulation<EmptySim>)
    ->Unit(benchmark::kMillisecond)
    ->Arg(1<<14)
    ;
BENCHMARK(BM_run_simulation<SimpleSim>)
    ->Unit(benchmark::kMillisecond)
    ->Arg(1<<14)
    ;

// Run the benchmark
BENCHMARK_MAIN();