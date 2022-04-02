#pragma once

#include <flecs.h>
#include <taskflow/core/executor.hpp>

/**
@brief Main entry point to use the library.
*/
namespace opack {

	/**
	@class Simulation

	@brief Class used to create and manipulate a simulation of agents evolving
	within a virtual world.
	*/
	class Simulation {
	public:
		Simulation();
		~Simulation();

	private:
		tf::Executor executor;
		flecs::world world;
	};

} // namespace opack
