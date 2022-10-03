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
	@brief Return current action done by @c entity with @c actuator_prefab
	*/
	Entity current_action(EntityView actuator_prefab, EntityView entity);

	/**
	@brief Return current action done by @c entity with actuator @c T
	*/
	template<ActuatorPrefab T>
	Entity current_action(EntityView entity);

	/**
	 * Return last @c n th action_prefab done. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value..
	 * If @c actuator do not track actions, the null entity is returned.
	 */
	EntityView last_action(EntityView entity, EntityView actuator, std::size_t n = 0);

	/**
	 * Return last @c n th action_prefab done. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value..
	 * If @c actuator do not track actions, the null entity is returned.
	 */
	template<ActuatorPrefab T>
	EntityView last_action(EntityView entity, std::size_t n = 0);

	/**
	 * Return true if @c entity has done @c action_prefab. False otherwise, even more if
	 * @c actuator do not track actions.
	 */
	bool has_done(EntityView entity, EntityView action_prefab);

	/**
	 * Return true if @c entity has done @c T. False otherwise, even more if
	 * @c actuator do not track actions.
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
	@brief Returns duration of action is simulation time (seconds).
	*/
	float duration(EntityView action);

	/**
	@brief @c initiatior is now doing an action of type @c T
	@return An @c ActionHandle if you need to tailor the action.
	*/
	template<ActionPrefab T>
	ActionHandle act(Entity initiator);

	/** Get the @c n -nth initiator of provided @c action.*/
	inline Entity initiator(EntityView action, size_t n = 0)
	{
		auto entity = action.target<By>(static_cast<int>(n));
		return entity.mut(action);
	}

	template<std::derived_from<Action> T>
	void on_action_begin(World& world, std::function<void(Entity)> func)
	{
		world.system()
			.kind<Act::PreUpdate>()
			.term(flecs::IsA).second<T>()
			.term(ActionStatus::starting)
			.each([func](flecs::entity action)
				{
					func(action);
				}
			).template child_of<opack::world::dynamics>()
		.set_doc_name(fmt::format(fmt::runtime("System_OnBegin_{}"), type_name_cstr<T>()).c_str());
	}

	template<std::derived_from<opack::Action> T>
	void on_action_update(World& world, std::function<void(Entity, float)> func)
	{
		world.system()
			.kind<Act::Update>()
			.term(flecs::IsA).second<T>()
			.term(ActionStatus::running)
			.each([func](flecs::iter& it, size_t index)
				{
					func(it.entity(index), it.delta_system_time());
				}
			).template child_of<opack::world::dynamics>()
		.set_doc_name(fmt::format(fmt::runtime("System_OnUpdate_{}"), type_name_cstr<T>()).c_str());
	}

	template<std::derived_from<opack::Action> T>
	void on_action_cancel(World& world, std::function<void(Entity)> func)
	{
	}

	template<std::derived_from<opack::Action> T>
	void on_action_end(World& world, std::function<void(Entity)> func)
	{
		world.system()
			.kind<Act::PostUpdate>()
			.term(flecs::IsA).second<T>()
			.term(ActionStatus::finished)
			.template term<Token>().self()
			.each([func](flecs::entity entity)
				{
					func(entity);
				}
			).template child_of<opack::world::dynamics>()
		.set_doc_name(fmt::format(fmt::runtime("System_OnEnd{}"), type_name_cstr<T>()).c_str());
	}

	// --------------------------------------------------------------------------- 
	// Definition
	// --------------------------------------------------------------------------- 

	inline ActuatorHandle& ActuatorHandle::track(std::size_t ring_buffer_size)
	{
		opack_warn_if(ring_buffer_size != 0, "Ring buffer size is equal to zero ! Fallback to a value of 1. A ring buffer of size zero is invalid.");
		set<LastActionPrefabs>({ ring_buffer_size ? ring_buffer_size : 1 });
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
		world.observer()
			.event(flecs::OnAdd)
			.term(flecs::IsA).second<TAgent>()
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

	//TODO Wrong logic, or I should add another call to cover case when you give an actuator_prefab.
	/**
	 * Return last @c n th action_prefab done. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value..
	 * If @c actuator do not track actions, the null entity is returned.
	 */
	inline EntityView last_action(EntityView entity, EntityView actuator, std::size_t n)
	{
		opack_assert(entity.is_valid(), "Given entity is invalid.");
		opack_assert(actuator.is_valid(), "Given actuator is invalid.");
		if (actuator.has<LastActionPrefabs>())
		{
			return actuator.get<LastActionPrefabs>()->peek(n);
		}
		else
		{
			opack_warn("Tried getting last action for entity [{}] with actuator [{}] that do not track previous actions.", entity.path().c_str(), actuator.path().c_str());
			return flecs::entity::null();
		}
	}

	template<ActuatorPrefab T>
	EntityView last_action(EntityView entity, std::size_t n)
	{
		return last_action(entity, opack::actuator<T>(entity), n);
	}

	inline bool has_done(EntityView entity, EntityView action_prefab)
	{
		opack_assert(entity.is_valid(), "Given entity is invalid.");
		opack_assert(action_prefab.is_valid(), "Given action_prefab is invalid.");
		auto actuator = opack::actuator(opack::get_required_actuator(action_prefab), entity);
		if (actuator.has<LastActionPrefabs>())
		{
			return actuator.get<LastActionPrefabs>()->has_done(action_prefab);
		}
		else
		{
			opack_warn("Tried seeing if entity [{}] has done action [{}], but actuator [{}] is not tracking previous actions.", entity.path().c_str(), action_prefab.path().c_str(), actuator.path().c_str());
			return false;
		}
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
			last_action.mut(action); //TODO

		effective_action.mut(action)
			.add<By>(initiator)
			.add(ActionStatus::starting)
		;
		actuator.mut(action).template add<Doing>(effective_action);
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
		opack_assert(action.has<ActionStatus>(flecs::Wildcard), "Action {} has no action status.\n", action.path().c_str());
		return *action.get<ActionStatus>();
	}

	inline float duration(EntityView action)
	{
		if (!action.has<Begin, Timestamp>())
			return 0.0f;
		if (action.has<End, Timestamp>())
			return action.get<End, Timestamp>()->value - action.get<Begin, Timestamp>()->value;
		return action.world().time() - action.get<Begin, Timestamp>()->value;
	}
}
