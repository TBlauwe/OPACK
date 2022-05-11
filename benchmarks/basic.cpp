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

static void create_n_agent(opack::Simulation& sim, size_t n)
{
	for(int i = 0; i<n; i++){
		opack::agent(sim);
	}
}

static void BM_create_n_agent(benchmark::State& state) {
    auto sim = opack::Simulation();
    for ([[maybe_unused]] auto _ : state) {
        create_n_agent(sim, state.range(0));
    }
}
BENCHMARK(BM_create_n_agent)
        ->Unit(benchmark::kMillisecond)
        ->Arg(1<<10)->Arg(1<<20)
;

struct MyActuator : opack::Actuator {};
struct MyAction : opack::Action {};
static void BM_create_action(benchmark::State& state) {
    auto sim = opack::Simulation();
    auto agent = opack::agent(sim);
    auto artefact = opack::artefact(sim);
    opack::register_actuator<MyActuator>(sim);
    opack::register_action<MyAction>(sim);
    for ([[maybe_unused]] auto _ : state) {
        auto action = opack::action<MyAction>(sim);
        action.add<opack::On>(artefact);
        opack::act<MyActuator>(agent, action);
    }
}
BENCHMARK(BM_create_action)
        ->Unit(benchmark::kNanosecond)
;

static void BM_create_n_actions_with_n_agents(benchmark::State& state) {
    auto sim = opack::Simulation();
    auto artefact = opack::artefact(sim);
    opack::register_actuator<MyActuator>(sim);
    opack::register_action<MyAction>(sim);
    create_n_agent(sim, state.range(0));
    sim.world.system<opack::Agent>()
        .multi_threaded(true)
        .iter(
            [&artefact](flecs::iter& it)
            {
                for (auto i : it)
                {
                    auto world = it.world();
                    auto action = opack::action<MyAction>(world);
                    action.add<opack::On>(artefact);
                    opack::act<MyActuator>(it.entity(i), action);
                }
            }
    );
    for ([[maybe_unused]] auto _ : state) {
        sim.step();
    }
}
BENCHMARK(BM_create_n_actions_with_n_agents)
        ->Unit(benchmark::kMillisecond)
        ->Arg(1<<10)//->Arg(1<<20)
;

struct MySense : opack::Sense {};
static void BM_perceive(benchmark::State& state) {
    auto sim = opack::Simulation();
    auto agent = opack::agent(sim);
    auto artefact = opack::artefact(sim);
    opack::register_sense<MySense>(sim);
    for ([[maybe_unused]] auto _ : state) {
        opack::perceive<MySense>(agent, artefact);
    }
}
BENCHMARK(BM_perceive)
        ->Unit(benchmark::kNanosecond)
;

static void BM_create_n_percepts_with_n_agents(benchmark::State& state) {
    auto sim = opack::Simulation();
    auto artefact = opack::artefact(sim);
    opack::register_sense<MySense>(sim);
    create_n_agent(sim, state.range(0));
    sim.world.system<opack::Agent>()
        .multi_threaded(true)
        .iter(
            [&artefact](flecs::iter& it)
            {
                for (auto i : it)
                {
                    opack::perceive<MySense>(it.entity(i), artefact);
                }
            }
    );
    for ([[maybe_unused]] auto _ : state) {
        sim.step();
    }
}
BENCHMARK(BM_create_n_percepts_with_n_agents)
        ->Unit(benchmark::kMillisecond)
        ->Arg(1<<10)->Arg(1<<20)
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