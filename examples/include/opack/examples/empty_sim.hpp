#pragma once

#include <opack/core.hpp>
#include <iostream>

struct EmptySim : opack::Simulation
{
	EmptySim(int argc = 0, char * argv[] = nullptr) : opack::Simulation{argc, argv}
	{
	}
};