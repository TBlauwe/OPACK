/*****************************************************************//**
 * \file   world.hpp
 * \brief  API for manipulating the world.
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
    @brief Retrieve (or create) an entity from given type @c T.

    @tparam T Any type.
    @param world explicit.
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
    Entity entity(World& world);

    /**
    @brief Retrieve (or create) a prefab associated with given type @c T.

    @tparam T Any type.
    @param world explicit.
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
    Entity prefab(World& world);

    /**
    @brief Initialize a sub-prefab according to its type and correctly inherits its parent.
    Since @c T is a special type (Agent, Artefact, etc.), we directly provide an handle to 
    manipulate them. Otherwise, a plain entity is returned.

    @tparam T Any type that matches a sub-prefab and a fundamental type.
    @param world explicit.
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
    template<typename T>
        requires SubPrefab<T> and HasHandle<T>
    typename T::handle_t init(World& world);

    /**
    @brief Initialize a sub-prefab according to its type and correctly inherits its parent.

    @tparam T Any type that matches a sub-prefab.
    @param world explicit.
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
    opack::Entity init(World& world);

    /**
    @brief Spawn a new entity instantiated from @c prefab.

    Usage :

    @code{.cpp}
    struct A {};
    auto prefab = opack::prefab<A>(world);
    // customize prefab ...
    auto e = opack::spawn(prefab);
    opack::is_a<A>(e); // true
    @endcode
    */
    Entity spawn(Entity& prefab);

    /**
    @brief Spawn a new entity instantiated from @c prefab, with @c name.

    @return A entity instantiated from @c prefab.

    Usage :

    @code{.cpp}
    struct A {};
    auto prefab = opack::prefab<A>(world);
    // customize prefab ...
    auto e = opack::spawn(prefab, "my_instance");
    @endcode
    */
    Entity spawn(Entity& prefab, const char * name);


    /**
    @brief Spawn a new entity instantiated from prefab @c T.

    @tparam T Any type that matches a prefab.
    @param world explicit.
    @return A entity instantiated from prefab @c T.

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
    Entity spawn(World& world);

    /**
    @brief Spawn a new entity with name @c name, instantiated from prefab @c T.

    @tparam T Any type that matches a prefab.
    @param world explicit.
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
    Entity spawn(World& world, const char* name);

    /**
    @brief Count number of entities matching the pattern.
    @param world explicit.
    @return Numbers of entities having @c T.

    WARNING ! Do not works with prefabs, e.g. : @c count<Agent>(world).
    Use @ref count(World& world, Entity rel, Entity obj).
    */
    template<typename T>
    size_t count(const World& world)
    {
        return static_cast<size_t>(world.count<T>());
    }

    /**
    @brief Count number of entities matching the pattern.
    @param world explicit.
    @param obj Match following pattern : entity--T-->obj.
    @return Numbers of matching pattern : entity--T-->obj.
    */
    template<typename T>
    size_t count(const World& world, const Entity obj)
    {
        return static_cast<size_t>(world.count<T>(obj));
    }

    /**
    @brief Count number of instance of type @c T.
    @tparam T Prefab's type
    @param world explicit.
    @return Numbers of instances of prefab @c T.
    */
    template<typename T>
    size_t count_instance(World& world)
    {
        return static_cast<size_t>(world.count(flecs::IsA, opack::entity<T>(world)));
    }

    /**
    @brief Count number of entities matching the pattern.
    @param world explicit.
    @param rel Relation entity.
    @param obj Object entity.
    @return Numbers of matching pattern : entity--rel-->obj.
    */
    size_t count(const World& world, const Entity rel, const Entity obj);

    /**
     *@brief For each instance of @c T, @c func is applied every update.
     *@tparam T must be used as a prefab.
     *
     *TODO Why no concepts ? We can't know by its type if it's used as a prefab or not.
     */
    template<typename T>
    void each(World& world, std::function<void(Entity)> func)
    {
        world.system()
            .term(flecs::IsA).second<T>()
            .each(func);
    }

    // --------------------------------------------------------------------------- 
    // Definition
    // --------------------------------------------------------------------------- 
    template<typename T>
    Entity entity(World& world)
    {
        return world.entity<T>();
    }

    template<typename T>
    Entity prefab(World& world)
    {
         return world.prefab<T>();
    }

    template<SubPrefab T>
    opack::Entity init(World& world)
    {
        auto e = prefab<T>(world);
        e.template is_a<typename T::base_t>();
        return e;
    }

    template<typename T>
        requires SubPrefab<T> and HasHandle<T>
    typename T::handle_t init(World& world)
    {
        auto e = prefab<T>(world);
        e.template is_a<typename T::base_t>();
        return typename T::handle_t(world, e);
    }

	inline size_t count(const World& world, const Entity rel, const Entity obj)
	{
		return static_cast<size_t>(world.count(rel, obj));
	}

    inline Entity spawn(Entity& prefab)
    {
#ifdef OPACK_ASSERTS
        ecs_assert(prefab.has(flecs::Prefab), ECS_INVALID_PARAMETER, fmt::format("Type \"{0}\" has not been initialized. Don't forget to initialize it !", prefab.path()).c_str());
#endif 
        return prefab.world().entity().is_a(prefab);
    }

    inline Entity spawn(Entity& prefab, const char * name)
    {
#ifdef OPACK_ASSERTS
        ecs_assert(prefab.has(flecs::Prefab), ECS_INVALID_PARAMETER, fmt::format("Type \"{0}\" has not been initialized. Don't forget to initialize it !", prefab.path()).c_str());
#endif 
        return prefab.world().entity(name).is_a(prefab);
    }

    template<typename T>
    Entity spawn(World& world)
    {
#ifdef OPACK_ASSERTS
        ecs_assert(world.entity<T>().has(flecs::Prefab), ECS_INVALID_PARAMETER, fmt::format("Type \"{0}\" has not been initialized. Don't forget to call : `opack::init<{0}>(world)`", type_name_cstr<T>()).c_str());
#endif 
        auto e = world.entity().is_a<T>();
        internal::organize_entity<T>(e);
        return e;
    }

    template<typename T>
    Entity spawn(World& world, const char* name)
    {
#ifdef OPACK_ASSERTS
        ecs_assert(world.entity<T>().has(flecs::Prefab), ECS_INVALID_PARAMETER, fmt::format("Type \"{0}\" has not been initialized. Don't forget to call : `opack::init<{0}>(world)`", type_name_cstr<T>()).c_str());
#endif 
        auto e = world.entity(name).is_a<T>();
        internal::organize_entity<T>(e);
        return e;
    }
}
