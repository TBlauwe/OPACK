#include <opack/utils/simulation_template.hpp>

#include <iostream>

opack::SimulationTemplate::SimulationTemplate(int argc, char* argv[]) : sim{ argc, argv }
{
	std::cout << "Building simulation ...\n";
}

opack::SimulationTemplate::~SimulationTemplate()
{
	std::cout << "=======================\n";
	std::cout << "Stopping simulation ...\n";
	sim.stop();
}

void opack::SimulationTemplate::run()
{
	sim.world.set<flecs::rest::Rest>({});
	std::cout << "Running simulation ...\n";
	std::cout << "======================\n";
	std::cout << "See web explorer on : https://www.flecs.dev/explorer/?remote=true\n";
	while (sim.step());
}
