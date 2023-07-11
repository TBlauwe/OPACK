#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <opack/core.hpp>
#include <opack/operations/basic.hpp>
#include <opack/operations/influence_graph.hpp>
#include <opack/utils/ring_buffer.hpp>
#include <opack/module/simple_agent.hpp>

OPACK_ACTION(Action);
OPACK_ACTUATOR(Actuator);
OPACK_SENSE(Sense);

int main()
{
    // =========================================================================== 
    // Parameters
    // =========================================================================== 
    
    // =========================================================================== 
    // World definition
    // =========================================================================== 
	auto world = opack::create_world();

    opack::init<Action>(world).require<simple::Actuator>().add<Color>();

	opack::run_with_webapp(world);

    return 0;
}
