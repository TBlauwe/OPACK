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
		return world.entity().template is_a<T>().set_doc_name(type_name_cstr<T>());
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
		auto last_action = initiator.get_object<T>();
		if (last_action)
			last_action.mut(initiator.world()).template remove<By>(initiator);

		action.add<By>(initiator);
		initiator.add<T>(action);
	}
}
