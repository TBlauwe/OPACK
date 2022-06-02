#pragma once

#include <iostream>

#include <opack/core.hpp>
#include <opack/module/activity_dl.hpp>
#include <opack/strategy/basic.hpp>

struct SimpleSim : opack::Simulation
{
	// Types : sense
	// =============
	struct MyCustomAgent : opack::Agent {};

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
	struct MyBehaviour : opack::Behaviour {};
	struct MyFlow : opack::Flow {};
	struct Operation_Percept : opack::Operation {};
	struct Operation_Reason : opack::Operation {};
	struct Operation_Act : opack::Operation {};
	struct Operation_UpdateStress : opack::Operation {};

	// Types : Stress 
	// =================
	struct Stress { float value = 10.f; };

	// Types : Activity-model
	// ======================

	SimpleSim(int argc = 0, char* argv[] = nullptr) : opack::Simulation{ argc, argv }
	{
		//sim.target_fps(1);
		world.entity("::SimpleSim").add(flecs::Module);
		world.import<adl>();

		// Step I : Register types
		// -----------------------
		// --- Actuator
		opack::register_actuator<Act>(world);
		world.component<Stress>().member<float>("value");

		// --- Actions
		auto help = opack::register_action<Help>(world);
		auto move = opack::register_action<Move>(world);
		auto tune = opack::register_action<Tune>(world); // TODO Automatic registration ?

		// --- Senses
		opack::register_sense<Hearing>(world);
		opack::perceive<Hearing, AudioMessage>(world);
		opack::perceive<Hearing, opack::Agent>(world);

		opack::register_sense<Vision>(world);
		opack::perceive<Vision, Act>(world);
		opack::perceive<Vision, opack::Agent>(world);

		// Step II : Additional dynamism
		// -----------------------------
		//world.observer("OnAdd_Hearing")
		//	.term<Hearing>().obj(flecs::Wildcard)
		//	.event(flecs::OnAdd)
		//	.iter(
		//		[](flecs::iter& iter)
		//		{
		//			auto id = iter.pair(1);
		//			auto obj = id.second();
		//			auto entity = iter.entity(0);
		//			if (obj.has<AudioMessage>())
		//			{
		//				entity.add<Help>(obj);
		//			}
		//		}
		//);

		//world.observer("OnRemove_Hearing")
		//	.term<Hearing>().obj(flecs::Wildcard)
		//	.event(flecs::OnRemove)
		//	.iter(
		//		[](flecs::iter& iter)
		//		{
		//			auto id = iter.pair(1);
		//			auto obj = id.second();
		//			auto entity = iter.entity(0);
		//		}
		//);

		world.system<const Help>("ActionHelpEffect")
			.term<opack::Delay>().oper(flecs::Not)
			.term<opack::By>().obj(flecs::Wildcard)
			.iter(
				[](flecs::iter& iter)
				{
					for (auto i : iter)
					{
						auto entity = iter.entity(i);
						auto obj = iter.id(2);
						entity.each<opack::By>([](flecs::entity obj) {});
						entity.destruct();
					}
				}
		);

		opack::flow<MyFlow>(world);

		opack::OperationBuilder<Operation_Percept>(world)
			.flow<MyFlow>()
			.build(
			[](flecs::entity agent) 
			{
				//std::cout << "Operation_Percept for " << agent.doc_name()  << "\n"; 
				//opack::each_perceived<opack::Sense, AudioMessage>(agent,
				//	[](flecs::entity subject, const AudioMessage& value)
				//	{
				//		std::cout << " - " << subject.doc_name() << " has " << value.value << "\n";
				//	}
				//);

				//opack::each_perceived_relation<opack::Sense, Act>(agent,
				//	[](flecs::entity subject, flecs::entity object)
				//	{
				//		std::cout << " - " << subject.doc_name() << " is acting on " << object.get_object<On>().doc_name() << "\n";
				//	}
				//);
			}
		);

		opack::OperationBuilder<Operation_UpdateStress, Stress>(world)
			.build(
				[](flecs::iter& iter, size_t index, Stress& stress)
				{
					stress.value -= iter.delta_system_time();
					if (stress.value <= 0)
						stress.value = 10;
				}
		);

		opack::OperationBuilder<Operation_Reason>(world)
			.flow<MyFlow>()
			.build(
			[](flecs::entity agent) 
			{
					agent.add<opack::Dataflow<Operation_Reason>>();
			}
		);

		// use typedef for operation and store input
		opack::OperationBuilder<Operation_Act, opack::Dataflow<Operation_Reason>, AudioMessage>(world)
			.flow<MyFlow>()
			.strategy<void>(&opack::strat::every<void, opack::Dataflow<Operation_Reason>, AudioMessage>)
			;

		opack::behaviour<MyBehaviour, const Stress>(world, [](flecs::entity e, const Stress& stress) {return stress.value > 5; });
		opack::impact<MyBehaviour, Operation_Act, void, opack::Dataflow<Operation_Reason>, AudioMessage>(world, 
			[](flecs::entity e, opack::Dataflow<Operation_Reason>& df, AudioMessage& msg)
			{
				std::cout << e.doc_name() << " has a dataflow of size : " << df.data.size() << "\n";
			}
		);

		// Step III : Populate world
		// -------------------------
		//auto prefab = opack::register_agent<MyCustomAgent>(world);
		//prefab.add<MyFlow>();
		//prefab.add<MyBehaviour>();
		//prefab.add<Stress>().override<Stress>();

		auto arthur = opack::agent(world, "Arthur");
		arthur
			.add<MyFlow>()
			.add<MyBehaviour>()
			.add<Stress>();
		auto beatrice = opack::agent(world, "Beatrice");
		beatrice
			.add<MyFlow>()
			.add<MyBehaviour>()
			.add<Stress>();
		auto cyril = opack::agent(world, "Cyril");
		cyril
			.add<MyFlow>()
			.add<MyBehaviour>()
			.add<Stress>();
		cyril.set<AudioMessage>({"I'm coming !"});

		auto radio = opack::artefact(world, "Radio");

		// (Step IV) : Fake a current state
		// --------------------------------
		opack::perceive<Vision, Hearing>(arthur, cyril);
		opack::perceive<Hearing>(arthur, radio);

		opack::perceive<Vision>(cyril, beatrice);
		radio.set<AudioMessage>({ "Hello there !" });

		opack::perceive<Vision, Hearing>(beatrice, cyril);
		opack::perceive<Vision, Hearing>(beatrice, radio);

		//opack::conceal<Hearing>(world, arthur, radio);

		{
			auto action = opack::action<Help>(world);
			action.set<opack::Delay>({ 6.0f });
			action.add<On>(cyril);
			opack::act<Act>(arthur, action);
		}

		{
			auto action = opack::action<Move>(world);
			action.set<opack::Delay>({ 4.0f });
			action.add<On>(beatrice);
			opack::act<Act>(cyril, action);
		}

		{
			auto action = opack::action<Tune>(world);
			action.set<opack::Delay>({ 2.0f });
			action.add<On>(radio);
			opack::act<Act>(beatrice, action);
		}
	}
};