#pragma once

#include <iostream>
#include <opack/core.hpp>

struct MedicalSim : opack::Simulation
{
	struct Patient : opack::Artefact {};
	struct Nurse : opack::Agent {};

	struct Base {};

	struct Qualification { size_t value{ 0 }; };

	struct FC { size_t value { 70 }; };
	struct FCBase { size_t value { 70 }; };

	MedicalSim(int argc = 0, char * argv[] = nullptr) : opack::Simulation{argc, argv}
	{
		world.component<FC>()
			.member<size_t>("value")
			;

		world.component<FCBase>()
			.member<size_t>("value")
			;

		opack::reg<Patient>(world);
		opack::reg<Nurse>(world)
			.override<Qualification>()
			;

		world.system<const FCBase, FC>()
			.iter(
				[](flecs::iter& it, const FCBase* fcb, FC* fc)
				{
					for (auto i : it)
					{
						fc[i].value = fcb[i].value * sin(it.world().time());
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