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
	struct Act : opack::Actuator {};

	struct Help : opack::Action {};
	struct Move : opack::Action {};


	// Types : data
	// ============
	struct AudioMessage { const char* value; };

	// Types : Relation
	// ================
	struct R {};

	SimpleSim(int argc = 0, char * argv[] = nullptr) : opack::SimulationTemplate{"SimpleSim", argc, argv}
	{
		sim.world.entity("::SimpleSim").add(flecs::Module);

		// Step I : Register types
		// -----------------------
		// --- Actuator
		sim.register_actuator_type<Act>();

		// --- Actions
		auto help = sim.register_action<Help>();
		help.add<opack::Action::Continuous>();

		// --- Senses
		sim.register_sense<Hearing>();
		sim.perceive<Hearing, AudioMessage>();

		sim.register_sense<Vision>();
		sim.perceive<Vision, Act>();

		// Step II : Additional dynamism
		// -----------------------------
		sim.world.observer()
			.term<Hearing>().obj(flecs::Wildcard)
			.event(flecs::OnAdd)
			.iter(
				[](flecs::iter& iter )
				{
					auto id		= iter.pair(1);
					auto obj	= id.second();
					auto entity = iter.entity(0);
					if(obj.has<AudioMessage>())
						std::cout << entity.name() << " is hearing \"" << obj.get<AudioMessage>()->value << "\" from " << obj.name() << std::endl;
					else
						std::cout << entity.name() << " is hearing " << obj.name() << std::endl;
				}
		);

		sim.world.observer()
			.term<Hearing>().obj(flecs::Wildcard)
			.event(flecs::OnRemove)
			.iter(
				[](flecs::iter& iter )
				{
					auto id		= iter.pair(1);
					auto obj	= id.second();
					auto entity = iter.entity(0);
					std::cout << entity.name() << " stopped hearing " << obj.name() << std::endl;
				}
		);

		// Step III : Populate world
		// -------------------------
		auto arthur		= sim.agent("Arthur");
		auto beatrice	= sim.agent("Beatrice");
		auto cyril		= sim.agent("Cyril");

		auto radio		= sim.artefact("Radio");

		// (Step IV) : Fake a current state
		// --------------------------------
		arthur.add<Help>(cyril);
		beatrice.add<Act>(radio);
		radio.set<AudioMessage>({ "Hello there !" });

		sim.perceive<Vision, Hearing>(arthur, cyril);
		sim.perceive<Hearing>(arthur, radio);

		sim.perceive<Vision>(cyril, beatrice);

		sim.perceive<Vision, Hearing>(beatrice, cyril);
		sim.perceive<Vision, Hearing>(beatrice, radio);

		sim.step();

		sim.conceal<Hearing>(arthur, radio);
	}
};