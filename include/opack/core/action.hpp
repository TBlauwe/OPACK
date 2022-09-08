/*****************************************************************//**
 * \file   action.hpp
 * \brief  Action API.
 * 
 * \author Hyperenor
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

#include <opack/core/api_types.hpp>
#include <opack/core/components.hpp>
#include <opack/utils/type_name.hpp>

#include <functional>

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
	};

	struct ActionHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct ActionHandle : Handle, ActionHandleView
	{
		using Handle::Handle;

		ActionHandle& require(const EntityView& actuator_prefab);

		template<ActuatorPrefab T>
		ActionHandle& require();
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
	Entity actuator(const EntityView& actuator_prefab, const EntityView& entity);

	/**
	 * @brief Retrieve instanced actuator @c T for current entity.
	 *
	 * WARNING : If you want to retrieve the actuator prefab, identified by
	 * @c T, use @ref entity<T>.
	 */
	template<ActuatorPrefab T>
	Entity actuator(const EntityView& entity);

	/**
	 * @brief Create an instanced action @c T.
	 */
	template<std::derived_from<opack::Action> T>
	Entity action(const Entity& entity);

	/**
	@brief Return current action done by @c entity with @c actuator_prefab
	*/
	Entity current_action(Entity actuator_prefab, Entity entity);

	/**
	@brief Return current action done by @c entity with actuator @c T
	*/
	template<ActuatorPrefab T>
	Entity current_action(Entity entity);

	/**
	@brief @c initiator is now doing @c action.
	*/
	void act(Entity initiator, Entity action);

	/**
	@brief @c initiatior is now doing an action of type @c T
	@return An @c ActionHandle if you need to tailor the action.
	*/
	template<ActionPrefab T>
	ActionHandle act(Entity initiator);

	/** Get the @c n -nth initiator of provided @c action.*/
	inline Entity initiator(Entity& action, size_t n = 0)
	{
		auto entity = action.target<By>(static_cast<int>(n));
		return entity.mut(action);
	}

	//TODO Maybe use systems instead of callback ? Maybe systems are better overall, espially, that nothing prevents from adding multiple on_begin, etc.
	template<ActionPrefab T>
	void on_action_begin(World& world, std::function<void(Entity)> func)
	{
		opack::prefab<T>(world).template set<OnBegin>({ func });
	}

	template<ActionPrefab T>
	void on_action_update(World& world, std::function<void(Entity, float)> func)
	{
		opack::prefab<T>(world).template set<OnUpdate>({ func });
	}

	template<ActionPrefab T>
	void on_action_cancel(World& world, std::function<void(Entity)> func)
	{
		opack::prefab<T>(world).template set<OnCancel>({ func });
	}

	template<ActionPrefab T>
	void on_action_end(World& world, std::function<void(Entity)> func)
	{
		opack::prefab<T>(world).template set<OnEnd>({ func });
	}

	// --------------------------------------------------------------------------- 
	// Definition
	// --------------------------------------------------------------------------- 

	inline ActionHandle& ActionHandle::require(const EntityView& actuator_prefab)
	{
		override<RequiredActuator>(actuator_prefab);
		return *this;
	}

	template<ActuatorPrefab T>
	ActionHandle& ActionHandle::require()
	{
		override<RequiredActuator, T>();
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
					internal::name_entity_after_type<TActuator>(child);
					e.add<TActuator>(child);
				}
		).template child_of<world::dynamics>();
	}

	inline Entity actuator(const EntityView& actuator_prefab, const EntityView& entity)
	{
		auto actuator = entity.target(actuator_prefab);
#ifdef OPACK_ASSERTS
		ecs_assert(actuator.is_valid(), ECS_INVALID_OPERATION, "No actuator \"{}\" for entity \"{}\". Make sure this was called : \"opack::add_actuator<{0}, YourPrefab>(world)\".", actuator_prefab, entity.path());
#endif
		return actuator;
	}

	template<ActuatorPrefab T>
	Entity actuator(const opack::EntityView& entity)
	{
		auto actuator = entity.target<T>();
#ifdef OPACK_ASSERTS
		ecs_assert(actuator.is_valid(), ECS_INVALID_OPERATION, "No actuator \"{}\" for entity \"{}\". Make sure this was called : \"opack::add_actuator<{0}, YourPrefab>(world)\".", type_name_cstr<T>(), entity.path());
#endif
		return actuator;
	}

	template<std::derived_from<opack::Action> T>
	Entity action(const Entity& entity)
	{
		auto world = entity.world();
		return opack::spawn<T>(world);
	}

	inline Entity current_action(opack::Entity actuator_prefab, Entity entity)
	{
		return opack::actuator(actuator_prefab, entity).template target<Doing>();
	}

	template<ActuatorPrefab T>
	Entity current_action(Entity entity)
	{
		return opack::actuator<T>(entity).template target<Doing>();
	}

	inline void act(Entity initiator, Entity action)
	{
		auto actuator = opack::actuator(action.target<RequiredActuator>(), initiator);
		auto last_action = actuator.template target<Doing>();
		if (last_action)
		{
			last_action.mut(action)
				.template add<Cancel>();
		}
		action.mut(action).add<By>(initiator);
		actuator.template add<Doing>(action);
	}

	template<ActionPrefab T>
	ActionHandle act(Entity initiator)
	{
		auto world = initiator.world();
		auto action = opack::spawn<T>(world);
		act(initiation, action);
		return ActionHandle(world, action);
	}
}
