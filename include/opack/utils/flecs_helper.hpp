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
	 * Get prefab of type @c T.
	 */
	template<typename T>
	flecs::entity prefab(flecs::world& world)
	{
		return *world.entity<T>();
	}

    template<typename T>
    void organize(flecs::entity& entity)
    {
#ifndef OPACK_OPTIMIZATION
        entity.child_of<T>();
#endif
    }

    template<typename T>
    void doc_name(flecs::entity& entity, const char* name)
    {
#ifndef OPACK_OPTIMIZATION
        entity.set_doc_name(name);
#endif
    }

    template<typename T>
    void doc_brief(flecs::entity& entity, const char* brief)
    {
#ifndef OPACK_OPTIMIZATION
        entity.set_doc_brief(brief);
#endif
    }

    /**
     * Returns the number of children for entity @c e.
     */
    inline size_t children_count(flecs::entity e)
    {
        size_t count{ 0 };
        e.children([&count](flecs::entity) {count++; });
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
            rule{ _rule }
        {}

        Rule(const Rule& other) :
            rule{ other.rule }
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
