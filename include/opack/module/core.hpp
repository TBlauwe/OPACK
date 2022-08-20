/*****************************************************************//**
 * \file   core.hpp
 * \brief  Core modules
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <flecs.h>
#include <opack/core/types.hpp>
#include <opack/core/world.hpp>

namespace opack
{
	/**
	 * Core module to add necessary components for OPACK.
	 */
	struct concepts
	{
		concepts(flecs::world& world);
	};

	/**
	 * Core module to add necessary systems for OPACK.
	 */
	struct dynamics 
	{
		dynamics(flecs::world& world);
	};
}
