/*****************************************************************//**
 * \file   adl.hpp
 * \brief  Header files to include all necessary headers
 * for using ACTIVITY-DL.
 * 
 * \author Tristan 
 * \date   September 2022
 *********************************************************************/
#pragma once

#include "adl/types.hpp"
#include "adl/components.hpp"
#include "adl/api.hpp"

namespace adl
{
    /** Import Activity-DL module in your world. */
    void import(opack::World& world);
}

