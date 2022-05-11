/*********************************************************************
 * \file   simulation.hpp
 * \brief
 *
 * \author Tristan
 * \date   April 2022
 *********************************************************************/
#pragma once

#include <concepts>

#include <flecs.h>

#include <opack/core/types.hpp>
#include <opack/utils/type_map.hpp>
#include <opack/utils/flecs_helper.hpp>

/**
@brief Main entry point to use the library.
*/
namespace opack {

	/**
	Create a new agent prefab. You may create agent from this type now.
	*/
	template<std::derived_from<Agent> T>
	inline flecs::entity register_agent_type(flecs::world& world)
	{
		return internal::register_t_as_u<T, Agent>(world);
	}

	/**
	Create a new artefact prefab. You may create artefact from this type now.
	*/
	template<std::derived_from<Artefact> T>
	inline flecs::entity register_artefact_type(flecs::world& world)
	{
		return internal::register_t_as_u<T, Artefact>(world);
	}

	/**
	Create a new sense. You may now use the type to perceive components of others entities,
	by adding a relation YourSense(Observer, Subject)
	*/
	template<std::derived_from<Sense> T>
	inline flecs::entity register_sense(flecs::world& world)
	{
		return internal::register_t_as_u<T, Sense>(world);
	}

	/**
	Create a new actuator. You may now use the type to operate actions.
	*/
	template<std::derived_from<Actuator> T>
	inline flecs::entity register_actuator_type(flecs::world& world)
	{
		return internal::register_t_as_u<T, Actuator>(world);
	}

	/**
	Create a new action prefab that you may instantiate.
	*/
	template<std::derived_from<Action> T>
	inline flecs::entity register_action(flecs::world& world)
	{

		return internal::register_t_as_u<T, Action>(world);
	}

	// Simulation interaction
	// ======================
	/**
	@brief Instantiate an agent named @c name, from prefab @c T.
	*/
	template<std::derived_from<Agent> T = opack::Agent>
	inline flecs::entity agent(flecs::world& world, const char* name = "")
	{
		return world.entity(name).is_a<T>();
	}

	/**
	@brief Add an artefact.
	*/
	template<std::derived_from<Artefact> T = opack::Artefact>
	inline flecs::entity artefact(flecs::world& world, const char* name = "")
	{
		return world.entity(name).is_a<T>();
	}

	/**
	Returns id of type @c.
	*/
	template<typename T>
	flecs::id id(const flecs::world& world)
	{
		return world.id<T>();
	}

	/**
	Returns entity of type @c.
	*/
	template<typename T>
	flecs::entity entity(const flecs::world& world)
	{
		return world.entity<T>();
	}

	/**
	@class Simulation

	@brief Class used to create and manipulate a simulation of agents evolving
	within a virtual world.
	*/
	class Simulation {
	public:
		Simulation(int argc = 0, char* argv[] = nullptr);
		~Simulation();

		//inline operator flecs::world() { return world.get_world(); }
		inline operator flecs::world& () { return world; }

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
		@brief Launch the simulation and also activates rest app. Never returns ! So it is essentially a breakpoint with no continue.
		*/
		void rest_app();

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


		// Simulation queries
		// ==================

		// Simulation stats
		// ================
		/**
		@brief Count number of entities matching the pattern.
		*/
		template<typename T>
		inline size_t count() const
		{
			return static_cast<size_t>(world.count<T>());
		}

		/**
		@brief Count number of entities matching the pattern.
		*/
		template<typename T>
		inline size_t count(const flecs::entity_t obj) const
		{
			return static_cast<size_t>(world.count<T>(obj));
		}

		/**
		@brief Count number of entities matching the pattern.
		*/
		inline size_t count(flecs::entity_t rel, flecs::entity_t obj) const
		{
			return static_cast<size_t>(world.count(rel, obj));
		}

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
	public:
		/**
		@brief Underlying flecs world (an ecs database). Use only if you now what you're doing.
		*/
		flecs::world	world;
	};

} // namespace opack
