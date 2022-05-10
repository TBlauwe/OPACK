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
#include <taskflow/core/executor.hpp>

#include <opack/core/types.hpp>
#include <opack/core/perception.hpp>
#include <opack/utils/type_map.hpp>
#include <opack/utils/type_name.hpp>

/**
@brief Main entry point to use the library.
*/
namespace opack {

	// Simulation register 
	// ===================
	template<typename T, typename U>
	flecs::entity register_t_as_u(flecs::world& world)
	{
		return world.template prefab<T>().template is_a<U>().template add<T>();
	}

	/**
	Create a new agent prefab. You may create agent from this type now.
	*/
	template<std::derived_from<Agent> T>
	inline flecs::entity register_agent_type(flecs::world& world)
	{
		return register_t_as_u<T, Agent>(world);
	}

	/**
	Create a new artefact prefab. You may create artefact from this type now.
	*/
	template<std::derived_from<Artefact> T>
	inline flecs::entity register_artefact_type(flecs::world& world)
	{
		return register_t_as_u<T, Artefact>(world);
	}

	/**
	Create a new sense. You may now use the type to perceive components of others entities,
	by adding a relation YourSense(Observer, Subject)
	*/
	template<std::derived_from<Sense> T>
	inline flecs::entity register_sense(flecs::world& world)
	{
		return register_t_as_u<T, Sense>(world);
	}

	/**
	Create a new actuator. You may now use the type to operate actions.
	*/
	template<std::derived_from<Actuator> T>
	inline flecs::entity register_actuator_type(flecs::world& world)
	{
		return register_t_as_u<T, Actuator>(world);
	}

	/**
	Create a new action prefab that you may instantiate.
	*/
	template<std::derived_from<Action> T>
	inline flecs::entity register_action(flecs::world& world)
	{

		return register_t_as_u<T, Action>(world);
	}

	/**
	@brief @c T sense is now able to perceive @c U component.
	@param agent Which agent perceives this
	@return entity of @c U component;
	*/
	template<std::derived_from<Sense> T = Sense, typename U>
	inline flecs::entity perceive(flecs::world& world)
	{
		return world.component<T>().template add<Sense, U>();
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
	@brief @c observer is now able to perceive @c subject through @c T sense.
	*/
	template<std::derived_from<Sense> ... T>
	inline void perceive(flecs::world& world, flecs::entity observer, flecs::entity subject)
	{
		(observer.add<T>(subject), ...);
	}

	/**
	@brief Create an action of type @c T. Compose the action as required, before having entites acting on it.
	*/
	template<std::derived_from<Action> T>
	inline flecs::entity action(flecs::world& world)
	{
		return world.entity().template is_a<T>().set_doc_name(type_name_cstr<T>());
	}

	/**
	@brief @c initiator is now acting with actuator @c to accomplish given @c action.
	*/
	template<std::derived_from<Actuator> T>
	inline void act(flecs::world& world, flecs::entity initiator, flecs::entity action)
	{
		//size_t count{ 0 };
		//flecs::entity last;
		//action.each<Initiator>([&count, &last](flecs::entity object) {count++; last = object; });

		//if (count >= action.get<Arity>()->value)
		//{
		//	//TODO Should issue warning - Here we replace the last initiator.
		//	//There will be a bug since we do not remove the relation from the initiator to the action.
		//	action.remove<Initiator>(last);
		//}

		// Action without initiator are cleaned up, so we need to remove relation from previous action.
		auto last_action = initiator.get_object<T>();
		if (last_action)
			last_action.mut(world).template remove<Initiator>(initiator);

		action.add<Initiator>(initiator);
		initiator.add<T>(action);
	}

	/**
	@brief @c source is now not able to perceive @c target through @c T sense.
	*/
	template<std::derived_from<Sense> ...T>
	inline void conceal(flecs::world& world, flecs::entity source, flecs::entity target)
	{
		(source.remove<T>(target), ...);
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

	private:
		tf::Executor	executor;
	};

} // namespace opack

std::ostream& operator<<(std::ostream& os, const opack::Percept& p);
