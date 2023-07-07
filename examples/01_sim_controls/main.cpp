#include <opack/core.hpp> // Core header to use the library

int main()
{
	// 1. Create an empty world.
	auto world = opack::create_world();

	// 2. Indicates how many times it should tried to run per second
	opack::target_fps(world, 30);	// --> There will be no more than
									// 30 cycle per second.

	// --convention used--:
	// ValueType value(world) -> value's getter
	// void value(world, value) -> value's setter
	// e.g : 
	opack::target_fps(world, 30); // --> setter
	auto target_fps = opack::target_fps(world); // --> getter


	// 3. Indicates how fast time flies.
	// 1.0 -> 1 second in real life means 1 second in simulation time
	// 2.0 -> 1 second in real life means 2 second in simulation time
	// 0.5 -> 1 second in real life means 0.5 second in simulation time
	opack::time_scale(world, 2.0f);	

	// 4. Returns number of elapsed tick.
	opack::tick(world);	

	// 5. Returns delta time or elapsed time between two ticks.
	opack::delta_time(world);	

	// 6. Returns total elapsed time in simulation (so it consider time scale).
	opack::time(world);	

	// 7. Advance simulation by
	// ---- 1. one step with automatic delta time computation
	opack::step(world);	

	// ---- 2. one step with specified delta time (no time scale applied)
	opack::step(world, 2.0f);	// 2 seconds have passed in simulation time

	// ---- 3. n steps with automatic delta time
	opack::step_n(world, 10);	// 10 cycle will happen.

	// ---- 4. n steps with specified delta time (no time scale applied)
	opack::step_n(world, 10, 2.0f);	// 10 cycle will happen with 2 seconds passed between each cycle.

	// Not explained here
	{
		world.system("System_PrintTick")	// Define a system called "System_PrintTick"
		.kind<opack::Cycle::Begin>()		// -- that will run at the beginning
		.iter([&](flecs::iter& iter)
			{
				fmt::print("Tick [{}] - {}s\n", iter.world().get_tick(), iter.world().time());
			}
		);
	}

	// 8. Run world in automatic mode
	// ---- 1. with web app activated so we can inspect it here : https://www.flecs.dev/explorer/?remote=true.
	opack::run_with_webapp(world);
	// ---- 2. without web app
	opack::run(world);

	// 9. Stop running world (must be called in a system, meaning this will do nothing
	// here, as we do not return from previous `run` functions.
	opack::stop(world);
}
