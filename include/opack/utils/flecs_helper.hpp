/*****************************************************************//**
 * \file   flecs_helper.hpp
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

namespace opack::internal
{
	/**
	 * Returns the number of children for entity @c e.
	 */
	inline size_t children_count(flecs::entity e)
	{
		size_t count{ 0 };
		e.children([&count](flecs::entity) {count++;});
		return count;
	}

	template<typename T, typename U>
	flecs::entity register_t_as_u(flecs::world& world)
	{
		return world.template prefab<T>().template is_a<U>().template add<T>();
	}

	// TODO Probably, copy should build a new query ? to prevent rule destruction if a copy is destroy ?
	// But not sure how flecs handles it since tests seems to work anyways.
	/**
	 * Holds a flecs rule and handles destruction.
	 */
	struct Rule
	{
		flecs::rule<> rule;

		Rule(flecs::rule<> _rule) : 
			rule{_rule}
		{}

		Rule(const Rule& other) : 
			rule{other.rule}
		{}

		Rule& operator=(const Rule& other) 
		{
			if (this == &other)
				return *this;

			this->rule = other.rule;

			return *this;
		}

		~Rule()
		{
			rule.destruct();
		}
	};
}
