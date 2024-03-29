/*****************************************************************//**
 * \file   action.hpp
 * \brief  Action API.
 *
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <functional>

#include <flecs.h>

#include <opack/core/api_types.hpp>
#include <opack/core/components.hpp>

 /**
 @brief Shorthand for OPACK_SUB_PREFAB(name, opack::Action)
 */
#define OPACK_ACTION(name) OPACK_SUB_PREFAB(name, opack::Action)

 /**
 @brief Identical to OPACK_SUB_PREFAB(name, base)
 */
#define OPACK_SUB_ACTION(name, base) OPACK_SUB_PREFAB(name, base)

 /**
 @brief Shorthand for OPACK_SUB_PREFAB(name, opack::Actuator)
 */
#define OPACK_ACTUATOR(name) OPACK_SUB_PREFAB(name, opack::Actuator)

 /**
 @brief Identical to OPACK_SUB_PREFAB(name, base)
 */
#define OPACK_SUB_ACTUATOR(name, base) OPACK_SUB_PREFAB(name, base)

namespace opack
{
	struct ActuatorHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct ActuatorHandle : Handle
	{
		using Handle::Handle;

		/** Should this actuator track last @c ring_buffer_size previous actions done. */
		ActuatorHandle& track(std::size_t ring_buffer_size);
	};

	struct ActionHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct ActionHandle : Handle
	{
		using Handle::Handle;

		/** Indicates which actuator should be used to enact this. */
		ActionHandle& require(EntityView actuator_prefab);

		/** Indicates which actuator should be used to enact this. */
		template<ActuatorPrefab T>
		ActionHandle& require();

		/** Set action duration by value (in seconds). */
		ActionHandle& duration(float value);

		/** Called when action is beginning (after delay so).
		 * First argument is the action entity
		 * Second argument is the simulation time when it began.
		 */
		template<std::derived_from<opack::Action> T>
		ActionHandle& on_action_begin(std::function<void(Entity)> func);

		/** Called when action has begun and is not finished yet.
		 * First argument is the action entity
		 * Second argument is the simulation time when it began.
		 * Third argument is delta time (last_time since this was called
		 */
		template<std::derived_from<opack::Action> T>
		ActionHandle& on_action_update(std::function<void(Entity, float)> func);

		/** Called when action has ended
		 * First argument is the action entity
		 * Second argument is the simulation time when it began.
		 * Third argument is delta time (last_time since this was called
		 */
		template<std::derived_from<opack::Action> T>
		ActionHandle& on_action_end(std::function<void(Entity)> func);
	};

	/**
	 *@brief Add actuator @c T to entity @c prefab
	 *Usage:
	 *@code{.cpp}
	 OPACK_SUB_PREFAB(MyAgent, opack::Agent)
	 OPACK_SUB_PREFAB(MyActuator, opack::Actuator)
	 //...
	 opack::add_actuator<MyActuator, MyAgent>(world);
	 *@endcode
	 */
	template<ActuatorPrefab TActuator, std::derived_from<Agent> TAgent>
	void add_actuator(World& world);

	/**
	 * @brief Retrieve actuator from prefab @c actuator_prefab for current @c entity.
	 */
	Entity actuator(EntityView actuator_prefab, EntityView entity);

	/**
	 * @brief Retrieve instanced actuator @c T for current entity.
	 *
	 * WARNING : If you want to retrieve the actuator prefab, identified by
	 * @c T, use @ref entity<T>.
	 */
	template<ActuatorPrefab T>
	Entity actuator(EntityView entity);

	EntityView get_required_actuator(EntityView action);

	/**
	 * @brief Create an instanced action @c T.
	 */
	template<std::derived_from<opack::Action> T>
	Entity action(EntityView entity);

	/**
	 * @brief Create an instanced action from prefab @c action_prefab.
	 */
	Entity action(EntityView action_prefab);

	/**
	@brief Return current action done by @c entity with @c actuator_prefab. Returns null entity if action is done.
	*/
	Entity current_action(EntityView actuator_prefab, EntityView entity);

	/**
	@brief Return current action done by @c entity with actuator @c T. Returns null entity if action is done.
	*/
	template<ActuatorPrefab T>
	Entity current_action(EntityView entity);

	/**
	 * Return last @c n th action_prefab done. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value..
	 * If @c actuator do not track actions, the null entity is returned.
	 */
	EntityView last_action_prefab(EntityView actuator, std::size_t n = 0);

	/**
	 * Return last @c n th action_prefab done. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value..
	 * If @c actuator do not track actions, the null entity is returned.
	 */
	template<ActuatorPrefab T>
	EntityView last_action_prefab(EntityView entity, std::size_t n = 0);

	/**
	 * Return last action_prefab done.
	 * If @c actuator do not track actions, it will assert.
	 */
	template<ActuatorPrefab T>
	const LastActionPrefabs& last_actions(EntityView entity);

	/**
	 * Return true if @c entity has done @c action_prefab, false otherwise. Only relevant if
	 * @c actuator is tracking actions prefab.
	 */
	bool has_done(EntityView entity, EntityView action_prefab);

	/**
	 * Return true if @c action has started.
	 */
	bool has_started(EntityView action);

	/**
	 * Return true if @c action is in progress.
	 */
	bool is_in_progress(EntityView action);

	/**
	 * Return true if @c action is finished.
	 */
	bool is_finished(EntityView action);

	/**
	 * Return true if @c entity has done @c T. False otherwise. Only relevant if
	 * @c actuator is tracking actions prefab.
	 */
	template<ActionPrefab T>
	bool has_done(EntityView entity);

	/**
	@brief @c initiator is now doing @c action.
	*/
	void act(Entity initiator, Entity action);

	/**
	@brief @c initiator is now doing @c action.
	*/
	void act(Entity initiator, EntityView action_prefab);

	/**
	@brief Returns current action status of @c action.
	*/
	ActionStatus action_status(EntityView action);

	/**
	@brief Returns supposed duration of action in simulation time (seconds).
	*/
	float duration(EntityView action);

	/**
	@brief Returns effective duration of action in simulation time (seconds).
	*/
	float effective_duration(EntityView action);

	/**
	@brief @c initiatior is now doing an action of type @c T
	@return An @c ActionHandle if you need to tailor the action.
	*/
	template<ActionPrefab T>
	ActionHandle act(Entity initiator);

	/** Get the @c n -nth initiator of provided @c action.*/
	inline Entity initiator(EntityView action, size_t n = 0)
	{
		opack_assert(action.is_valid(), "Action is invalid");
		auto entity = action.target<By>(static_cast<int>(n));
		return entity;
	}

	template<std::derived_from<Action> T>
	void on_action_begin(World& world, std::function<void(Entity)> func)
	{
		world.system(fmt::format(fmt::runtime("System_OnBegin_{}"), friendly_type_name<T>().c_str()).c_str())
			.template kind<Act::PreUpdate>()
			.term(flecs::IsA).template second<T>()
			.term(ActionStatus::starting)
			.each([func](flecs::entity action)
				{
					func(action);
				}
			).template child_of<opack::world::dynamics>();
	}

	template<std::derived_from<opack::Action> T>
	void on_action_update(World& world, std::function<void(Entity, float)> func)
	{
		world.system(fmt::format(fmt::runtime("System_OnUpdate_{}"), friendly_type_name<T>().c_str()).c_str())
			.template kind<Act::Update>()
			.term(flecs::IsA).template second<T>()
			.term(ActionStatus::running)
			.each([func](flecs::entity entity)
				{
					func(entity, entity.delta_time());
				}
			).template child_of<opack::world::dynamics>();
	}

	template<std::derived_from<opack::Action> T>
	void on_action_cancel(World& world, std::function<void(Entity)> func)
	{
	}

	template<std::derived_from<opack::Action> T>
	void on_action_end(World& world, std::function<void(Entity)> func)
	{
		world.system(fmt::format(fmt::runtime("System_OnEnd_{}"), friendly_type_name<T>().c_str()).c_str())
			.template kind<Act::PostUpdate>()
			.term(flecs::IsA).template second<T>()
			.term(ActionStatus::finished)
			.template term<Token>().self()
			.each([func](flecs::entity entity)
				{
					func(entity);
				}
			).template child_of<opack::world::dynamics>();
	}

	// --------------------------------------------------------------------------- 
	// Definition
	// --------------------------------------------------------------------------- 

	inline ActuatorHandle& ActuatorHandle::track(std::size_t ring_buffer_size)
	{
		opack_assert(ring_buffer_size > 0, "Ring buffer size is equal to zero, which is invalid !");
		set_override<LastActionPrefabs>( {ring_buffer_size});
		return *this;
	}

	inline ActionHandle& ActionHandle::require(EntityView actuator_prefab)
	{
		set<RequiredActuator>({ actuator_prefab });
		return *this;
	}

	template<ActuatorPrefab T>
	ActionHandle& ActionHandle::require()
	{
		set<RequiredActuator>({ world().entity<T>() });
		return *this;
	}

	inline ActionHandle& ActionHandle::duration(float value)
	{
		set_override<Duration>({ value });
		return *this;
	}

	template<std::derived_from<opack::Action> T>
	ActionHandle& ActionHandle::on_action_begin(std::function<void(Entity)> func)
	{
		auto world_ = world();
		opack::on_action_begin<T>(world_, func);
		return *this;
	}

	template<std::derived_from<opack::Action> T>
	ActionHandle& ActionHandle::on_action_update(std::function<void(Entity, float)> func)
	{
		auto world_ = world();
		opack::on_action_update<T>(world_, func);
		return *this;
	}

	template<std::derived_from<opack::Action> T>
	ActionHandle& ActionHandle::on_action_end(std::function<void(Entity)> func)
	{
		auto world_ = world();
		opack::on_action_end<T>(world_, func);
		return *this;
	}

	template<ActuatorPrefab TActuator, std::derived_from<Agent> TAgent>
	void add_actuator(World& world)
	{
		// Waiting for fix : https://github.com/SanderMertens/flecs/issues/791
		world.observer(fmt::format(fmt::runtime("Observer_AddActuator_{}_to_{}"), friendly_type_name<TActuator>().c_str(), friendly_type_name<TAgent>().c_str()).c_str())
			.event(flecs::OnAdd)
			.term(flecs::IsA).template second<TAgent>()
			.each(
				[](flecs::entity e)
				{
					auto child = e.world().entity().is_a<TActuator>().child_of(e);
					child.set_name(friendly_type_name<TActuator>().c_str());
					e.add<TActuator>(child);
				}
		).template child_of<world::dynamics>();
	}

	inline Entity actuator(EntityView actuator_prefab, EntityView entity)
	{
		opack_assert(entity.is_valid(), "Given entity is invalid.");
		opack_assert(actuator_prefab.is_valid(), "Given actuator prefab is invalid");
		opack_assert(entity.target(actuator_prefab).is_valid(), "No actuator \"{0}\" for entity \"{1}\". Make sure this was called : \"opack::add_actuator<{0}, YourPrefab>(world)\".", actuator_prefab.path().c_str(), entity.path().c_str());
		return entity.target(actuator_prefab);
	}

	template<ActuatorPrefab T>
	Entity actuator(EntityView entity)
	{
		opack_assert(entity.target<T>().is_valid(), "No actuator \"{0}\" for entity \"{1}\". Make sure this was called : \"opack::add_actuator<{0}, YourPrefab>(world)\".", type_name_cstr<T>(), entity.path().c_str());
		return entity.target<T>();
	}

	inline EntityView get_required_actuator(EntityView action)
	{
		opack_assert(action.is_valid(), "Given action is invalid.");
		opack_assert(action.has<RequiredActuator>(), "Action {} has no required actuator set ! Did you call something like : `opack::init<MyAction>(world).require<MyActuator>();` ?", action.path().c_str());
		return action.get<RequiredActuator>()->value;
	}

	template<std::derived_from<opack::Action> T>
	Entity action(EntityView entity)
	{
		opack_assert(entity.is_valid(), "Given entity is invalid.");
		auto world = entity.world();
		return opack::spawn<T>(world);
	}

	/**
	 * Return last @c n th action_prefab done. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value..
	 * If @c actuator do not track actions, the null entity is returned.
	 */
	inline EntityView last_action_prefab(EntityView actuator, std::size_t n)
	{
		opack_assert(actuator.is_valid(), "Given actuator is invalid.");
		if (actuator.has<LastActionPrefabs>())
		{
			return actuator.get<LastActionPrefabs>()->peek(n);
		}
		opack_warn("Actuator [{}] with parent [{}] do not track previous actions.", actuator.path().c_str(), actuator.parent().path().c_str());
		return flecs::entity::null();
	}

	template<ActuatorPrefab T>
	EntityView last_action_prefab(EntityView entity, std::size_t n)
	{
		return last_action_prefab(opack::actuator<T>(entity), n);
	}

	template<ActuatorPrefab T>
	const LastActionPrefabs& last_actions(EntityView entity)
	{
		opack_assert(opack::actuator<T>(entity).is_valid(), "Entity {} has no actuator {}.", entity.path().c_str(), type_name_cstr<T>());
		opack_assert(opack::actuator<T>(entity).template has<LastActionPrefabs>(), "Entity {} with actuator {} has no LastActionsPrefabs. Did you make sure you called 'track' on actuator ?", entity.path().c_str(), type_name_cstr<T>());
		return *opack::actuator<T>(entity).template get<LastActionPrefabs>();
	}

	inline bool has_done(EntityView entity, EntityView action_prefab)
	{
		opack_assert(entity.is_valid(), "Given entity is invalid.");
		opack_assert(action_prefab.is_valid(), "Given action_prefab is invalid.");
		const auto actuator = opack::actuator(get_required_actuator(action_prefab), entity);
		if (actuator.has<LastActionPrefabs>())
		{
			return actuator.get<LastActionPrefabs>()->has_done(action_prefab);
		}
		opack_warn("Tried seeing if entity [{}] has done action [{}], but actuator [{}] is not tracking previous actions.", entity.path().c_str(), action_prefab.path().c_str(), actuator.path().c_str());
		return false;
	}

	template<ActionPrefab T>
	bool has_done(EntityView entity)
	{
		auto world = entity.world();
		return has_done(entity, opack::entity<T>(world));
	}

	inline Entity action(EntityView action_prefab)
	{
		opack_assert(action_prefab.is_valid(), "Trying to create an action from an invalid action prefab.");
		return opack::spawn(action_prefab);
	}

	inline Entity current_action(EntityView actuator_prefab, EntityView entity)
	{
		opack_assert(entity.is_valid(), "Given entity is invalid.");
		opack_assert(actuator_prefab.is_valid(), "Given actuator prefab is invalid");
		return opack::actuator(actuator_prefab, entity).template target<Doing>();
	}

	template<ActuatorPrefab T>
	Entity current_action(Entity entity)
	{
		opack_assert(entity.is_valid(), "Given entity is invalid.");
		return opack::actuator<T>(entity).template target<Doing>();
	}

	inline void act(Entity initiator, EntityView action)
	{
		opack_assert(initiator.is_valid(), "Given initiator is invalid.");
		opack_assert(action.is_valid(), "Given action is invalid.");
		act(initiator, action.mut(initiator));
	}

	inline void act(Entity initiator, Entity action)
	{
		opack_assert(initiator.is_valid(), "Given initiator is invalid.");
		opack_assert(action.is_valid(), "Given action is invalid.");
		opack_assert(action.has<RequiredActuator>(), "Action {0} has no required actuator set ! Did you call : opack::init<YourAction>(world).require<YourActuator>().", action.path().c_str());
		opack_assert(action.get<RequiredActuator>()->value.is_valid(), "Action required actuator set is invalid ! Did you call : opack::init<YourAction>(world).require(required_actuator) with a correct actuator ?", action.path().c_str());

		flecs::entity effective_action{ action };
		if (action.has(flecs::Prefab))
			effective_action = spawn(action);

		auto actuator = opack::actuator(action.get<RequiredActuator>()->value, initiator);
		auto last_action = actuator.template target<Doing>();
		if (last_action)
			last_action.mut(action); //TODO Handle cancel

		//TODO Wait / Error on arity not satisfied

		effective_action.mut(action)
			.remove<Begin, Timestamp>()
			.remove<End, Timestamp>()
			.add<By>(initiator)
			.set<Begin, Timestamp>({ action.world().time() })
			.add(ActionStatus::starting)
			.set_doc_name(action.name())
		;
		actuator.mut(action).template add<Doing>(effective_action).template add<Token>();
	}

	template<ActionPrefab T>
	ActionHandle act(Entity initiator)
	{
		auto world = initiator.world();
		auto action = opack::spawn<T>(world);
		act(initiator, action);
		return ActionHandle(world, action);
	}

	inline ActionStatus action_status(EntityView action)
	{
		opack_assert(action.is_valid(), "Action is invalid.\n");
		opack_assert(action.has<ActionStatus>(flecs::Wildcard), "Action {} has no action status.\n", action.path().c_str());
		return *action.get<ActionStatus>();
	}

	inline float duration(EntityView action)
	{
		opack_assert(action.is_valid(), "Action is invalid.\n");
		opack_assert(action.has<Duration>(), "Action {} has no action status.\n", action.path().c_str());
		return action.get<Duration>()->value;
	}

	inline float effective_duration(EntityView action)
	{
		opack_assert(action.is_valid(), "Action is invalid.\n");
		if (!action.has<Begin, Timestamp>())
			return 0.0f;
		if (action.has<End, Timestamp>())
			return action.get<End, Timestamp>()->value - action.get<Begin, Timestamp>()->value;
		return action.world().time() - action.get<Begin, Timestamp>()->value;
	}

	inline bool has_started(EntityView action)
	{
		return action.has<Begin, Timestamp>();
	}

	inline bool is_in_progress(EntityView action)
	{
		return has_started(action) && !is_finished(action);
	}

	inline bool is_finished(EntityView action)
	{
		return action.has<End, Timestamp>();
	}

}
