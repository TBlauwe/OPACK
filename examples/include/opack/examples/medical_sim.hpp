#pragma once

#include <opack/core.hpp>
#include <opack/operations/influence_graph.hpp>

struct MedicalSim : opack::Simulation
{
	// -------------------
	// --- Environment ---
	// -------------------
	struct Base {}; // Relation
	struct Near {}; // Relation

	struct Qualification { size_t value{ 0 }; };

	struct FC { size_t value{ 70 }; };
	struct FCBase { size_t value { 70 }; float var{ 5 };};

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

	// --- Caracteristic

	// --- Flow & Operation
	struct Flow : opack::Flow{};

	struct Perception : opack::operations::Union<opack::Percept> {};
	struct SuitableActions : opack::operations::Union<flecs::entity> {};
	struct ActionSelection : opack::operations::SelectionByIGraph<flecs::entity, SuitableActions, opack::df<Perception, typename Perception::output>> {};
	struct Act : opack::operations::All<opack::df<ActionSelection, typename ActionSelection::output>> {};
	struct UpdateStress : opack::operations::All<opack::df<Perception, typename Perception::output>> {};

	MedicalSim(int argc = 0, char * argv[] = nullptr) : opack::Simulation{argc, argv}
	{

		// --- Reflection for visualisation
		{
			world.component<Near>()
				.add(flecs::Exclusive)
				;

			world.component<Vision>()
				.add(flecs::Exclusive)
				;

			world.component<FC>()
				.member<size_t>("value")
				;

			world.component<FCBase>()
				.member<size_t>("value")
				;
		}
		
		// --- World dynamics
		{
			world.system<const FCBase, FC>()
				.iter(
					[](flecs::iter& it, const FCBase* fcb, FC* fc)
					{
						for (auto i : it)
						{
							fc[i].value = fcb[i].value + (fcb[i].var * sin(it.world().time()));
						}
					}
				);

			world.system<const Nurse>()
				.kind(flecs::PreUpdate)
				.term<Near>().obj(flecs::Wildcard)
				.iter(
					[](flecs::iter& iter, const Nurse* _)
					{
						auto id = iter.pair(2);
						auto obj = id.second();
						for (auto i : iter)
						{
							iter.entity(i).add<Vision>(obj);
						}
					}
				);
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

			opack::reg<Hearing>(world)
				;

			opack::reg<Vision>(world)
				;
		}

		// --- Flow definition
		{
			opack::flow<Flow>(world);
			opack::operation<Flow, SuitableActions>(world);
			//opack::operation<Flow, ActionSelection>::make<opack::strat::influence_graph>(world);

			opack::default_impact<SuitableActions>(world,
				[](flecs::entity agent, SuitableActions::inputs& inputs)
				{
					SuitableActions::iterator(inputs) = agent;
					return opack::make_outputs<SuitableActions>();
				}
				);

			//opack::impact<ActionSelection>::make<>(world,
			//	[](flecs::entity agent, const opack::df<SuitableActions, opack::Actions_t>&)
			//	{
			//		std::cout << "Hello\n";
			//		return opack::make_outputs<ActionSelection>();
			//	}
			//	);
		}

		// --- World population
		{
			opack::artefact<Patient>(world, "Patient 1");
			opack::artefact<Patient>(world, "Patient 2");
			opack::artefact<Patient>(world, "Patient 3");
			opack::artefact<Patient>(world, "Patient 4");
			opack::artefact<Patient>(world, "Patient 5");
			auto patient = opack::artefact<Patient>(world, "Patient 6");

			opack::agent<Nurse>(world, "Nurse 1").add<Near>(patient);
			opack::agent<Nurse>(world, "Nurse 2");

		}
	}
};