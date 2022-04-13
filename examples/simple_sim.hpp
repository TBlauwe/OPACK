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
		help.add<opack::Continuous>();

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
					if (obj.has<AudioMessage>())
					{
						std::cout << entity.name() << " is hearing \"" << obj.get<AudioMessage>()->value << "\" from " << obj.name() << std::endl;
						entity.add<Help>(obj);
					}
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

		sim.world.system<Help>()
			.term<opack::Initiator>().obj(flecs::Wildcard)
			.iter(
				[](flecs::iter& iter )
				{
					for (auto i : iter)
					{ 
						auto entity = iter.entity(i);
						auto obj = iter.id(2);
						std::cout << entity.path() << " is initiating \"help\" with ";
						entity.each<opack::Initiator>([](flecs::entity obj) { std::cout << obj.path() << ", "; });
						std::cout << "\n";
					}
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
		radio.set<AudioMessage>({ "Hello there !" });

		sim.perceive<Vision, Hearing>(arthur, cyril);
		sim.perceive<Hearing>(arthur, radio);

		sim.perceive<Vision>(cyril, beatrice);

		sim.perceive<Vision, Hearing>(beatrice, cyril);
		sim.perceive<Vision, Hearing>(beatrice, radio);

		sim.conceal<Hearing>(arthur, radio);

		auto help_inst = sim.action<Help>();
		sim.act<Act>(arthur, help_inst);
	}
};