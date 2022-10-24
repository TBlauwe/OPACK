#include <opack/core.hpp>					// Core header to use the library
#include <opack/module/simple_agent.hpp>	// Additional library header to
											// import a simple agent module

int main()
{
	// 1. Create an empty world.
	auto world = opack::create_world();

	// 2. Import a module. "simple" module defines a simple Agent with :
	// - one actuator,
	// - one sense,
	// - one flow.
	opack::import<simple>(world);

	// 3. Load a ".flecs" file to populate the world :
	// - One simple agent called "MyAgent"
	// - One simple agent called "MySuperAgent"
	opack::load(world, "plecs/simple_agent.flecs");

	// 4. Retrieve agent by name
	auto a = world.lookup("A");
	auto b = world.lookup("B");

	// 5. Get perception API for our agent "A"
	auto p_a = opack::perception(a);
	auto p_b = opack::perception(b);

	// 6. Interrogate perception
	fmt::print("Does A perceive B ? {}\n", p_a.perceive<simple::Sense>(b));
	fmt::print("Does B perceive A ? {}\n", p_b.perceive<simple::Sense>(a));

	// 7. Programmatically indicate that B perceive A.
	opack::perceive<simple::Sense>(b, a);
	fmt::print("Does B perceive A ? {}\n", p_b.perceive<simple::Sense>(a)); // true

	// As usual, let's run the world to inspect it here :
	// https://www.flecs.dev/explorer/?remote=true.
	opack::run_with_webapp(world);
}
