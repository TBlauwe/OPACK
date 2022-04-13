#pragma once

#include <opack/core.hpp>
#include <opack/utils/simulation_template.hpp>

struct SimpleSim : opack::SimulationTemplate
{
	// Types : sense
	// =============
	struct Hearing : opack::Sense {};
	struct Vision : opack::Sense {};

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

	SimpleSim(int argc = 0, char* argv[] = nullptr) : opack::SimulationTemplate{ "SimpleSim", argc, argv }
	{
		//sim.target_fps(1);
		sim.world.entity("::SimpleSim").add(flecs::Module);

		// Step I : Register types
		// -----------------------
		// --- Actuator
		opack::register_actuator_type<Act>(sim);

		// --- Actions
		auto help = opack::register_action<Help>(sim);
		help.add<opack::Continuous>();

		// --- Senses
		opack::register_sense<Hearing>(sim);
		opack::perceive<Hearing, AudioMessage>(sim);

		opack::register_sense<Vision>(sim);
		opack::perceive<Vision, Act>(sim);

		// Step II : Additional dynamism
		// -----------------------------
		sim.world.observer()
			.term<Hearing>().obj(flecs::Wildcard)
			.event(flecs::OnAdd)
			.iter(
				[](flecs::iter& iter)
				{
					auto id = iter.pair(1);
					auto obj = id.second();
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
				[](flecs::iter& iter)
				{
					auto id = iter.pair(1);
					auto obj = id.second();
					auto entity = iter.entity(0);
					std::cout << entity.name() << " stopped hearing " << obj.name() << std::endl;
				}
		);

		sim.world.system<Help>()
			.term<opack::Initiator>().obj(flecs::Wildcard)
			.iter(
				[](flecs::iter& iter)
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

		sim.world.system<Help>()
			.term<opack::Initiator>().obj(flecs::Wildcard)
			.iter(
				[](flecs::iter& iter)
				{
					for (auto i : iter)
					{
						auto entity = iter.entity(i);
						auto obj = iter.id(2);
						std::cout << entity.path() << " is also initiating \"help\" with ";
						entity.each<opack::Initiator>([](flecs::entity obj) { std::cout << obj.path() << ", "; });
						std::cout << "\n";
					}
				}
		);

		// Step III : Populate world
		// -------------------------
		auto arthur		= opack::agent(sim, "Arthur");
		auto beatrice	= opack::agent(sim, "Beatrice");
		auto cyril		= opack::agent(sim, "Cyril");

		auto radio		= opack::artefact(sim, "Radio");

		// (Step IV) : Fake a current state
		// --------------------------------
		radio.set<AudioMessage>({ "Hello there !" });

		opack::perceive<Vision, Hearing>(sim, arthur, cyril);
		opack::perceive<Hearing>(sim, arthur, radio);

		opack::perceive<Vision>(sim, cyril, beatrice);

		opack::perceive<Vision, Hearing>(sim, beatrice, cyril);
		opack::perceive<Vision, Hearing>(sim, beatrice, radio);

		opack::conceal<Hearing>(sim, arthur, radio);


		sim.world.system<opack::Agent>()
			.interval(5)
			.iter(
				[&](flecs::iter& iter)
				{
					for (auto i : iter)
					{
						auto entity = iter.entity(i);
						auto world = entity.world();
						auto action = opack::action<Help>(world);
						std::cout << "Action has arity : " << action.has<opack::Arity>() << "\n";
						opack::act<Act>(world, entity, action);
					}
				}
		);
	}
};