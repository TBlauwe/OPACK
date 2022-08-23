/*****************************************************************//**
 * \file   common.hpp
 * \brief  Common API
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <opack/core/types.hpp>

namespace opack
{
    /**
     *@brief Add a component
     *
     *@tparam T Type must be default constructable.
     *
    Usage :
    @code{.cpp}
    struct A {}; // Some struct that is default constructible.
    opack::World world;
    opack::Entity entity = opack::entity(world);
    opack::add<A>(world);
    opack::add<A>(entity);
    opack::is_a<A>(e); // true
    @endcode
     */
    template<DefaultConstructible T, Composable<T> U>
    void add(U& u)
    {
        u.template add<T>();
    }
}
