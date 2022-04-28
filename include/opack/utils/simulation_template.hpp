/*****************************************************************//**
 * \file   simulation_template.hpp
 * \brief  A simple struct that you can inherit to quickly build a simulation.
 * 
 * \author Tristan
 * \date   April 2022
 * 
 * Basic usage : 
 * \code{.cpp}
 * struct EmptySim : opack::SimulationTemplate
 * {
 *    EmptySim(int argc = 0, char * argv[] = nullptr) : opack::SimulationTemplate{"EmptySim", argc, argv}
 *    {
 *    }
 * };
 * \endcode
 *********************************************************************/
#pragma once

#include <opack/core/simulation.hpp>

namespace opack
{
	struct SimulationTemplate
	{
		/**
		 * Name should be similar to class name for consistency.
		 */
		SimulationTemplate(int argc = 0, char* argv[] = nullptr);
		~SimulationTemplate();

		void run();

		Simulation sim;
	};
}
