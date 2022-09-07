/*****************************************************************//**
 * \file   entity.hpp
 * \brief  API for manipulating entity
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

#include <opack/core/api_types.hpp>

namespace opack
{
    /** 
    @brief Returns true if @c entity is an instance of @c prefab.

    Usage :

    @code{.cpp}
    OPACK_PREFAB(A); // expands to struct A {};
    OPACK_SUB_PREFAB(B, A); // expands to struct B : public A {using base_t = A; };
    auto a = opack::prefab<A>(world);
    auto b = opack::init<B>(world);
    // customize prefab a ...
    auto e1 = opack::spawn<A>(world, "Arthur");
    auto e2 = opack::spawn<B>(world, "Bob");
    opack::is_a(a, e1); // true
    opack::is_a(b, e2); // true
    opack::is_a(a, e2); // false
    @endcode
    */
    bool is_a(Entity prefab, Entity entity);

    /** 
    @brief Returns true if @c entity is an instance of prefab @c T.

    @tparam T Prefab's type
    @param entity to check 

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
    bool is_a(Entity entity);


    // --------------------------------------------------------------------------- 
    // Definition
    // --------------------------------------------------------------------------- 
	inline bool is_a(Entity prefab, Entity entity)
	{
		return entity.has(flecs::IsA, prefab);
	}

	template<typename T>
	bool is_a(Entity entity)
	{
		return is_a(entity.world().entity<T>(), entity);
	}
}

