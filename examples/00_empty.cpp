#include <opack/core.hpp>

int main()
{
	// 1. Create an empty world.
	auto world = opack::create_world();

	// 2. Run world with web app activated so we can inspect it here :
	// https://www.flecs.dev/explorer/?remote=true.
	opack::run_with_webapp(world);
}
