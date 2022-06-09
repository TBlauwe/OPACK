#pragma once

#include <opack/core.hpp>
#include <opack/strategy/influence_graph.hpp>

struct MedicalSim : opack::Simulation
{
	struct Patient : opack::Artefact {};
	struct Nurse : opack::Agent {};

	struct Base {};

	struct Qualification { size_t value{ 0 }; };

	struct FC { size_t value{ 70 }; };
	struct FCBase { size_t value { 70 }; float var{ 5 };};

	struct Flow : opack::Flow{};
	struct SuitableActions : opack::O<opack::Inputs<>, opack::Outputs<std::vector<flecs::entity>>> {};

	MedicalSim(int argc = 0, char * argv[] = nullptr) : opack::Simulation{argc, argv}
	{
		world.component<FC>()
			.member<size_t>("value")
			;

		world.component<FCBase>()
			.member<size_t>("value")
			;

		opack::reg<Patient>(world)
			.override<FC>()
			.override<FCBase>()
			;

		opack::reg<Nurse>(world)
			.add<Flow>()
			.override<Qualification>()
			;

		opack::flow<Flow>(world);
		opack::operation<Flow, SuitableActions>::make<opack::strat::influence_graph>(world);
		opack::impact<SuitableActions>::make<>(world,
			[](flecs::entity agent)
			{
				std::cout << "Hello\n";
				return opack::make_output<SuitableActions>();
			}
			);

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

		// Population
		opack::artefact<Patient>(world, "Patient 1");
		opack::artefact<Patient>(world, "Patient 2");
		opack::artefact<Patient>(world, "Patient 3");
		opack::artefact<Patient>(world, "Patient 4");
		opack::artefact<Patient>(world, "Patient 5");
		opack::artefact<Patient>(world, "Patient 6");

		opack::agent<Nurse>(world, "Nurse 1");
		opack::agent<Nurse>(world, "Nurse 2");
	}
};