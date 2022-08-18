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

	struct Order { flecs::entity patient; };

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
	struct Contest : opack::Action {};
	struct Comply : opack::Action {};
	struct Ignore : opack::Action {};
	struct Communicative {};

	// --- Action
	struct Voice : opack::Actuator {};
	struct Body : opack::Actuator {};


	// --- Activity
	struct March : adl::Activity {};
	struct Examine : opack::Action {};
	struct Treat : opack::Action {};

	// --- Caracteristic
	struct UnWell {};
	struct IsCoughing {};
	struct WaitOrder {};
	struct FollowOrder {};

	struct ProActive {};
	struct Regulatory {};
	struct Stressed {};

	// --- Flow & Operation
	struct Flow : opack::Flow{};

	struct UpdateKnowledge : opack::operations::All<> {};
	struct Filter : opack::operations::All<> {};
	struct SuitableActions : opack::operations::Union<flecs::entity> {};
	struct ActionSelection : opack::operations::SelectionByIGraph<SuitableActions> {};
	struct Act : opack::operations::All<opack::df<ActionSelection, typename ActionSelection::output>> {};
	struct UpdateStress : opack::operations::All<> {};

	struct CommunicationFlow : opack::Flow{};

	struct SuitableAnswers : opack::operations::Union<flecs::entity> {};
	struct AnswerSelection : opack::operations::SelectionByIGraph<SuitableAnswers> {};
	struct Communicate : opack::operations::All<opack::df<AnswerSelection, typename AnswerSelection::output>> {};

	// --- Cognitive Model
	struct Behaviour_Friendship : opack::Behaviour {};
	struct Behaviour_Consistent : opack::Behaviour {};
	struct Behaviour_Stress : opack::Behaviour {};
	struct Behaviour_Regulatory : opack::Behaviour {};
	struct Behaviour_ProActive : opack::Behaviour {};
	struct Behaviour_Communicative : opack::Behaviour {};
	struct Behaviour_Passive : opack::Behaviour {};

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
		
		// --- World dynamics
		{
			auto patient_query = world.query_builder().term<Patient>().build();
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

			world.observer<Order>("Event_order")
				.event(flecs::OnSet)
				.iter(
					[](flecs::iter& it, Order* order)
					{
						for (auto i : it)
						{
							auto agent = it.entity(i);
							std::cout << agent.doc_name() << " has been ordered to treat " << order[i].patient.doc_name() << "\n";
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
				.term<opack::By>().second(flecs::Wildcard)
				.term<opack::On>().second(flecs::Wildcard)
				.term<adl::Satisfied>().write()
				.term<opack::End, opack::Timestamp>().oper(flecs::Not)
				.term<opack::End, opack::Timestamp>().write()
				.iter(
					[](flecs::iter& iter)
					{
						for (auto i : iter)
						{
							auto action = iter.entity(i);
							auto initiator = action.target<opack::By>();
							auto patient = action.target<opack::On>();
							std::cout << initiator.doc_name() << " is doing " << action.doc_name() << " on " << action.target<opack::On>().doc_name() << "\n";
							action.set<opack::End, opack::Timestamp>({iter.world().time()});
							action.add<adl::Satisfied>();
							initiator.mut(iter).remove<Body>(action);
						}
					}
				);

			world.system<const Treat>("TreatEffect")
				.kind(flecs::PreUpdate)
				.term<opack::By>().second(flecs::Wildcard)
				.term<opack::On>().second(flecs::Wildcard)
				.term<adl::Satisfied>().write()
				.term<opack::End, opack::Timestamp>().oper(flecs::Not)
				.term<opack::End, opack::Timestamp>().write()
				.iter(
					[](flecs::iter& iter)
					{
						for (auto i : iter)
						{
							auto action = iter.entity(i);
							auto initiator = action.target<opack::By>();
							auto patient = action.target<opack::On>();
							patient.remove<UnWell>();
							std::cout << initiator.doc_name() << " is doing " << action.doc_name() << " on " << action.target<opack::On>().doc_name() << "\n";
							action.set<opack::End, opack::Timestamp>({iter.world().time()});
							action.add<adl::Satisfied>();
							initiator.mut(iter).remove<Body>(action);
						}
					}
				);

			world.system<const Comply>("ComplyAnswer")
				.kind(flecs::PreUpdate)
				.term<opack::By>().second(flecs::Wildcard)
				.term<opack::End, opack::Timestamp>().write()
				.iter(
					[](flecs::iter& iter)
					{
						for (auto i : iter)
						{
							auto action = iter.entity(i);
							auto initiator = action.target<opack::By>();
							std::cout << initiator.doc_name() << " complied with order to treat " << initiator.get_mut<Order>()->patient.doc_name() << "\n";
							action.set<opack::End, opack::Timestamp>({iter.world().time()});
							initiator.add<FollowOrder>();
							action.destruct();
						}
					}
				);

			world.system<const Ignore>("IgnoreAnswer")
				.kind(flecs::PreUpdate)
				.term<opack::By>().second(flecs::Wildcard)
				.iter(
					[](flecs::iter& iter)
					{
						for (auto i : iter)
						{
							auto action = iter.entity(i);
							auto initiator = action.target<opack::By>();
							std::cout << initiator.doc_name() << " is not communicating\n";
							action.destruct();
						}
					}
				);

			world.system<const Contest>("ContestAnswer")
				.kind(flecs::PreUpdate)
				.term<opack::By>().second(flecs::Wildcard)
				.iter(
					[](flecs::iter& iter)
					{
						for (auto i : iter)
						{
							auto action = iter.entity(i);
							auto initiator = action.target<opack::By>();
							std::cout << initiator.doc_name() << " is contesting order to treat " << initiator.get_mut<Order>()->patient.doc_name() << "\n";
							action.destruct();
						}
					}
				);

			world.system("FinishCondition")
				.kind(flecs::PostUpdate)
				.iter(
					[](flecs::iter& iter)
						{
							if (iter.world().count<UnWell>() == 0)
							{
								auto time = iter.world().time();
								auto tick = iter.world().tick();
								std::cout << "---------- [ STOP ] ----------\n";
								std::cout << "All patients treated.\n";
								std::cout << "--- stats :\n";
								std::cout << " Simulation time (seconds): " << time << "\n";
								std::cout << "          Number of ticks : " << tick << "\n";
								std::cout << "Average tick duration (ms): " << (time/tick)*100 << "\n";
								iter.world().quit();
							}
						}
				).child_of<opack::world::Dynamics>();
		}

		// --- Activity
		{
			opack::reg_n<Examine, Treat, Ignore>(world);
			opack::reg<Comply>(world).add<Communicative>();
			opack::reg<Contest>(world).add<Communicative>();
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
		opack::FlowBuilder<Flow>(world).Not<WaitOrder>().build();
		{
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
							auto activity = agent.target(subject);
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

		opack::FlowBuilder<CommunicationFlow>(world).has<WaitOrder>().has<Order>().build();
		{
			opack::operation<CommunicationFlow, SuitableAnswers, AnswerSelection, Communicate>(world);

			opack::default_impact<SuitableAnswers>(world,
				[](flecs::entity agent, auto& inputs)
				{				
					SuitableAnswers::iterator(inputs) = opack::action<Contest>(agent);
					SuitableAnswers::iterator(inputs) = opack::action<Comply>(agent);
					SuitableAnswers::iterator(inputs) = opack::action<Ignore>(agent);

					return opack::make_outputs();
				}
			);

			opack::default_impact<AnswerSelection>(world,
				[](flecs::entity agent, auto& inputs)
				{				
					const auto id	= AnswerSelection::get_influencer(inputs);
					auto& actions	= AnswerSelection::get_choices(inputs);
					auto& graph		= AnswerSelection::get_graph(inputs);
					for (auto& a : actions)
					{
						graph.entry(a);
					}

					return opack::make_outputs();
				}
			);

			opack::default_impact<Communicate>(world,
				[](flecs::entity agent, auto& inputs)
				{
					auto action = std::get<opack::df<AnswerSelection, typename AnswerSelection::output>&>(inputs).value;
					if (action)
					{
						opack::act<Voice>(agent, action);
					}
					agent.mut(agent).remove<WaitOrder>();
					return opack::make_outputs();
				}
			);

		}

		// --- Cognitive models controls
		bool active_consistent{ true };
		bool active_friendship{ true };

		// --- Cognitive models definition
		{
			opack::behaviour<Behaviour_Consistent>(world, [active_consistent](flecs::entity agent) {return active_consistent; });

			// Impacts
			{
				opack::impact<ActionSelection, Behaviour_Consistent>(world,
					[](flecs::entity agent, auto& inputs)
					{
						const auto id = ActionSelection::get_influencer(inputs);
						auto& actions = ActionSelection::get_choices(inputs);
						auto& graph = ActionSelection::get_graph(inputs);
						for (auto& a : actions)
						{
							auto patient = a.target<opack::On>();
							auto procedure = agent.target(patient);
							if (adl::in_progress(procedure))
							{
								graph.positive_influence(id, a);
							}
						}
						return opack::make_outputs();
					}
				);
			}

			opack::behaviour<Behaviour_Friendship>(world, [active_friendship](flecs::entity agent) {return active_friendship; });
			// Impacts
			{
				opack::impact<ActionSelection, Behaviour_Friendship>(world,
					[](flecs::entity agent, auto& inputs)
					{
						const auto id = ActionSelection::get_influencer(inputs);
						auto& actions = ActionSelection::get_choices(inputs);
						auto& graph = ActionSelection::get_graph(inputs);
						for (auto& a : actions)
						{
							auto patient = a.target<opack::On>();
							if (agent.has<is_friend>(patient))
							{
								graph.positive_influence(id, a);
							}
						}
						return opack::make_outputs();
					}
				);
			}

			opack::behaviour<Behaviour_Communicative>(world, [](flecs::entity agent) {return  true; });
			// Impacts
			{
				opack::impact<AnswerSelection, Behaviour_Communicative>(world,
					[](flecs::entity agent, auto& inputs)
					{				
						const auto id	= AnswerSelection::get_influencer(inputs);
						auto& actions	= AnswerSelection::get_choices(inputs);
						auto& graph		= AnswerSelection::get_graph(inputs);
						for (auto& a : actions)
						{
							if (a.has<Communicative>() && agent.has<Communicative>())
								graph.positive_influence(id, a);
							else if (!a.has<Communicative>() && agent.has<Communicative>())
								graph.negative_influence(id, a);
							else if (!a.has<Communicative>() && !agent.has<Communicative>())
								graph.positive_influence(id, a);
						}

						return opack::make_outputs();
					}
				);
			}

			opack::behaviour<Behaviour_ProActive>(world, [](flecs::entity agent) {return  true; });
			// Impacts
			{
				opack::impact<AnswerSelection, Behaviour_ProActive>(world,
					[](flecs::entity agent, auto& inputs)
					{				
						const auto id	= AnswerSelection::get_influencer(inputs);
						auto& actions	= AnswerSelection::get_choices(inputs);
						auto& graph		= AnswerSelection::get_graph(inputs);
						for (auto& a : actions)
						{
							if (a.has<Comply>() && agent.has<ProActive>())
								graph.negative_influence(id, a);
							else if (!a.has<Contest>() && agent.has<ProActive>())
								graph.positive_influence(id, a);
						}

						return opack::make_outputs();
					}
				);
				
				opack::impact<ActionSelection, Behaviour_ProActive>(world,
					[](flecs::entity agent, auto& inputs)
					{				
						const auto id	= ActionSelection::get_influencer(inputs);
						auto& actions	= ActionSelection::get_choices(inputs);
						auto& graph		= ActionSelection::get_graph(inputs);
						for (auto& a : actions)
						{
							if (a.has<FollowOrder>() && !agent.has<ProActive>())
								graph.positive_influence(id, a);
						}

						return opack::make_outputs();
					}
				);
			}
		}

		// --- World population
		{
			// Defines a "Patient" prefab
			opack::reg<Patient>(world)
				.override<UnWell>()
				.override<FC>()
				.override<FCBase>()
				;

			// Defines a "Nurse" prefab
			opack::reg<Nurse>(world)
				.add<Flow>()				
				.add<CommunicationFlow>()	
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
			nurse.set<Order>({ patient_2 });
			nurse.add<WaitOrder>();
		}

		std::cout << "---------- [ START ] ----------\n";
	}
};