#include <chrono>
#include <iostream>

#include "src/model.hpp"

int main()
{
	// =========================================================================== 
	// Parameters
	// =========================================================================== 
	constexpr size_t height = 100;
	constexpr size_t width = 100;
	constexpr float density = .95f;
	constexpr float similar_wanted = .30;

	auto world = opack::create_world();
	auto shelling = Shelling<height, width>(world, density, similar_wanted);

	//world.system<const GlobalStats>("Task_PrintGlobalStats")
	//	.kind(flecs::PostFrame)
	//	.term_at(1).singleton()
	//	.iter([&shelling](flecs::iter& it, const GlobalStats* stats)
	//		{
	//			fmt::print("===== TICK {} =====\n", it.world().get_tick());
	//			fmt::print("Percent similar : {}%\n", stats[0].percent_similar);
	//			fmt::print("Percent unhappy : {}%\n", stats[0].percent_unhappy);
	//			fmt::print("Total agents : {}\n", shelling.agents_query.count());
	//			fmt::print("Happy agents : {}\n", shelling.happy_agents.count());
	//		}
	//	);

	using clock = std::chrono::system_clock;
	using sec = std::chrono::duration<double>;
	// for milliseconds, use using ms = std::chrono::duration<double, std::milli>;

	const auto before = clock::now();

	opack::step_n(world, 100);

	const sec duration = clock::now() - before;

	std::cout << "It took " << duration.count() / 100 << "s" << std::endl;

	return 0;
}
