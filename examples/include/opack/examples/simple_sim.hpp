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
	struct Tune : opack::Action {};

	// Types : data
	// ============
	struct AudioMessage { const char* value; };

	// Types : Relation
	// ================
	struct R {};
	struct On {};

	// Types : Behaviour
	// =================
	struct MyBehaviour {};

	// Types : Stress 
	// =================
	struct Stress { float value = 10.f; };

	SimpleSim(int argc = 0, char* argv[] = nullptr) : opack::SimulationTemplate{argc, argv }
	{
		//sim.target_fps(1);
		sim.world.entity("::SimpleSim").add(flecs::Module);

		// Step I : Register types
		// -----------------------
		// --- Actuator
		opack::register_actuator_type<Act>(sim);

		// --- Actions
		auto help = opack::register_action<Help>(sim);
		auto move = opack::register_action<Move>(sim);
		auto tune = opack::register_action<Tune>(sim); // TODO Automatic registration ?

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
						entity.add<Help>(obj);
					}
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

		sim.world.system<const Help>()
			.term<opack::Delay>().oper(flecs::Not)
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
						entity.destruct();
					}
				}
		);

		sim.world.system<Stress>()
			.iter(
				[](flecs::iter& iter, Stress* stress)
				{
					for (auto i : iter)
					{
						stress[i].value -= iter.delta_system_time();
						if (stress[i].value <= 0)
							stress[i].value = 10;
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
		opack::perceive<Vision, Hearing>(sim, arthur, cyril);
		opack::perceive<Hearing>(sim, arthur, radio);

		opack::perceive<Vision>(sim, cyril, beatrice);
		radio.set<AudioMessage>({ "Hello there !" });

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
						flecs::entity action;
						switch (rand() % 3)
						{
						case 0:
							action = opack::action<Help>(world);
							action.set<opack::Delay>({ 6.0f });
							action.add<On>(cyril);
							break;
						case 1:
							action = opack::action<Move>(world);
							action.set<opack::Delay>({ 4.0f });
							action.add<On>(beatrice);
							break;
						case 2:
							action = opack::action<Tune>(world);
							action.set<opack::Delay>({ 2.0f });
							action.add<Tune>(radio);
							break;
						}
						opack::act<Act>(world, entity, action);
					}
				}
		);
	}
};