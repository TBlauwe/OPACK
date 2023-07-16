#include <chrono>
#include <iostream>

#include "src/model.hpp"

int main()
{
	// =========================================================================== 
	// Parameters
	// =========================================================================== 
	constexpr size_t height = 9;
	constexpr size_t width = 9;
	constexpr float density = .95f;
	constexpr float similar_wanted = .30;

	auto world = opack::create_world();
	auto shelling = Shelling<height, width>(world, density, similar_wanted);

	using clock = std::chrono::system_clock;
	using sec = std::chrono::duration<double>;
	// for milliseconds, use using ms = std::chrono::duration<double, std::milli>;

	const auto before = clock::now();

	//shelling.run(false, true, true);
	shelling.grid_display.display_stats();
	shelling.grid_display.display_grid();
	shelling.interactive_run(true, false);
	//opack::step_n(world, 100);
	//opack::run_with_webapp(world);

	const sec duration = clock::now() - before;

	std::cout << "It took " << duration.count() / 100 << "s" << std::endl;

	return 0;
}
