#include <opack/core.hpp>
#include <opack/module/simple_agent.hpp>

int main()
{
	auto world = opack::create_world();
	world.import<simple>();
	world.plecs_from_file("plecs/simple_agent.flecs");
	opack::run_with_webapp(world);
}
