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
#include <opack/module/core.hpp>
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
	inline flecs::entity register_agent(flecs::world& world)
	{
		return internal::register_t_as_u<T, Agent>(world).template child_of<world::prefab::Agents>();
	}

	/**
	Create a new artefact prefab. You may create artefact from this type now.
	*/
	template<std::derived_from<Artefact> T>
	inline flecs::entity register_artefact(flecs::world& world)
	{
		return internal::register_t_as_u<T, Artefact>(world).template child_of<world::prefab::Artefacts>();
	}

	/**
	Create a new sense. You may now use the type to perceive components of others entities,
	by adding a relation YourSense(Observer, Subject)
	*/
	template<std::derived_from<Sense> T>
	inline flecs::entity register_sense(flecs::world& world)
	{
		return internal::register_t_as_u<T, Sense>(world).template child_of<world::Senses>();
	}

	/**
	Create a new actuator. You may now use the type to operate actions.
	*/
	template<std::derived_from<Actuator> T>
	inline flecs::entity register_actuator(flecs::world& world)
	{
		return internal::register_t_as_u<T, Actuator>(world).template child_of<world::Actuators>();
	}

	/**
	Create a new action prefab that you may instantiate.
	*/
	template<std::derived_from<Action> T>
	inline flecs::entity register_action(flecs::world& world)
	{
		return internal::register_t_as_u<T, Action>(world).template child_of<world::prefab::Actions>();
	}

	// Simulation interaction
	// ======================
	/**
	@brief Instantiate an entity from from prefab @c T.
	*/
	template<typename T>
	inline flecs::entity instantiate(flecs::world& world)
	{
		return world.entity().is_a<T>();
	}

	/**
	Set a name for entity @c e - used for debugging purposes.
	You may want to add a specific component for more complex name.
	*/
	void name(flecs::entity e, const char* name);

	/**
	@brief Instantiate an entity named @c name (for debugging/visualization) purposes, from prefab @c T.
	*/
	template<typename T>
	inline flecs::entity instantiate(flecs::world& world, const char* name)
	{
		auto e = world.entity().is_a<T>();
		opack::name(e, name);
		return e;
	}
	/**
	@brief Instantiate an agent named @c name (for debugging/visualization) purposes, from prefab @c T.

	BUG rework prefab without using component (with a type so)
	*/
	template<std::derived_from<Agent> T = opack::Agent>
	inline flecs::entity agent(flecs::world& world, const char* name)
	{
		return instantiate<T>(world, name).template child_of<world::Agents>().template add<Active, Behaviour>();
	}

	/**
	@brief Instantiate an agent from prefab @c T.
	*/
	template<std::derived_from<Agent> T = opack::Agent>
	inline flecs::entity agent(flecs::world& world)
	{
		return instantiate<T>(world).template child_of<world::Agents>().template add<Active, Behaviour>();
	}

	/**
	@brief Instantiate an artefact named @c name (for debugging/visualization) purposes, from prefab @c T.
	*/
	template<std::derived_from<Artefact> T = opack::Artefact>
	inline flecs::entity artefact(flecs::world& world, const char* name)
	{
		return instantiate<T>(world, name).template child_of<world::Artefacts>();
	}


	/**
	@brief Instantiate a artefact from prefab @c T.
	*/
	template<std::derived_from<Artefact> T = opack::Artefact>
	inline flecs::entity artefact(flecs::world& world)
	{
		return instantiate<T>(world).template child_of<world::Artefacts>();
	}

	/**
	Returns id of type @c.
	*/
	template<typename T>
	inline flecs::id id(const flecs::world& world)
	{
		return world.id<T>();
	}

	/**
	Returns entity of type @c.
	*/
	template<typename T>
	inline flecs::entity entity(const flecs::world& world)
	{
		return world.entity<T>();
	}


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
