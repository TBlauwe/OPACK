#pragma once
#include <numeric>
#include <algorithm>
#include <fmt/compile.h>
#include <effolkronium/random.hpp>
using Random = effolkronium::random_static;

#include <opack/core.hpp>

#include "environment.hpp"
#include "identifiers.hpp"

template<size_t H, size_t W>
class Shelling
{
public:
	using Grid_t = Grid<flecs::entity, H, W>;
	Shelling(flecs::world& world, float density, float similar_wanted)
		:
		// ----- Flecs -----
		world{ world },
		empty_patches{ world.query_builder().term<Position>().term<Agent>().not_().build()},
		happy_agents{ world.query_builder().term<Agent>().term<Happy>().build() },
		agents_query{ world.query_builder().term<Agent>().build() },
		// ----- Model -----
		grid{},
		density{ density },
		similar_wanted{ similar_wanted }
	{
		init_components(); // For web app inspection
		populate();
		define_logic();
		opack::step(world);
		add_move_system();
	}

public:
	// ----- Flecs
	flecs::world& world;
	flecs::query<> empty_patches;
	flecs::query<> happy_agents;
	flecs::query<> agents_query;

	// ----- Model
	Grid_t grid;
	float density{ 0.95f };
	float similar_wanted{ 0.95f };

	void define_logic()
	{
		// =========================================================================== 
		// Logic
		// =========================================================================== 
		world.system<const Position, LocalStats>("Update_LocalStats")
			.kind(flecs::OnUpdate)
			.each([this](flecs::entity agent, const Position& pos, LocalStats& stats)
				{
					auto neighbours = this->grid.neighbours(pos);
					auto team = agent.get<Team>();
					stats.total_nearby = neighbours.size();
					stats.similar_nearby = std::count_if(neighbours.begin(), neighbours.end(),
						[&team](const flecs::entity e) {return team == e.get<Team>(); });
					stats.other_nearby = stats.total_nearby - stats.similar_nearby;
				});

		world.system<const LocalStats>("System_ComputeHappiness")
			.kind(flecs::OnUpdate)
			.term<Happy>().write()
			.each([this](flecs::entity e, const LocalStats& stats)
				{
					if (stats.similar_nearby >= this->similar_wanted * stats.total_nearby)
						e.add<Happy>();
					else
						e.remove<Happy>();
				});


		// =========================================================================== 
		// Globals update
		// =========================================================================== 
		world.system<const LocalStats, GlobalStats>("Update_Globals_Stats")
			.arg(2).singleton()
			.kind(flecs::OnValidate)
			.iter([this](flecs::iter& it, const LocalStats* local_stats, GlobalStats* global_stats)
				{
					int similar_neighbours{ 0 };
					int total_neighbours{ 0 };
					for (auto i : it)
					{
						similar_neighbours += local_stats[i].similar_nearby;
						total_neighbours += local_stats[i].total_nearby;
					}
					global_stats->percent_similar = (static_cast<float>(similar_neighbours) / total_neighbours) * 100;
					global_stats->percent_unhappy = (static_cast<float>(this->happy_agents.count()) / this->agents_query.count()) * 100;
				}
		);

		world.system("StopCondition")
			.kind(flecs::PreFrame)
			.iter([this](flecs::iter& it)
				{
					if (agents_query.count() == this->happy_agents.count())
						it.world().quit();
				}
		);
	}

	void add_move_system()
	{
		world.system<Agent, Position>("System_Move")
			.kind(flecs::PostUpdate)
			.term<Happy>().not_()
			.no_staging()
			.each([this](flecs::entity agent, Agent, Position& pos)
				{
					auto empty_cell = this->empty_patches.first();
					const Position& empty_cell_pos = *empty_cell.template get<Position>();
					this->grid.swap(pos, empty_cell_pos);
					Position tmp = pos;
					pos = empty_cell_pos;
					empty_cell.set<Position>(tmp);
					//fmt::print("Moving {} from ({}, {}) to ({}, {})\n", agent.id(), empty_cell_pos.w, empty_cell_pos.h, pos.w, pos.h);
				});
	}

	flecs::entity spawn_entity(const size_t w, const size_t h)
	{
		auto entity = world.entity().set<Position>({ w, h });
		if (Random::get<bool>(density))
		{
			entity.add<Agent>().add<LocalStats>();
			if (Random::get<bool>())
				entity.add(Team::Red);
			else
				entity.add(Team::Blue);
		}
		return entity;
	}

	void populate()
	{
		auto agents = world.entity("Agents");
		auto scope = world.set_scope(agents);
		for (size_t w = 0; w < W; w++) {
			for (size_t h = 0; h < H; h++) {
				grid.set(w, h, spawn_entity(w, h));
			}
		}
		world.set_scope(scope);
	}

	void init_components()
	{
		world.add<GlobalStats>();
		world.component<GlobalStats>()
			.member<float>("percent_similar")
			.member<float>("percent_unhappy")
			;

		world.component<LocalStats>()
			.member<int>("similar_nearby")
			.member<int>("other_nearby")
			.member<int>("total_nearby")
			;

		world.component<Team>()
			.constant("Red", static_cast<int32_t>(Team::Red))
			.constant("Blue", static_cast<int32_t>(Team::Blue))
			;

		world.component<Position>()
			.member<size_t>("height")
			.member<size_t>("width")
			;
	}
};

