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
					_::name_entity_after_type<TActuator>(child);
					e.add<TActuator>(child);
				}
		).template child_of<world::dynamics>();
	}

	/**
	 * @brief Retrieve instanced actuator @c T for current entity.
	 *
	 * WARNING : If you want to retrieve the actuator prefab, identified by
	 * @c T, use @ref entity<T>.
	 */
	template<ActuatorPrefab T>
	Entity actuator(const Entity& entity)
	{
#ifdef OPACK_DEBUG
		auto actuator = entity.target<T>();
		ecs_assert(actuator.is_valid(), ECS_INVALID_OPERATION, "No actuator for given entity. Make sure to add actuator to its prefab (or to it directly).");
		return actuator;
#else 
		return entity.target<T>();
#endif
	}

	/**
	 * @brief Create an instanced action @c T.
	 */
	template<std::derived_from<opack::Action> T>
	Entity action(const Entity& entity)
	{
		auto world = entity.world();
		return opack::spawn<T>(world);
	}


	/**
	@brief Return current action done by @c entity with actuator @c T
	*/
	template<ActuatorPrefab T>
	Entity current_action(Entity entity)
	{
		return opack::actuator<T>(entity).template target<Doing>();
	}

	/**
	@brief @c initiator is now acting with actuator @c to accomplish given @c action.
	*/
	template<ActuatorPrefab T>
	void act(Entity initiator, Entity action)
	{
		//size_t count{ 0 };
		//Entity last;
		//action.each<By>([&count, &last](Entity object) {count++; last = object; });

		//if (count >= action.get<Arity>()->value)
		//{
		//	//TODO Should issue warning - Here we replace the last initiator.
		//	//There will be a bug since we do not remove the relation from the initiator to the action.
		//	action.remove<By>(last);
		//}

		// Action without initiator are cleaned up, so we need to remove relation from previous action.
		auto world = initiator.world();
		auto actuator = opack::actuator<T>(initiator);
		auto last_action = actuator.template target<Doing>();
		if (last_action)
		{
			last_action.mut(world)
				.template add<Cancel>();
		}
		
		action.mut(world)
			.add<By>(initiator)
			.add<Actuator>(actuator);
		actuator.template add<Doing>(action);
	}

	/** Get the @c n -nth initiator of provided @c action.*/
	inline Entity initiator(Entity& action, size_t n = 0)
	{
		auto entity = action.target<By>(static_cast<int>(n));
		return entity.mut(action);
	}

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
}
