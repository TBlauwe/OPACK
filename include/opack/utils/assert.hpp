/*****************************************************************//**
 * \file   assert.hpp
 * \brief  API used only if in debug mode to provide more information
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once


#include <flecs.h>
#ifdef OPACK_DEBUG
#include <fmt/compile.h>
#include <opack/utils/type_name.hpp>
#endif

namespace opack::_
{
    template<typename T>
    void check_type(flecs::world& world)
    {
#ifdef OPACK_DEBUG
        ecs_assert(world.entity<T>().has(flecs::Prefab), ECS_INVALID_PARAMETER, 
            fmt::format(FMT_COMPILE("Unregistered type : {}"), type_name<T>()).c_str());
#endif
    }
}

