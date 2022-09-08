#include "../utils.hpp"

#include <opack/module/simple_agent.hpp>

static void BM_create_n_simple_agent(benchmark::State& state) {
    auto world = opack::create_world();
    world.import<simple>();
    for ([[maybe_unused]] auto _ : state)
    {
        spawn_n<simple::Agent>(world, state.range(0));
    }
}
BENCHMARK(BM_create_n_simple_agent)
    ->Unit(benchmark::kNanosecond)
    ->Arg(1<<0);

BENCHMARK(BM_create_n_simple_agent)
    ->Unit(benchmark::kMillisecond)
    ->Arg(1 << 10)->Arg(1 << 15);
