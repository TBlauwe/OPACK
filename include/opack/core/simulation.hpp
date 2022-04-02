#pragma once

#include <flecs.h>

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
		/**
		@brief Construct an empty simulation.
		*/
		Simulation();

	private:
		flecs::world world;
	};

} // namespace opack
