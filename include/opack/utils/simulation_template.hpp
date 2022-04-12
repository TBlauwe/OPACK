#pragma once
#include <opack/core/simulation.hpp>

namespace opack
{
	struct SimulationTemplate
	{
		SimulationTemplate(int argc = 0, char* argv[] = nullptr);
		~SimulationTemplate();

		void run();

		Simulation sim;
	};
}
