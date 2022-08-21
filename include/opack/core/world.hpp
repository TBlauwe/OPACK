/*****************************************************************//**
 * \file   world.hpp
 * \brief  API for manipulating the world.
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
    @brief Retrieve (or create) an entity from given type @c T.

    @tparam T Any type.
    @param world 
    @return Entity associated with @c T.

    When no entity are associated with this type, a new entity will be spawned.

    Usage :

    @code{.cpp}
    struct A {};
    auto e = opack::entity<A>(world);
    // At any moment, in any place ...
    opack::entity<A>(world) == e; // true
    @endcode
    */
    template<typename T>
    Entity entity(World& world)
    {
        return world.entity<T>();
    }

    /** 
    @brief Retrieve (or create) a prefab associated with given type @c T.

    @tparam T Any type.
    @param world 
    @return A prefab entity associated with type @c T.

    See also @ref spawn to instantiate an entity from this prefab.

    Usage :

    @code{.cpp}
    struct A {};
    auto prefab = opack::prefab<A>(world);
    // customize prefab ...
    auto e = opack::spawn<A>(world);
    opack::is_a<A>(e); // true
    @endcode
    */
    template<typename T>
    Entity prefab(World& world)
    {
        return world.prefab<T>();
    }
    
    /** 
    @brief Spawn a new entity instantiated from prefab @c T.

    @tparam T Any type that matches a prefab.
    @param world 
    @return A entity instanted from prefab @c T.

    Usage :

    @code{.cpp}
    struct A {};
    auto prefab = opack::prefab<A>(world);
    // customize prefab ...
    auto e = opack::spawn<A>(world);
    opack::is_a<A>(e); // true
    @endcode
    */
    template<typename T>
    Entity spawn(World& world)
    {
        return world.entity().is_a<T>();
    }

    /** 
    @brief Spawn a new entity with name @c name, instantiated from prefab @c T.

    @tparam T Any type that matches a prefab.
    @param world 
    @param name Must be unique (in current scope). 
    @return A entity instantiated from prefab @c T

    Usage :

    @code{.cpp}
    struct A {};
    auto prefab = opack::prefab<A>(world);
    // customize prefab ...
    auto e = opack::spawn<A>(world, "Arthur");
    opack::is_a<A>(e); // true
    @endcode
    */
    template<typename T>
    Entity spawn(World& world, const char * name)
    {
        return world.entity(name).is_a<T>();
    }

    /** 
    @brief Initialize a sub-prefab, so it correctly inherits its parent.

    @tparam T Any type that matches a sub-prefab.
    @param world 
    @return A prefab entity instantiated from prefab @c T.

    A sub-prefab is a prefab based on another one. This function ensures that this link
    is formed.

    Afterwards, you can retrieve any prefab (sub-prefab also) by calling @ref prefab.

    We do not need to specify from which prefab the sub-prefab inherits. It should be
    already specified in its definition. It is done automatically with the helper macro
    @ref OPACK_SUB_PREFAB(name, base). Or manually, by adding the static typename member @c base_t
    equal to the type of the inheriting prefab (see below).

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
    @endcode
    */
	template<SubPrefab T>
	Entity init(World& world)
	{
        auto e = prefab<T>(world);
        e.template is_a<typename T::base_t>();
#ifndef OPACK_OPTIMIZE
        if constexpr (ActionPrefab<T>)
            e.template child_of<world::prefab::Actions>();
        else if constexpr (AgentPrefab<T>)
			e.template child_of<world::prefab::Agents>();
        else if constexpr (ArtefactPrefab<T>)
			e.template child_of<world::prefab::Artefacts>();
#endif
        return e;
	}

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