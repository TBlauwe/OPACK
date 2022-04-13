#pragma once

#include <concepts>

#include <flecs.h>
#include <taskflow/core/executor.hpp>

#include <opack/utils/type_map.hpp>

/**
@brief Main entry point to use the library.
*/
namespace opack {

	// Types
	struct Agent {};
	struct Artefact {};

	struct Actuator {};
	struct Action
	{
		struct Continuous {};
		struct Ponctual {};

		struct Arity { size_t value{ 1 }; };
	};

	struct Sense {};
	/**
	@brief A percept is a small class to tell what can be perceived for a given entity and sense.
	*/
	struct Percept 
	{
		enum class Type {Component, Relation};
		Type				type {Type::Component};
		flecs::entity_view	sense;
		flecs::entity_view	subject;
		flecs::entity_view	predicat;
		flecs::entity_view	object; // Only set if @c type is equal to @c Type::Component.

		Percept(flecs::entity_view _sense, flecs::entity_view _subject, flecs::entity_view _predicat) : 
			type{ Type::Component }, sense { _sense }, subject{ _subject }, predicat{_predicat}, object {}
		{}

		Percept(flecs::entity_view _sense, flecs::entity_view _subject, flecs::entity_view _predicat, flecs::entity_view _object) : 
			type{ Type::Relation }, sense { _sense }, subject{ _subject }, predicat{ _predicat}, object {_object}
		{}

		/**
		@brief Tells if percept is using given sense.
		*/
		template<std::derived_from<Sense> T = Sense>
		inline bool use() { return sense == sense.world().id<T>(); }

		template<typename T>
		inline bool is_pred() { return predicat == predicat.world().id<T>(); }

		template<typename T>
		inline bool is_pred(flecs::entity object) 
		{ 
			auto world = subject.world();
			return world.pair<T>(object) == world.pair(component, object);
		}

		/**
		@brief Retrieve the current value from the perceived entity. 
		Pointer stability is not guaranteed. Copy value if you need to keep it.
		Does not check if @c T is indeed accessible from this sense and if @c target do have it.
		Use @c is() if you wish to check this beforehand.
		*/
		template<typename T>
		inline const T * fetch() { return subject.get<T>(); }
	};
	using Percepts = std::vector<Percept>;

	/**
	@class Simulation

	@brief Class used to create and manipulate a simulation of agents evolving
	within a virtual world.
	*/
	class Simulation {
	public:
		Simulation(int argc = 0, char * argv[] = nullptr);
		~Simulation();

		// Simulation register 
		// ===================
		/** Shorthand to say that T is an agent.
		*/
		template<std::derived_from<Agent> T>
		inline flecs::entity register_agent_type()
		{
			return register_t_as<T, Agent>();
		}

		/** Shorthand to say that T is a artefact.
		*/
		template<std::derived_from<Artefact> T>
		inline flecs::entity register_artefact_type()
		{
			return register_t_as<T, Artefact>();
		}

		/** Shorthand to say that T is a sense.
		*/
		template<std::derived_from<Sense> T>
		inline flecs::entity register_sense()
		{
			return register_t_as<T, Sense>();
		}

		/** Shorthand to say that T is an actuator.
		*/
		template<std::derived_from<Actuator> T>
		inline flecs::entity register_actuator_type()
		{
			return register_t_as<T, Actuator>();
		}

		/** Shorthand to say that T is an action.
		*/
		template<std::derived_from<Action> T>
		inline flecs::entity register_action()
		{

			return register_t_as<T, Action>();
		}

		/**
		@brief @c T sense is now able to perceive @c U component.
		@param agent Which agent perceives this
		@return entity of @c U component;
		*/
		template<std::derived_from<Sense> T = Sense, typename U>
		inline flecs::entity perceive()
		{
			return world.component<T>().template add<Sense, U>();
		}

		// Simulation interaction
		// ======================
		/**
		@brief Add an agent.
		*/
		template<std::derived_from<Agent> T = opack::Agent>
		inline flecs::entity agent(const char * name = "")
		{
			return world.entity(name).is_a<T>();
		}

		/**
		@brief Add an artefact.
		*/
		template<std::derived_from<Artefact> T = opack::Artefact>
		inline flecs::entity artefact(const char * name = "")
		{
			return world.entity(name).is_a<T>();
		}

		/**
		@brief @c observer is now able to perceive @c subject through @c T sense.
		*/
		template<std::derived_from<Sense> ... T>
		inline void perceive(flecs::entity observer, flecs::entity subject)
		{
			(observer.add<T>(subject), ...);
		}

		/**
		@brief Create an action of type @c T. Compose the action as required, before having entites acting on it.
		*/
		template<std::derived_from<Action> T>
		inline flecs::entity action()
		{
			return world.entity().template is_a<T>();
		}

		/**
		@brief @c initiator is now acting with actuator @c to accomplish given @c action.
		*/
		template<std::derived_from<Actuator> T>
		inline void act(flecs::entity initiator, flecs::entity action)
		{
			initiator.add(entity<T>(), action);
		}

		/**
		@brief @c source is now not able to perceive @c target through @c T sense.
		*/
		template<std::derived_from<Sense> ...T>
		inline void conceal(flecs::entity source, flecs::entity target)
		{
			(source.remove<T>(target), ...);
		}

		// Simulation queries
		// ==================

		/**
		Retrieve percepts for a specific agent.

		TODO With default type @c opack::Sense no perception are retrieved since there is no percepts retrievable with this Sense.
		Maybe specialize function to return all percepts ?

		@return A vector of all percepts of this type, perceived by the agent.
		*/
		template<std::derived_from<Sense> T = opack::Sense>
		Percepts query_percepts(flecs::entity source)
		{
			Percepts percepts{};
			{
				auto observer_var = rule_components_perception.find_var("Observer");
				auto sense_var = rule_components_perception.find_var("Sense");
				auto subject_var = rule_components_perception.find_var("Subject");
				auto predicat_var = rule_components_perception.find_var("Predicat");

				rule_components_perception.iter()
					.set_var(observer_var, source)
					.set_var(sense_var, world.id<T>())
					.iter(
						[&](flecs::iter& it)
						{
							percepts.push_back(Percept{ it.get_var(sense_var), it.get_var(subject_var), it.get_var(predicat_var) });
						}
					)
				;
			}
			{
				auto observer_var = rule_relations_perception.find_var("Observer");
				auto sense_var	= rule_relations_perception.find_var("Sense");
				auto subject_var = rule_relations_perception.find_var("Subject");
				auto predicat_var= rule_relations_perception.find_var("Predicat");
				auto object_var	= rule_relations_perception.find_var("Object");

				rule_relations_perception.iter()
					.set_var(observer_var, source)
					.set_var(sense_var, world.id<T>())
					.iter(
						[&](flecs::iter& it)
						{
							percepts.push_back(Percept{ it.get_var(sense_var), it.get_var(subject_var), it.get_var(predicat_var), it.get_var(object_var)});
						}
					)
				;
			}

			return percepts;
		}

		template<typename T>
		flecs::id id() const
		{
			return world.id<T>();
		}

		template<typename T>
		flecs::entity entity() const
		{
			return world.entity<T>();
		}

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
		inline size_t count(const flecs::entity_t obj) const
		{
			return static_cast<size_t>(world.count<T>(obj));
		}

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
		template<typename T, typename U>
		flecs::entity register_t_as()
		{
			return world.prefab<T>().template is_a<U>().template add<T>();
		}

	public:
		/**
		@brief Underlying flecs world (an ecs database). Use only if you now what you're doing.
		*/
		flecs::world	world;

	private:
		tf::Executor	executor;

		flecs::rule<>	rule_components_perception;
		flecs::rule<>	rule_relations_perception;
	};

} // namespace opack
	
std::ostream& operator<<(std::ostream& os, const opack::Percept& p);
