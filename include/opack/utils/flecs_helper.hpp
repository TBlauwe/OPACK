/*****************************************************************//**
 * \file   flecs_helper.hpp
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

namespace opack::internal
{
	// TODO Probably, copy should build a new query ? to prevent rule destruction if a copy is destroy ? But not sure how flecs handles it
	// since tests seems to work anyways.
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
