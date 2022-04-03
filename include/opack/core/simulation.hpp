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

		// Simulation control
		// ==================
		/**
        @brief Advance simulation by one-step and specify elapsed time. 
        @param elapsed_time time elapsed. If 0 (default), then it is automatically measured;
		@return False, if application should stop.
        */
        bool step(float elapsed_time = 0.0f);

        /**
        @brief Advance simulation by @c n step and specify elapsed time between each step.
        @param n number of steps
        @param elapsed_time time elapsed. If 0 (default), then it is automatically measured;
        */
        void step_n(size_t n, float elapsed_time = 0.0f);

        /**
        @brief Stop the simulation. Ensure all threads are finished. Some may be still running even if you have not called "step" functions for some time.
        Automatically called by simulation destructor.
        */
        void stop();

        /**
        @brief Return target fps.
        */
        float target_fps();

        /**
        @brief Set target fps.
        */
        void target_fps(float value);

        /**
        @brief Return time scale.
        */
        float time_scale();

        /**
        @brief Set time scale.
        */
        void time_scale(float value);

		// Simulation stats
		// ================
        /**
        @brief Current number of steps simulated.
        */
        size_t tick() const;

        /**
        @brief Last delta time.
        */
        float delta_time() const;

        /**
        @brief Get simulation time.
        */
        float time() const;

	private:
		tf::Executor executor;
		flecs::world world;
	};

} // namespace opack
