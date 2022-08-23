/*********************************************************************
 * \file   simulation.hpp
 * \brief
 *
 * \author Tristan
 * \date   April 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

#include <opack/core/types.hpp>

namespace opack {
	// Simulation control
	// ==================
	/**
	@brief Advance simulation by one-step and specify elapsed time.
	@param delta_time How much time elapsed between two cycles. If 0 (default), then it is automatically measured;
	@return False, if application should stop.
	*/
	bool step(flecs::world& world, float delta_time = 0.0f);

	/**
	@brief Advance simulation by @c n step and specify elapsed time between each step.
	@param n number of steps
	@param delta_time How much time elapsed between two cycles. If 0 (default), then it is automatically measured;
	*/
	void step_n(flecs::world& world, size_t n, float delta_time = 0.0f);

	/**
	@brief Stop simulation after current cycle
	*/
	void stop(flecs::world& world);

	// Simulation stats
	// ================
	/**
	@brief Count number of entities matching the pattern.
	*/
	template<typename T>
	inline size_t count(flecs::world& world)
	{
		return static_cast<size_t>(world.count<T>());
	}

	/**
	@brief Count number of entities matching the pattern.
	*/
	template<typename T>
	size_t count(flecs::world& world, const flecs::entity_t obj)
	{
		return static_cast<size_t>(world.count<T>(obj));
	}

	/**
	@brief Count number of entities matching the pattern.
	*/
	size_t count(flecs::world& world, flecs::entity_t rel, flecs::entity_t obj);

	struct Simulation
	{
		Simulation(int argc = 0, char* argv[] = nullptr);

		inline operator flecs::world& () { return world; }


		// Configuration
		// -------------
		/**
		@brief Return target fps.
		*/
		float target_fps() const;

		/**
		@brief Set target fps.
		*/
		void target_fps(float value);

		/**
		@brief Return time scale.
		*/
		float time_scale() const;

		/**
		@brief Set time scale.
		*/
		void time_scale(float value);

		// Status
		// ------
		/**
		@brief Return number of elapsed ticks.
		*/
		int32_t tick();

		/**
		@brief Return last reported delta time (elapsed time between two cycles). Affected by time scale.
		*/
		float delta_time();

		/**
		@brief Return total elapsed simulation time.
		*/
		float time();

		/**
		 * Import a module @c T and return corresponding entity.
		 */
		template<typename T>
		inline flecs::entity import()
		{
			return world.import<T>();
		}


		// Controls
		// --------
		/**
		@brief Advance simulation by one-step and specify elapsed time.
		@param elapsed_time time elapsed. If 0 (default), then it is automatically measured. Affected by time scale.
		@return False, if application should stop.
		*/
		bool step(float elapsed_time = 0.0f);

		/**
		@brief Advance simulation by @c n step and specify elapsed time between each step.
		@param n number of steps
		@param elapsed_time time elapsed. If 0 (default), then it is automatically measured. Affected by time scale. 
		*/
		void step_n(size_t n, float elapsed_time = 0.0f);

		/**
		 * Run the simulation with rest enable,
		 * so that you can inspect underlying world from a web application.
		 * See: https://www.flecs.dev/explorer/?remote=true.
		 */
		void run_with_webapp();

		/**
		 * Run the simulation.
		 */
		void run();

		// Internal
		// --------
		flecs::world world;
	};

	namespace internal
	{
		flecs::world world(int argc = 0, char* argv[] = nullptr);
	}
} // namespace opack
