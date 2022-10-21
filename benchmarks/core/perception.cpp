#include "../utils.hpp"

OPACK_SUB_PREFAB(MySense, opack::Sense);

static void BM_create_n_percepts_with_n_agents(benchmark::State& state) {
    auto world = opack::create_world();
    auto artefact = opack::spawn<opack::Artefact>(world);
    opack::init<MySense>(world);
    opack::add_sense<MySense, opack::Agent>(world);
    opack::spawn_n<opack::Agent>(world, state.range(0));
    auto filter = world.filter_builder<>()
        .term(flecs::IsA).second<opack::Agent>()
        .term(flecs::Prefab).not_()
        .build();

    for ([[maybe_unused]] auto _ : state)
    {
        filter.each(
            [artefact](flecs::entity e)
            {
                opack::perceive<MySense>(e, artefact);
            }
        );
    }
}
BENCHMARK(BM_create_n_percepts_with_n_agents)
        ->Unit(benchmark::kNanosecond)
        ->Arg(1<<0);

BENCHMARK(BM_create_n_percepts_with_n_agents)
        ->Unit(benchmark::kMillisecond)
        ->Arg(1<<10)->Arg(1<<15)
;
