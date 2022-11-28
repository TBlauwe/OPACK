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
	struct AgentHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct AgentHandle : Handle
	{
		using Handle::Handle;
	};

	struct ArtefactHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct ArtefactHandle : Handle
	{
		using Handle::Handle;
	};

    /** 
    @brief Returns true if @c entity is entity identified by @c prefab.

    Usage :

    @code{.cpp}
    OPACK_ACTION(A); 
    auto e = opack::init<A>(world);
    opack::is<A>(e); // true
    @endcode
    */
    template<typename T>
    bool is(EntityView entity);

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
    bool is_a(EntityView prefab, EntityView entity);

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
    bool is_a(EntityView entity);

    inline void print(EntityView entity);

    /** Always return true. */
    inline bool always(EntityView) { return true; };

    /** Always return false. */
	inline bool never(EntityView) { return false; };

    /** Returns true iff @c agent has all of @c Ts. */
	template<typename... Ts>
	bool with(EntityView agent) { return (agent.has<Ts>() && ...); };

    /** Returns true iff @c agent has none of @c Ts. */
	template<typename... Ts>
	bool without(EntityView agent) { return (!agent.has<Ts>() && ...); };


    // --------------------------------------------------------------------------- 
    // Definition
    // --------------------------------------------------------------------------- 
    template<typename T>
    bool is(EntityView entity)
    {
		return entity == entity.world().id<T>();
    }

	inline bool is_a(EntityView prefab, EntityView entity)
	{
		return entity.has(flecs::IsA, prefab);
	}

	template<typename T>
	bool is_a(EntityView entity)
	{
		return is_a(entity.world().entity<T>(), entity);
	}

	inline void print(EntityView entity)
	{
		fmt::print("Entity : {}\n", entity.path());
	}
}

