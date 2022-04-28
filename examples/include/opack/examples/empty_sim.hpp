#pragma once

#include <opack/core.hpp>
#include <opack/utils/simulation_template.hpp>

struct EmptySim : opack::SimulationTemplate
{
	EmptySim(int argc = 0, char * argv[] = nullptr) : opack::SimulationTemplate{argc, argv}
	{
	}
};