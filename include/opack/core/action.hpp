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
#include <opack/utils/type_name.hpp>

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
					e.add<TActuator>(child);
				}
		).template child_of<world::dynamics>();
	}

	/**
	 * @brief Retrieve instanced actuator @c T for current entity.
	 *
	 * WARNING : If you want to retrieve the sense prefab, identified by
	 * @c T, use @ref entity<T>.
	 */
	template<ActuatorPrefab T>
	Entity actuator(const Entity& entity)
	{
#ifdef OPACK_DEBUG
		auto sense = entity.target<T>();
		ecs_assert(sense.is_valid(), ECS_INVALID_OPERATION, "No sense for given entity. Make sure to add sense to its prefab (or to it directly");
		return sense;
#else 
		return entity.target<T>();
#endif
	}

	/**
	@brief Return current action done by @c entity with actuator @c T
	*/
	template<ActuatorPrefab T>
	Entity current_action(Entity entity)
	{
		return opack::actuator<T>(entity).template target<T>();
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
		auto actuator = opack::actuator<T>(initiator).template target<T>();
		auto last_action = actuator.template target<Act>();
		if (last_action)
		{
			last_action.mut(world)
		        .template set<End, Timestamp>({world.time()})
			    .template remove<By>(initiator);
		}
		
		action.mut(world)
			.add<By>(initiator)
			.set<Begin, Timestamp>({ world.time() });
		actuator.template add<T>(action);
	}
}
