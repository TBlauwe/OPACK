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
    namespace _
    {
        template<typename T>
        void organize_entity(Entity& entity){}

        template<typename T>
        requires (HasRoot<T> && HasFolder<typename T::root_t>)
        void organize_entity(Entity& entity)
        {
#ifndef OPACK_OPTIMIZE
            entity.child_of<typename T::root_t::entities_folder_t>();
#endif
        }

        template<typename T>
        void organize_prefab(Entity& entity){}

        template<typename T>
        requires (HasRoot<T> && HasFolder<typename T::root_t>)
        void organize_prefab(Entity& entity)
        {
#ifndef OPACK_OPTIMIZE
            entity.child_of<typename T::root_t::prefabs_folder_t>();
#endif
        }

        template<HasFolder T>
        void create_module_entity(World& world)
        {
#ifndef OPACK_OPTIMIZE
	    world.entity<typename T::entities_folder_t>().add(flecs::Module);
	    world.entity<typename T::prefabs_folder_t>().add(flecs::Module);
#endif
        }
    }

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
    Entity entity(World& world)
    {
        return world.entity<T>();
    }

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
    Entity prefab(World& world)
    {
        auto prefab = world.prefab<T>();
        prefab.template child_of<world::prefabs>();
        return prefab;
    }

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
    requires HasRoot<T> && HasFolder<typename T::root_t>
    Entity prefab(World& world)
    {
        auto prefab = world.prefab<T>();
        prefab.template child_of<typename T::root_t::prefabs_folder_t>();
        return prefab;
    }
    
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
    Entity spawn(World& world)
    {
        auto e = world.entity().is_a<T>();
        _::organize_entity<T>(e);
        return e;
    }

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
    Entity spawn(World& world, const char * name)
    {
        auto e = world.entity(name).is_a<T>();
        _::organize_entity<T>(e);
        return e;
    }

    /** 
    @brief Initialize a sub-prefab according to its type and  correctly inherits its parent.

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
	Entity init(World& world)
	{
        auto e = prefab<T>(world);
        e.template is_a<typename T::base_t>();
        _::organize_prefab<T>(e);
        return e;
	}

    /** 
    @brief Calls @ref init for each passed types.
    Use this if you want to initialize multiples types at once and you
    do not care to customize each entity.
    You can always customize each entity later.
    Order does not matter.

    @tparam T Parameter pack of type you which to initialize.
    @param world explicit.

    Usage :

    @code{.cpp}
    opack::batch_init<MyType1, MyType2, ...>(world);
    @endcode
    */
	template<SubPrefab... T>
	void batch_init(World& world)
	{
        (init<T>(world), ...);
	}

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

}
