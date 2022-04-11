#define FLECS_APP
#include <opack/core.hpp>

struct SimpleSim
{
	opack::Simulation sim;

	SimpleSim(int argc = 0, char * argv[] = nullptr) : sim{argc, argv}
	{
		sim.agent("Arthur");
		sim.agent("Beatrice");
		sim.agent("Cyril");

		sim.artefact("Radio");
	}

	void run()
	{
		sim.flecs_world().set<flecs::rest::Rest>({});
		while (sim.step());
	}
};