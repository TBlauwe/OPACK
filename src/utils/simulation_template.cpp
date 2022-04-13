#include <opack/utils/simulation_template.hpp>

#include <iostream>

opack::SimulationTemplate::SimulationTemplate(const char* _name, int argc, char* argv[]) : name{ _name }, sim { argc, argv }
{
	std::cout << "----------------------\n";
	std::cout << "Building simulation \""<< name << "\" ...\n";
	std::cout << "---- Build Events ----\n";
}

opack::SimulationTemplate::~SimulationTemplate()
{
	std::cout << "=======================\n";
	std::cout << "Stopping simulation \""<< name << "\"...\n";
	sim.stop();
}

void opack::SimulationTemplate::run()
{
	std::cout << "----------------------\n";
	std::cout << "Running simulation \""<< name << "\"...\n";
	std::cout << "----Running events----\n";
	sim.rest_app();
}
