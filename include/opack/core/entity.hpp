/*****************************************************************//**
 * \file   entity.hpp
 * \brief  API for manipulating entity
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

#include <opack/core/types.hpp>

namespace opack
{
    /** 
    @brief Returns true if given entity is an instance of given prefab

    @tparam T Prefab's type
    @param entity 
    @return A prefab entity instantiated from prefab @c T.

    Usage :

    @code{.cpp}
    OPACK_PREFAB(A); // expands to struct A {};
    OPACK_SUB_PREFAB(B, A); // expands to struct B : public A {using base_t = A; };
    auto a = opack::prefab<A>(world);
    auto b = opack::init<B>(world);
    // customize prefab a ...
    auto e1 = opack::spawn<A>(world, "Arthur");
    auto e2 = opack::spawn<B>(world, "Bob");
    opack::is_a<A>(e1); // true
    opack::is_a<B>(e2); // true
    opack::is_a<A>(e2); // false
    @endcode
    */
	template<typename T>
	bool is_a(Entity entity)
	{
        auto world = entity.world();
		return entity.has(flecs::IsA, prefab<T>(world));
	}
}
