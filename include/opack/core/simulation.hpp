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

#define OPACK_ACTION_TYPE(name) struct name : opack::Action {}
#define OPACK_AGENT_TYPE(name) struct name : opack::Agent {}
#define OPACK_ARTEFACT_TYPE(name) struct name : opack::Artefact {}
#define OPACK_ACTUATOR(name) struct name : opack::Actuator {}
#define OPACK_BEHAVIOUR(name) struct name : opack::Behaviour {}
#define OPACK_FLOW(name) struct name : opack::Flow {}
#define OPACK_SENSE(name) struct name : opack::Sense {}

 /**
 @brief Main entry point to use the library.
 */
namespace opack {

	/**
	Create a new agent prefab. You may create agent from this type now.
	*/
	template<std::derived_from<Agent> T, std::derived_from<Agent> TDerived = Agent>
	inline flecs::entity register_agent(flecs::world& world)
	{
		if (opack::internal::has_prefab<T>(world))
			return opack::prefab<T>(world);
		else
		{
			world.component<T>().template child_of<opack::concepts>();
			return opack::internal::add_prefab<T>(world)
				.is_a(prefab<TDerived>(world))
				.template child_of<world::prefab::Agents>();
		}
	}

	/**
	Create a new artefact prefab. You may create artefact from this type now.
	*/
	template<std::derived_from<Artefact> T, std::derived_from<Artefact> TDerived = Artefact>
	inline flecs::entity register_artefact(flecs::world& world)
	{
		if (opack::internal::has_prefab<T>(world))
			return opack::prefab<T>(world);
		else
		{
			world.component<T>().template child_of<opack::concepts>();
			return opack::internal::add_prefab<T>(world)
				.is_a(prefab<TDerived>(world))
				.template child_of<world::prefab::Artefacts>();
		}
	}

	/**
	Create a new action prefab that you may instantiate.
	*/
	template<std::derived_from<Action> T, std::derived_from<Action> TDerived = Action>
	inline flecs::entity register_action(flecs::world& world)
	{
		if (opack::internal::has_prefab<T>(world))
			return opack::prefab<T>(world);
		else
		{
			world.component<T>().template child_of<opack::concepts>();
			return opack::internal::add_prefab<T>(world)
				.is_a(prefab<TDerived>(world))
				.template child_of<world::prefab::Actions>();
		}
	}

	/**
	Create a new sense. You may now use the type to perceive components of others entities,
	by adding a relation YourSense(Observer, Subject)
	*/
	template<std::derived_from<Sense> T>
	inline flecs::entity register_sense(flecs::world& world)
	{
		return world.component<T>()
			.template is_a<Sense>()
			.template child_of<world::Senses>();
	}

	/**
	Create a new actuator. You may now use the type to operate actions.
	*/
	template<std::derived_from<Actuator> T>
	inline flecs::entity register_actuator(flecs::world& world)
	{
		return world.component<T>()
			.add(flecs::Exclusive)
			.template is_a<Actuator>()
			.template child_of<world::Actuators>();
	}

	template<typename T>
	bool is_a(flecs::entity entity)
	{
		auto world = entity.world();
		return entity.has(flecs::IsA, prefab<T>(world));
	}

	/**
	Automatically register type based on its inheritance.
	*/
	template<typename T, typename TDerived = T>
	inline flecs::entity reg(flecs::world& world)
	{
		if constexpr (std::derived_from<T, Agent>)
		{
			if constexpr (std::same_as<T, TDerived>)
				return register_agent<T>(world);
			else
				return register_agent<T, TDerived>(world);
		}
		else if constexpr (std::derived_from<T, Artefact>)
		{
			if constexpr (std::same_as<T, TDerived>)
				return register_artefact<T>(world);
			else
				return register_artefact<T, TDerived>(world);
		}
		else if constexpr (std::derived_from<T, Action>)
		{
			if constexpr (std::same_as<T, TDerived>)
				return register_action<T>(world);
			else
				return register_action<T, TDerived>(world);
		}
		else if constexpr (std::derived_from<T, Sense>)
			return register_sense<T>(world);
		else if constexpr (std::derived_from<T, Actuator>)
			return register_actuator<T>(world);
		return flecs::entity{};
	}

	/**
	If you need to quickly register multiple types without modifying them and without derivation (use @c opack::reg).
	You can always modify them afterwards with @c opack::entity<T>.
	*/
	template<typename... Ts>
	inline void reg_n(flecs::world& world)
	{
		(reg<Ts>(world), ...);
	}

	// Simulation interaction
	// ======================
	/**
	@brief Instantiate an entity from prefab @c U of category @c T.
	*/
	template<typename T>
	inline flecs::entity instantiate(flecs::world& world)
	{
		return world.entity().is_a(prefab<T>(world));
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
		auto e = world.entity().is_a(prefab<T>(world));
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
		return instantiate<T>(world, name).template child_of<world::Agents>().template add<DefaultBehaviour>();
	}

	/**
	@brief Instantiate an agent from prefab @c T.
	*/
	template<std::derived_from<Agent> T = opack::Agent>
	inline flecs::entity agent(flecs::world& world)
	{
		return instantiate<T>(world).template child_of<world::Agents>().template add<DefaultBehaviour>();
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

	template<typename T>
	inline const T& get(flecs::entity entity)
	{
		return *entity.get<T>();
	}

	template<typename T>
	inline T& get_mut(flecs::entity entity)
	{
		return *entity.get_mut<T>();
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
