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
	struct Behaviour {};
	struct MyBehaviour {};
	struct MyFlow : opack::Flow {};
	struct Operation_Percept : opack::Operation {};
	struct Operation_Reason : opack::Operation {};
	struct Operation_Act : opack::Operation {};

	// Types : Stress 
	// =================
	struct Stress { float value = 10.f; };

	SimpleSim(int argc = 0, char* argv[] = nullptr) : opack::SimulationTemplate{ argc, argv }
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
		sim.world.observer("OnAdd_Hearing")
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

		sim.world.observer("OnRemove_Hearing")
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

		sim.world.system<const Help>("ActionHelpEffect")
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

		//opack::flow<MyFlow>(sim);
		//opack::operation<MyFlow, Operation_Percept>(sim, [](flecs::entity agent, const Operation_Percept){});

		sim.world.system<Stress>("UpdateStress")
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
		auto arthur = opack::agent(sim, "Arthur");
		//arthur.add<MyFlow>(flow);
		arthur.add<Stress>();
		auto beatrice = opack::agent(sim, "Beatrice");
		//beatrice.add<MyFlow>(flow);
		beatrice.add<Stress>();
		auto cyril = opack::agent(sim, "Cyril");

		auto radio = opack::agent(sim, "Radio");

		// (Step IV) : Fake a current state
		// --------------------------------
		opack::perceive<Vision, Hearing>(sim, arthur, cyril);
		opack::perceive<Hearing>(sim, arthur, radio);

		opack::perceive<Vision>(sim, cyril, beatrice);
		radio.set<AudioMessage>({ "Hello there !" });

		opack::perceive<Vision, Hearing>(sim, beatrice, cyril);
		opack::perceive<Vision, Hearing>(sim, beatrice, radio);

		opack::conceal<Hearing>(sim, arthur, radio);

		{
			auto action = opack::action<Help>(sim);
			action.set<opack::Delay>({ 6.0f });
			action.add<On>(cyril);
			opack::act<Act>(sim, arthur, action);
		}

		{
			auto action = opack::action<Move>(sim);
			action.set<opack::Delay>({ 4.0f });
			action.add<On>(beatrice);
			opack::act<Act>(sim, cyril, action);
		}

		{
			auto action = opack::action<Tune>(sim);
			action.set<opack::Delay>({ 2.0f });
			action.add<On>(radio);
			opack::act<Act>(sim, beatrice, action);
		}
	}
};