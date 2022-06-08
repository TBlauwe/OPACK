/*********************************************************************
 * \file   action.hpp
 * \brief  API for manipulating actions
 * 
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

#include <opack/core/types.hpp>
#include <opack/utils/type_name.hpp>

namespace opack
{
	/**
	@brief Create an action of type @c T. Compose the action as required, before having entites acting on it.
	*/
	template<std::derived_from<Action> T>
	flecs::entity action(flecs::world& world)
	{
		return world.entity().is_a(prefab<Action, T>(world)).set_doc_name(type_name_cstr<T>()).template child_of<world::Actions>();
	}

	/**
	@brief @c initiator is now acting with actuator @c to accomplish given @c action.
	*/
	template<std::derived_from<Actuator> T>
	void act(flecs::entity initiator, flecs::entity action)
	{
		//size_t count{ 0 };
		//flecs::entity last;
		//action.each<By>([&count, &last](flecs::entity object) {count++; last = object; });

		//if (count >= action.get<Arity>()->value)
		//{
		//	//TODO Should issue warning - Here we replace the last initiator.
		//	//There will be a bug since we do not remove the relation from the initiator to the action.
		//	action.remove<By>(last);
		//}

		// Action without initiator are cleaned up, so we need to remove relation from previous action.
		auto world = initiator.world();
		auto last_action = initiator.get_object<T>();
		if (last_action)
		{
			last_action.mut(world).template set<End, Timestamp>({world.time()});
			last_action.mut(world).template remove<By>(initiator);
		}
		
		action.mut(world).add<By>(initiator);
		action.mut(world).set<Begin, Timestamp>({world.time()});
		action.mut(world).remove<End, Timestamp>();
		initiator.add<T>(action);
	}
}
