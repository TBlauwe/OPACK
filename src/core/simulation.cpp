#include <opack/core/simulation.hpp>

opack::Simulation::Simulation()
{
}

opack::Simulation::~Simulation()
{
	executor.wait_for_all();
}
