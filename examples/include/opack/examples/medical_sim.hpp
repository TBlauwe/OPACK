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
	struct On {}; // Relation

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
			// Defines a "Patient" prefab
			opack::reg<Patient>(world)
				.override<FC>()
				.override<FCBase>()
				;

			// Defines a "Nurse" prefab
			opack::reg<Nurse>(world)
				.add<Flow>()
				.add<FC>()
				.override<Qualification>()
				;

			opack::reg<Hearing>(world);
			opack::perceive<Hearing, IsCoughing>(world);

			opack::reg<Vision>(world);
			opack::perceive<Vision, Patient, FC>(world);
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
									action.add<On>(subject);
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
						std::cout << agent.doc_name() << " is doing " << action.doc_name() << " on " << action.get_object<On>().doc_name() << "\n";
						action.add<adl::Satisfied>();
					}
					return opack::make_outputs<Act>();
				}
				);
		}

		// --- World population
		{
			opack::artefact<Patient>(world, "Patient 1");
			opack::artefact<Patient>(world, "Patient 2");
			opack::artefact<Patient>(world, "Patient 3");
			opack::artefact<Patient>(world, "Patient 4");
			opack::artefact<Patient>(world, "Patient 5");
			opack::artefact<Patient>(world, "Patient 6");

			opack::agent<Nurse>(world, "Nurse 1");
			opack::agent<Nurse>(world, "Nurse 2");
		}
	}
};