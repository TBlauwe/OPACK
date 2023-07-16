#pragma once
#include <numeric>
#include <algorithm>
#include <fmt/compile.h>
#include <effolkronium/random.hpp>
using Random = effolkronium::random_static;

#include <opack/core.hpp>

#include "environment.hpp"
#include "identifiers.hpp"
#include "display.hpp"

template<size_t H, size_t W>
class Shelling
{
public:
	using Grid_t = Grid<flecs::entity, H, W>;
	Shelling(flecs::world& world, float density, float similar_wanted)
		:
		// ----- Flecs -----
		world{ world },
		empty_patches{ world.query_builder<Position>().term<Agent>().not_().build() },
		happy_agents{ world.query_builder<const Happy>().build() },
		agents_query{ world.query_builder<const LocalStats>().build() },
		// ----- Model -----
		grid{},
		grid_display{ world, grid },
		density{ density }
	{
		// Singleton
		world.add<GlobalStats>();

		// For web app inspection
		init_components();

		// Define a entity "model", a prefab, that will be used
		// to instantiate entities based on it.
		opack::init<Agent>(world)
			.set<SimilarWanted>({ similar_wanted })
			.add<Agent>()
			.override<LocalStats>();
		;

		populate();
		define_logic();
		opack::step(world);
		add_move_system();
	}

public:
	// ----- Flecs
	flecs::world& world;
	flecs::query<Position> empty_patches;
	flecs::query<const Happy> happy_agents;
	flecs::query<const LocalStats> agents_query;

	// ----- Model
	Grid_t grid;
	float density{ 0.95f };

	// ----- Model
	GridDisplay<H, W> grid_display;

	void interactive_run(bool with_display, bool clear_every_tick)
	{
		std::string input;
		do {
			std::getline(std::cin, input);
			world.system()
				.kind(flecs::PostFrame)
				.iter([&](flecs::iter& it)
					{
						if (clear_every_tick) grid_display.clear();
						if (with_display)
						{
							grid_display.display_stats();
							grid_display.display_grid();
						}

					});
			opack::step(world);

		} while (true);
	}

	void run(bool with_web_app, bool with_display, bool clear_every_tick)
	{
		world.system()
			.kind(flecs::PostFrame)
			.iter([&](flecs::iter& it)
				{
					if (clear_every_tick) grid_display.clear();
					if (with_display)
					{
						grid_display.display_stats();
						grid_display.display_grid();
					}

				});
			if (with_web_app)
				opack::run_with_webapp(world);
			else
				opack::run(world);
	}

	void define_logic()
	{
		// =========================================================================== 
		// Logic
		// =========================================================================== 
		world.system<const Position, LocalStats>("Update_LocalStats")
			.each([this](flecs::entity agent, const Position& pos, LocalStats& stats)
				{
					auto neighbours = this->grid.neighbours(pos);
					auto team = agent.get<Team>();
					stats.total_nearby = neighbours.size();
					stats.similar_nearby = std::count_if(neighbours.begin(), neighbours.end(),
						[&team](const flecs::entity e) {return team == e.get<Team>(); });
					stats.other_nearby = stats.total_nearby - stats.similar_nearby;
				});

		world.system<const SimilarWanted, const LocalStats>("System_ComputeHappiness")
			.term<Happy>().write()
			.each([this](flecs::entity e, const SimilarWanted& similar_wanted, const LocalStats& stats)
				{
					if (stats.similar_nearby >= similar_wanted.value * stats.total_nearby)
						e.add<Happy>();
					else
						e.remove<Happy>();
				});


		// =========================================================================== 
		// Globals update
		// =========================================================================== 
		world.system<GlobalStats>("Update_GlobalsStats")
			.term<Happy>().read()
			.each([this](flecs::iter& it, size_t i, GlobalStats& global_stats)
				{
					int similar_neighbours{ 0 };
					int total_neighbours{ 0 };
					this->agents_query.each([&similar_neighbours, &total_neighbours](flecs::entity e, const LocalStats& local_stats)
						{
							similar_neighbours += local_stats.similar_nearby;
							total_neighbours += local_stats.total_nearby;
						}
					);
					global_stats.percent_similar = (static_cast<float>(similar_neighbours) / total_neighbours) * 100;
					global_stats.percent_unhappy = (static_cast<float>(this->happy_agents.count()) / this->agents_query.count()) * 100;
				}
		);

		world.system("StopCondition")
			.term<Happy>().read()
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
		world.system<Position>("System_Move")
			.term<Position>().read_write()
			.term(flecs::IsA, opack::entity<Agent>(world))
			.kind(flecs::PreUpdate)
			.term<Happy>().not_()
			.no_staging()
			.iter([this](flecs::iter& it, Position* pos)
				{
					for (auto i : it)
					{
						auto empty_cell = this->empty_patches.first();
						it.world().defer_suspend();
						Position& empty_cell_pos = *empty_cell.template get_mut<Position>();
						this->grid.swap(pos[i], empty_cell_pos);
						std::swap(pos[i], empty_cell_pos);
						it.world().defer_resume();
						//fmt::print("Moving {} from ({}, {}) to ({}, {})\n", it.entity(i).id(), empty_cell_pos.w, empty_cell_pos.h, pos[i].w, pos[i].h);
					}
				});
	}

	flecs::entity spawn_entity(const size_t w, const size_t h)
	{
		auto entity = world.entity().set<Position>({ w, h });
		if (Random::get<bool>(density))
		{
			entity.is_a<Agent>();
			if (Random::get<bool>())
				entity.add(Team::Red);
			else
				entity.add(Team::Blue);
		}
		return entity;
	}

	void populate()
	{
		for (size_t h = 0; h < H; h++) {
			for (size_t w = 0; w < W; w++) {
				grid.set(w, h, spawn_entity(w, h));
			}
		}
	}

	void init_components()
	{
		world.component<GlobalStats>()
			.member<float>("percent_similar")
			.member<float>("percent_unhappy")
			;

		world.component<SimilarWanted>()
			.member<float>("value")
			;

		world.component<LocalStats>()
			.member<uint8_t>("similar_nearby")
			.member<uint8_t>("other_nearby")
			.member<uint8_t>("total_nearby")
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

