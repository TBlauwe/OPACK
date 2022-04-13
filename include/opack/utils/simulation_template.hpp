#pragma once
#include <opack/core/simulation.hpp>

namespace opack
{
	struct SimulationTemplate
	{
		SimulationTemplate(const char* _name = "Unspecified", int argc = 0, char* argv[] = nullptr);
		~SimulationTemplate();

		void run();

		const char* name;
		Simulation sim;
	};
}
