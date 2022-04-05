#pragma once

#include <concepts>

#include <flecs.h>
#include <taskflow/core/executor.hpp>

/**
@brief Main entry point to use the library.
*/
namespace opack {

	// Types
	struct Agent {};
	struct Artefact {};
	struct Percept {};

	// Relation
	struct source {};

	/**
	@class Simulation

	@brief Class used to create and manipulate a simulation of agents evolving
	within a virtual world.
	*/
	class Simulation {
	public:
		Simulation();
		~Simulation();

		// Simulation register 
		// ===================
		/** Shorthand to say that T is an agent.
		*/
		template<std::derived_from<Agent> T>
		inline void register_agent_type()
		{
			register_t_as<T, Agent>();
		}

		/** Shorthand to say that T is a artefact.
		*/
		template<std::derived_from<Artefact> T>
		inline void register_artefact_type()
		{
			register_t_as<T, Artefact>();
		}

		/** Shorthand to say that T is a percept.
		*/
		template<std::derived_from<Percept> T>
		inline void register_percept_type()
		{
			register_t_as<T, Percept>();
		}

		// Simulation interaction
		// ======================
		/**
		@brief Add an agent.
		*/
		template<std::derived_from<Agent> T = opack::Agent>
		inline flecs::entity agent(const char * name = "")
		{
			return world.entity(name).add<T>();
		}

		/**
		@brief Add an artefact.
		*/
		template<std::derived_from<Artefact> T = opack::Artefact>
		inline flecs::entity artefact(const char * name = "")
		{
			return world.entity(name).add<T>();
		}

		/**
		@brief Add a percept for the agent. You are responsible for deleting it, if it is perceived anymore.
		@param agent Which agent perceives this
		*/
		template<std::derived_from<Percept> T = opack::Percept, typename ... Args>
		inline flecs::entity percept(flecs::entity agent, flecs::entity object)
		{
			auto p = world.entity().add<T>();
			p.child_of(agent);
			p.template add<source>(object);
			(p.template add<Args>, ...);
			agent.add<T>(p);
			return p;
		}

		// Simulation queries
		// ==================

		/**
		Retrieve percepts for a specific agent.
		@return A vector of all percepts percevied by the agent.
		*/
		std::vector<flecs::entity> query_perceptions_of(flecs::entity agent);


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
		@brief Count number of entities matching the pattern.
		*/
		template<typename T>
		inline size_t count() const
		{
			return static_cast<size_t>(world.count<T>());
		}

		template<typename T>
		inline size_t count(flecs::entity obj) const
		{
			return static_cast<size_t>(world.count<T>(obj));
		}

		inline size_t count(flecs::entity rel, flecs::entity obj) const
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
		template<typename T, typename U>
		void register_t_as()
		{
			world.component<T>().template is_a<U>();
		}


	private:
		tf::Executor	executor;

		flecs::world	world;
		flecs::rule<>	rule_perceptions;
	};

} // namespace opack
