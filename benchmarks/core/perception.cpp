#include "../utils.hpp"

OPACK_SUB_PREFAB(MySense, opack::Sense);
struct MySenseValue {int i{0};};

static void BM_create_n_percepts_with_m_agents(benchmark::State& state) {
    auto world = opack::create_world();
    opack::init<MySense>(world);
    opack::add_sense<MySense, opack::Agent>(world);

    opack::spawn_n<opack::Artefact>(world, state.range(0));
    opack::spawn_n<opack::Agent>(world, state.range(1));

    auto agent_filter = world.query_builder<>()
        .term(flecs::IsA).second<opack::Agent>()
        .term(flecs::Prefab).not_()
        .build();

    auto artefact_filter = world.query_builder<>()
        .term(flecs::IsA).second<opack::Artefact>()
        .term(flecs::Prefab).not_()
        .build();

    for ([[maybe_unused]] auto _ : state)
    {
        agent_filter.each(
            [&artefact_filter](flecs::entity agent)
            {
				artefact_filter.each(
					[&agent](flecs::entity artefact)
					{
						opack::perceive<MySense>(agent, artefact);
					}
				);
            }
        );
    }
}
BENCHMARK(BM_create_n_percepts_with_m_agents)
        ->Unit(benchmark::kNanosecond)
		->Args({1<<0, 1<< 0});

BENCHMARK(BM_create_n_percepts_with_m_agents)
        ->Unit(benchmark::kMillisecond)
		->Ranges({ { 1 << 2, 1 << 5}, {1 << 2, 1 << 5} });
;
static void BM_iterate_n_percepts_with_m_agents(benchmark::State& state) {
    auto world = opack::create_world();
    opack::init<MySense>(world);
    opack::add_sense<MySense, opack::Agent>(world);

    opack::spawn_n<opack::Artefact>(world, state.range(0));
    opack::spawn_n<opack::Agent>(world, state.range(1));

    auto agent_filter = world.query_builder<>()
        .term(flecs::IsA).second<opack::Agent>()
        .term(flecs::Prefab).not_()
        .build();

    auto artefact_filter = world.query_builder<>()
        .term(flecs::IsA).second<opack::Artefact>()
        .term(flecs::Prefab).not_()
        .build();

	agent_filter.each(
		[&artefact_filter](flecs::entity agent)
		{
			artefact_filter.each(
				[&agent](flecs::entity artefact)
				{
					opack::perceive<MySense>(agent, artefact);
				}
			);
		}
	);

    for ([[maybe_unused]] auto _ : state)
    {
		agent_filter.each(
			[](flecs::entity agent)
			{
                opack::perception(agent).each<opack::Artefact>([](opack::Entity artefact){});
			}
		);
    }
}

BENCHMARK(BM_iterate_n_percepts_with_m_agents)
        ->Unit(benchmark::kNanosecond)
		->Args({1<<0, 1<< 0});

BENCHMARK(BM_iterate_n_percepts_with_m_agents)
        ->Unit(benchmark::kMillisecond)
		->Ranges({ { 1 << 2, 1 << 5}, {1 << 2, 1 << 5} });

static void BM_does_perceive(benchmark::State& state) {
    auto world = opack::create_world();
    opack::init<MySense>(world);
    opack::add_sense<MySense, opack::Agent>(world);

    auto artefact = opack::spawn<opack::Artefact>(world);
    auto agent = opack::spawn<opack::Agent>(world);
    auto p_a = opack::perception(agent);

    for ([[maybe_unused]] auto _ : state)
    {
		p_a.perceive<MySense>(artefact);
    }
}

BENCHMARK(BM_does_perceive)
        ->Unit(benchmark::kNanosecond);

static void BM_does_perceive_value(benchmark::State& state) {
    auto world = opack::create_world();
    opack::init<MySense>(world);
    opack::add_sense<MySense, opack::Agent>(world);
    opack::perceive<MySense, MySenseValue>(world);

    auto artefact = opack::spawn<opack::Artefact>(world);
    artefact.add<MySenseValue>();

    auto agent = opack::spawn<opack::Agent>(world);
    auto p_a = opack::perception(agent);

    for ([[maybe_unused]] auto _ : state)
    {
		auto value = p_a.value<MySense, MySenseValue>(artefact);
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK(BM_does_perceive_value)
        ->Unit(benchmark::kNanosecond);
