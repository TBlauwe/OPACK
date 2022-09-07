#include "../utils.hpp"

OPACK_ACTUATOR(MyActuator);
OPACK_ACTION(MyAction);

static void BM_create_n_actions_with_n_agents(benchmark::State& state) {
    auto world = opack::create_world();
    opack::init<MyActuator>(world);
    opack::init<MyAction>(world);
    opack::add_actuator<MyActuator, opack::Agent>(world);
    spawn_n<opack::Agent>(world, state.range(0));
    auto filter = world.filter_builder<>()
        .term(flecs::IsA).second<opack::Agent>()
        .term(flecs::Prefab).not_()
        .build();

    for ([[maybe_unused]] auto _ : state)
    {
        filter.each(
            [](flecs::entity e)
            {
                auto world = e.world();
                auto action = opack::spawn<MyAction>(world);
                opack::act(e, action);
            }
        );
    }
}
BENCHMARK(BM_create_n_actions_with_n_agents)
        ->Unit(benchmark::kNanosecond)
        ->Arg(1<<0);

BENCHMARK(BM_create_n_actions_with_n_agents)
        ->Unit(benchmark::kMillisecond)
        ->Arg(1<<10)->Arg(1<<15)
;
