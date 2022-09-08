#include <opack/core.hpp>
#include <opack/module/simple_agent.hpp>

int main()
{
	auto world = opack::create_world();
	world.import<simple>();
	opack::spawn<simple::Agent>(world);
	opack::spawn<simple::Agent>(world);
	opack::spawn<simple::Agent>(world);
	opack::run_with_webapp(world);
}
