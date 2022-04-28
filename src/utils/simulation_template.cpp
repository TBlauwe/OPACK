#include <opack/utils/simulation_template.hpp>
opack::SimulationTemplate::SimulationTemplate(int argc, char* argv[]) : sim { argc, argv }
{
}

opack::SimulationTemplate::~SimulationTemplate()
{
}

void opack::SimulationTemplate::run()
{
	sim.rest_app();
}
