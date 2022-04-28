#include <opack/utils/simulation_template.hpp>

#include <iostream>

#include <fmt/format.h>

opack::SimulationTemplate::SimulationTemplate(const char* name, int argc, char* argv[]) : sim { argc, argv }
{
	std::cout << "Building simulation \""<< name << "\" ...\n";
	sim.world.entity(fmt::format("::{}", name).c_str()).add(flecs::Module);
}

opack::SimulationTemplate::~SimulationTemplate()
{
	std::cout << "=======================\n";
	std::cout << "Stopping simulation ...\n";
	sim.stop();
}

void opack::SimulationTemplate::run()
{
	std::cout << "Running simulation ...\n";
	std::cout << "=======================\n";
	sim.rest_app();
}
