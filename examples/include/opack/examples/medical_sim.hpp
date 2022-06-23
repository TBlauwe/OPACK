#pragma once

#include <opack/core.hpp>
#include <opack/operations/influence_graph.hpp>
#include <opack/module/activity_dl.hpp>

struct MedicalSim : opack::Simulation
{
	// -------------------
	// --- Environment ---
	// -------------------
	struct Base {}; // Relation
	struct Near {}; // Relation
	struct is_friend {};

	struct Qualification { size_t value{ 0 }; };

	struct FC { size_t value{ 70 }; };
	struct FCBase { size_t value { 70 }; float var{ 5 };};

	struct has_order {};

	// -------------------------
	// --- Agent & Artefacts ---
	// -------------------------
	struct Patient	: opack::Artefact {};
	struct Nurse	: opack::Agent {};


	// -------------
	// --- OPACK ---
	// -------------

	// --- Percept
	struct Hearing : opack::Sense {};
	struct Vision : opack::Sense {};

	// --- Action

	// --- Action
	struct Voice : opack::Actuator {};
	struct Body : opack::Actuator {};

	struct Communicative {};

	// --- Activity
	struct March : adl::Activity {};
	struct Examine : opack::Action {};
	struct Treat : opack::Action {};

	// --- Caracteristic
	struct UnWell {};
	struct IsCoughing {};

	// --- Flow & Operation
	struct Flow : opack::Flow{};

	struct UpdateKnowledge : opack::operations::All<> {};
	struct Filter : opack::operations::All<> {};
	struct SuitableActions : opack::operations::Union<flecs::entity> {};
	struct ActionSelection : opack::operations::SelectionByIGraph<SuitableActions> {};
	struct Act : opack::operations::All<opack::df<ActionSelection, typename ActionSelection::output>> {};
	struct UpdateStress : opack::operations::All<> {};

	// --- Cognitive Model
	struct Behaviour_Friendship : opack::Behaviour {};
	struct Behaviour_Consistent : opack::Behaviour {};
	struct Behaviour_Stress : opack::Behaviour {};

	MedicalSim(int argc = 0, char * argv[] = nullptr) : opack::Simulation{argc, argv}
	{

		world.import<adl>();

		// --- Reflection for visualisation
		{
			world.component<Near>()
				.add(flecs::Exclusive)
				;

			world.component<Vision>()
				;

			world.component<FC>()
				.member<size_t>("value")
				;

			world.component<FCBase>()
				.member<size_t>("value")
				;
		}

		// --- Environment definition
		{
		}
		auto patient_query = world.query_builder().term<Patient>().build();
		
		// --- World dynamics
		{
			world.system<const FCBase, FC>("UpdateFC")
				.iter(
					[](flecs::iter& it, const FCBase* fcb, FC* fc)
					{
						for (auto i : it)
						{
							fc[i].value = fcb[i].value + (fcb[i].var * sin(it.world().time()));
						}
					}
				).child_of<opack::world::Dynamics>();

			world.system<const Nurse>("UpdateVision")
				.kind(flecs::PreUpdate)
				.iter(
					[&patient_query](flecs::iter& iter, const Nurse* _)
					{
						for (auto i : iter)
						{
							auto agent = iter.entity(i);
							auto world = agent.world();
							world.filter<const Patient>().each(
								[&agent](flecs::entity patient, const Patient)
								{
									agent.add<Vision>(patient);
									agent.add<Hearing>(patient);
								}
							);
						}
					}
				).child_of<opack::world::Dynamics>();

			world.system<const Examine>("ExamineEffect")
				.kind(flecs::PreUpdate)
				.term<opack::By>().obj(flecs::Wildcard)
				.term<opack::On>().obj(flecs::Wildcard)
				.term<adl::Satisfied>().inout(flecs::Out).set(flecs::Nothing)
				.term<opack::End, opack::Timestamp>().oper(flecs::Not)
				.term<opack::End, opack::Timestamp>().inout(flecs::Out).set(flecs::Nothing)
				.iter(
					[](flecs::iter& iter)
					{
						for (auto i : iter)
						{
							auto action = iter.entity(i);
							auto initiator = action.get_object<opack::By>();
							auto patient = action.get_object<opack::On>();
							std::cout << initiator.doc_name() << " is doing " << action.doc_name() << " on " << action.get_object<opack::On>().doc_name() << "\n";
							action.set<opack::End, opack::Timestamp>({iter.world().time()});
							action.add<adl::Satisfied>();
							initiator.mut(iter).remove<Body>(action);
						}
					}
				);

			world.system<const Treat>("TreatEffect")
				.kind(flecs::PreUpdate)
				.term<opack::By>().obj(flecs::Wildcard)
				.term<opack::On>().obj(flecs::Wildcard)
				.term<adl::Satisfied>().inout(flecs::Out).set(flecs::Nothing)
				.term<opack::End, opack::Timestamp>().oper(flecs::Not)
				.term<opack::End, opack::Timestamp>().inout(flecs::Out).set(flecs::Nothing)
				.iter(
					[](flecs::iter& iter)
					{
						for (auto i : iter)
						{
							auto action = iter.entity(i);
							auto initiator = action.get_object<opack::By>();
							auto patient = action.get_object<opack::On>();
							std::cout << initiator.doc_name() << " is doing " << action.doc_name() << " on " << action.get_object<opack::On>().doc_name() << "\n";
							action.set<opack::End, opack::Timestamp>({iter.world().time()});
							action.add<adl::Satisfied>();
							initiator.mut(iter).remove<Body>(action);
						}
					}
				);
		}

		// --- Activity
		{
			opack::reg_n<Examine, Treat>(world);
			auto march = adl::activity<March>(world, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
			auto examine = adl::action<Examine>(march);
			auto treat = adl::action<Treat>(march);
		}

		// --- Types definition
		{
			opack::reg<Hearing>(world);
			opack::perceive<Hearing, IsCoughing>(world);

			opack::reg<Vision>(world);
			opack::perceive<Vision, Patient, FC>(world);

			opack::reg<Body>(world);
			opack::reg<Voice>(world);
		}

		// --- Flow definition
		{
			opack::FlowBuilder<Flow>(world).interval().build();
			opack::operation<Flow, UpdateKnowledge, SuitableActions, ActionSelection, Act>(world);

			opack::default_impact<UpdateKnowledge>(world,
				[](flecs::entity agent, UpdateKnowledge::inputs& inputs)
				{
					opack::each_perceived<Patient>(agent,
						[&agent](flecs::entity subject)
						{
							auto world = agent.world();
							if (!agent.has(subject, flecs::Wildcard))
							{
								auto instance = adl::instantiate<March>(world);
								agent.add(subject, instance);
							}
						}
					);
					return opack::make_outputs<UpdateKnowledge>();
				}
				);

			opack::default_impact<SuitableActions>(world,
				[](flecs::entity agent, SuitableActions::inputs& inputs)
				{
					opack::each_perceived<Patient>(agent,
						[&inputs, agent](flecs::entity subject)
						{
							auto activity = agent.get_object(subject);
							if (activity)
							{
								std::vector<flecs::entity> actions{};
								adl::potential_actions(activity, std::back_inserter(actions));
								for (auto action : actions)
								{
									action.add<opack::On>(subject);
									SuitableActions::iterator(inputs) = action;
								}

							}
						}
					);
					return opack::make_outputs<SuitableActions>();
				}
				);

			opack::default_impact<ActionSelection>(world,
				[](flecs::entity agent, ActionSelection::inputs& inputs)
				{				
					const auto id = ActionSelection::get_influencer(inputs);
					auto& actions = ActionSelection::get_choices(inputs);
					auto& graph = ActionSelection::get_graph(inputs);
					for (auto& a : actions)
					{
						graph.entry(a);
					}

					return opack::make_outputs<ActionSelection>();
				}
			);

			opack::default_impact<Act>(world,
				[](flecs::entity agent, Act::inputs& inputs)
				{
					auto action = std::get<opack::df<ActionSelection, typename ActionSelection::output>&>(inputs).value;
					if (action)
					{
						opack::act<Body>(agent, action);
					}
					return opack::make_outputs<Act>();
				}
			);
		}

		// --- Cognitive models controls
		bool active_consistent{ true };
		bool active_friendship{ true };

		// --- Cognitive models definition
		{
			opack::behaviour<Behaviour_Consistent>(world, [&active_consistent](flecs::entity agent) {return active_consistent; });
			opack::impact<ActionSelection, Behaviour_Consistent>(world,
				[](flecs::entity agent, ActionSelection::inputs& inputs)
				{
					const auto id = ActionSelection::get_influencer(inputs);
					auto& actions = ActionSelection::get_choices(inputs);
					auto& graph = ActionSelection::get_graph(inputs);
					for (auto& a : actions)
					{
						auto patient = a.get_object<opack::On>();
						auto procedure = agent.get_object(patient);
						if (!adl::has_started(procedure) && !adl::is_finished(procedure))
						{
							graph.positive_influence(id, a);
						}
					}
					return opack::make_outputs<ActionSelection>();
				}
			);

			opack::behaviour<Behaviour_Friendship>(world, [&active_friendship](flecs::entity agent) {return active_friendship; });
			opack::impact<ActionSelection, Behaviour_Friendship>(world,
				[](flecs::entity agent, ActionSelection::inputs& inputs)
				{
					const auto id = ActionSelection::get_influencer(inputs);
					auto& actions = ActionSelection::get_choices(inputs);
					auto& graph = ActionSelection::get_graph(inputs);
					for (auto& a : actions)
					{
						auto patient = a.get_object<opack::On>();
						if(agent.has<is_friend>(patient))
						{
							graph.positive_influence(id, a);
						}
					}
					return opack::make_outputs<ActionSelection>();
				}
			);
		}

		// --- World population
		{
			// Defines a "Patient" prefab
			opack::reg<Patient>(world)
				.override<FC>()
				.override<FCBase>()
				;

			// Defines a "Nurse" prefab
			opack::reg<Nurse>(world)
				.add<Flow>()	// Reason using "Flow"
				.override<Qualification>()
				;

			auto patient_1 = opack::artefact<Patient>(world, "Patient 1");
			auto patient_2 = opack::artefact<Patient>(world, "Patient 2");
			auto patient_3 = opack::artefact<Patient>(world, "Patient 3");
			auto patient_4 = opack::artefact<Patient>(world, "Patient 4");
			auto patient_5 = opack::artefact<Patient>(world, "Patient 5");
			auto patient_6 = opack::artefact<Patient>(world, "Patient 6");

			auto nurse = opack::agent<Nurse>(world, "Nurse 1");
			nurse.add<is_friend>(patient_3);
		}
	}
};