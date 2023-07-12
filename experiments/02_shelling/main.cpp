#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include <fmt/compile.h>

#include <effolkronium/random.hpp>
using Random = effolkronium::random_static;

#include <opack/core.hpp>

struct Position
{
	size_t w;
	size_t h;
};

template<typename T, size_t H, size_t W>
struct Grid
{

	static constexpr size_t height() { return H; }
	static constexpr size_t weight() { return W; }
	static constexpr size_t size() { return H * W; }

	static constexpr size_t linearize(const size_t w, const size_t h) { return h * W + w; }
	static constexpr size_t linearize(const Position pos) { return pos.h * W + pos.w; }

	static constexpr Position n_neighbour(const Position pos) { return { pos.w % W , (pos.h - 1) % H }; }
	static constexpr Position s_neighbour(const Position pos) { return { pos.w % W , (pos.h + 1) % H }; }
	static constexpr Position o_neighbour(const Position pos) { return { (pos.w - 1) % W ,  pos.h }; }
	static constexpr Position e_neighbour(const Position pos) { return { (pos.w + 1) % W ,  pos.h }; }

	static constexpr Position ne_neighbour(const Position pos) { return { (pos.w + 1) % W , (pos.h - 1) % H }; }
	static constexpr Position no_neighbour(const Position pos) { return { (pos.w - 1) % W , (pos.h - 1) % H }; }
	static constexpr Position se_neighbour(const Position pos) { return { (pos.w + 1) % W , (pos.h + 1) % H }; }
	static constexpr Position so_neighbour(const Position pos) { return { (pos.w - 1) % W , (pos.h + 1) % H }; }

	Grid(std::function<T(const size_t w, const size_t h)> builder)
	{
		for (size_t w = 0; w < W; w++) {
			for (size_t h = 0; h < H; h++) {
				cells[linearize(w, h)] = builder(w, h);
			}
		}
	}

	constexpr void set(const size_t w, const size_t h, T value) {
		cells[linearize(w, h)] = value;
	}

	constexpr const T operator()(const size_t w, const size_t h) {
		return cells[linearize(w, h)];
	}

	constexpr std::array<T, 8> neighbours(const Position pos)
	{
		return std::array<T, 8>{
			cells[linearize(n_neighbour(pos))],
				cells[linearize(s_neighbour(pos))],
				cells[linearize(e_neighbour(pos))],
				cells[linearize(o_neighbour(pos))],
				cells[linearize(ne_neighbour(pos))],
				cells[linearize(no_neighbour(pos))],
				cells[linearize(se_neighbour(pos))],
				cells[linearize(so_neighbour(pos))]
		};
	}

	std::array<T, H* W> cells;
};

struct Happy {};
struct GlobalStats
{
	float percent_similar{ 0.0f };
	float percent_unhappy{ 0.0f };
};

struct LocalStats
{
	int similar_nearby{ 0 };
	int other_nearby{ 0 };
	int total_nearby{ 0 };
};

enum class Team
{
	Red,
	Blue
};

bool same_team(const flecs::entity a, const flecs::entity b)
{
	return a.get<Team>() == b.get<Team>();
}

struct Neighbours
{
	std::vector<flecs::entity> container;
};


struct Cell {};
struct OccupiedBy {};
struct Occupy {};
void move(flecs::entity agent, flecs::entity patch)
{
	if(auto last_patch = agent.target<Occupy>())
		last_patch.remove<OccupiedBy>(agent);
	agent.add<Occupy>(patch);
	patch.add<OccupiedBy>(agent);
}

struct Agent {};

int main()
{
	// =========================================================================== 
	// Parameters
	// =========================================================================== 
	constexpr size_t height = 8;
	constexpr size_t width = 8;
	constexpr float density = .95f;
	constexpr float similar_wanted = .30;


	// =========================================================================== 
	// World definition
	// =========================================================================== 
	auto world = opack::create_world();

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

	world.component<OccupiedBy>()
		.add(flecs::Union)
		.add(flecs::Exclusive)
		;

	world.component<Occupy>()
		.add(flecs::Union)
		.add(flecs::Exclusive)
		;

	auto empty_patches = world.query_builder()
		.term<Position>()
		.term<OccupiedBy>().second(flecs::Wildcard).not_()
		.build();

	auto happy_agents = world.query_builder()
		.term<Agent>()
		.term<Happy>()
		.build();

	auto agents_query = world.query_builder()
		.term<Agent>()
		.build();


	// =========================================================================== 
	// World setup
	// =========================================================================== 
	auto patches = world.entity("Patches");
	auto agents = world.entity("Agents");
	using Grid_t = Grid<flecs::entity, height, width>;
	Grid_t grid([&world, &patches, &agents](size_t w, size_t h) {
		auto patch = world.entity(fmt::format("Patch_{}_{}", w, h).c_str())
			.add<Cell>()
			.add<Neighbours>()
			;
		patch.set<Position>({ w, h });
		patch.child_of(patches);
		if (Random::get<bool>(density))
		{
			auto agent = world.entity()
				.add<Agent>()
				.add<LocalStats>()
				.add<Neighbours>()
				;
			agent.child_of(agents);
			if (Random::get<bool>())
				agent.add(Team::Red);
			else
				agent.add(Team::Blue);
			move(agent, patch);
		}
		return patch;
		});

	world.each([&grid](flecs::entity patch, const Position& position, Neighbours& neighbours)
		{
			for (auto neighbour : grid.neighbours(position))
			{
				neighbours.container.push_back(neighbour);
			}
		}
	);

	// =========================================================================== 
	// Logic
	// =========================================================================== 
	world.system<Agent, Neighbours>("Update_Neighbours")
		.kind(flecs::PreUpdate)
		.each([](flecs::entity agent, Agent, Neighbours& neighbours)
			{
				neighbours.container.clear();
				int32_t index = 0;
				auto patch = agent.target<Occupy>();
				for (auto neighbour_patch : patch.get<Neighbours>()->container)
				{
					auto neighbour = neighbour_patch.target<OccupiedBy>();
					if (neighbour)
						neighbours.container.push_back(neighbour);
				}
			});

	world.system<const Neighbours, LocalStats>("Update_LocalStats")
		.kind(flecs::OnUpdate)
		.each([](flecs::entity agent, const Neighbours& neighbours, LocalStats& stats)
			{
				auto team = agent.get<Team>();
				stats.total_nearby = neighbours.container.size();
				stats.similar_nearby = std::count_if(neighbours.container.begin(), neighbours.container.end(),
					[&team](const flecs::entity e) {return team == e.get<Team>(); });
				stats.other_nearby = stats.total_nearby - stats.similar_nearby;
			});

	world.system<const LocalStats>("System_ComputeHappiness")
		.kind(flecs::OnUpdate)
		.term<Happy>().write()
		.each([&similar_wanted](flecs::entity e, const LocalStats& stats)
			{
				if (stats.similar_nearby >= similar_wanted * stats.total_nearby)
					e.add<Happy>();
				else
					e.remove<Happy>();
			});

	world.system<Agent>("System_Move")
		.kind(flecs::PostUpdate)
		.term<Happy>().not_()
		.no_staging()
		.each([&empty_patches](flecs::entity agent, Agent)
			{
				auto patch = empty_patches.first();
				move(agent, patch);
			});


	// =========================================================================== 
	// Globals update
	// =========================================================================== 
	world.system<const LocalStats, GlobalStats>("Update_Globals_Stats")
		.arg(2).singleton()
		.kind(flecs::OnValidate)
		.iter([](flecs::iter& it, const LocalStats* local_stats, GlobalStats* global_stats)
			{
				int similar_neighbours{ 0 };
				int total_neighbours{ 0 };
				for (auto i : it)
				{
					similar_neighbours += local_stats[i].similar_nearby;
					total_neighbours += local_stats[i].total_nearby;
				}
				global_stats->percent_similar = (static_cast<float>(similar_neighbours) / total_neighbours) * 100;
				global_stats->percent_unhappy = (static_cast<float>(it.world().count<Happy>()) / it.world().count<Agent>()) * 100;
			}
	);

	//world.system<const GlobalStats>("Task_PrintGlobalStats")
	//	.kind(flecs::PostFrame)
	//	.term_at(1).singleton()
	//	.iter([](flecs::iter& it, const GlobalStats* stats)
	//		{
	//			fmt::print("===== TICK {} =====\n", it.world().get_tick());
	//			fmt::print("Percent similar : {}%\n", stats[0].percent_similar);
	//			fmt::print("Percent unhappy : {}%\n", stats[0].percent_unhappy);
	//		}
	//	);

	world.system("StopCondition")
		.kind(flecs::PreFrame)
		.iter([&agents_query, &happy_agents](flecs::iter& it) 
			{
				if (agents_query.count() == happy_agents.count())
					it.world().quit();
			}
	);

	opack::run(world);

	return 0;
}
