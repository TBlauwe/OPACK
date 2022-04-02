#include <doctest/doctest.h>
#include <opack/core/simulation.hpp>

TEST_SUITE_BEGIN("Simulation");

TEST_CASE("Basics") 
{
	{ // Creation & Destruction
		opack::Simulation();
	}
}

TEST_SUITE_END();
