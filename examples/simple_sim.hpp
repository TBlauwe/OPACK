#pragma once

#include <opack/core.hpp>
#include <opack/utils/simulation_template.hpp>

struct SimpleSim : opack::SimulationTemplate
{
	// Types : sense
	// =============
	struct Hearing : opack::Sense{};
	struct Vision : opack::Sense{};

	// Types : action
	// ==============
	struct Act {};
	struct R {};

	// Types : data
	// ==============
	struct AudioMessage { const char* value; };

	SimpleSim(int argc = 0, char * argv[] = nullptr) : opack::SimulationTemplate{argc, argv}
	{
		sim.world.entity("::SimpleSim").add(flecs::Module);

		// Step I : Register types
		// -----------------------
		sim.register_sense<Hearing>();
		sim.perceive<Hearing, AudioMessage>();

		sim.register_sense<Vision>();
		sim.perceive<Vision, Act>();

		sim.world.observer()
			.term<Act>().obj().var("Object")
			.term<opack::Artefact>().subj().var("Object")
			.event(flecs::OnAdd)
			.iter(
				[](flecs::iter& iter)
				{
					for (auto i : iter)
					{
						auto e = iter.entity(i).object();
						auto obj = iter.id(1).object();
						std::cout << e.path() << " is acting on " << obj.path() << "\n";
					}
				}
		);

		// Step II : Populate world
		// ------------------------
		auto arthur		= sim.agent("Arthur");
		auto beatrice	= sim.agent("Beatrice");
		auto cyril		= sim.agent("Cyril");

		auto radio		= sim.artefact("Radio");

		// Fake a current state
		// --------------------
		arthur.add<Act>(cyril);
		cyril.add<Act>();
		beatrice.add<Act>(radio);
		radio.set<AudioMessage>({ "Hello there !" });

		sim.perceive<Vision, Hearing>(arthur, cyril);
		sim.perceive<Hearing>(arthur, radio);

		sim.perceive<Vision>(cyril, beatrice);

		sim.perceive<Vision, Hearing>(beatrice, cyril);
		sim.perceive<Vision, Hearing>(beatrice, radio);
	}
};