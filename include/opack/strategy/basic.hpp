/*****************************************************************//**
 * \file   basic.hpp
 * \brief  Defines basic strategy that can be used for operations
 * 
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <flecs.h>
#include <opack/core/types.hpp>

namespace opack::strat
{

	/**
	 * Strategy with no output that will trigger every impact with adequate inputs.
	 */
	template<typename TOutput, typename ... TInputs>
	struct every
	{
		TOutput operator()(flecs::entity agent, const opack::Impacts<TOutput, TInputs ...>& impacts, TInputs& ... args) const
		{
			for (auto impact : impacts)
			{
				impact->operator()(agent, args ...);
			}
		}
	};

	/**
	 * Strategy with no output that will trigger every impact with adequate inputs.
	 */
	template<typename TOutput, typename ... TInputs>
	struct random 
	{
		TOutput operator()(flecs::entity agent, const opack::Impacts<TOutput, TInputs ...>& impacts, TInputs& ... args) const
		{
			impacts[rand() % impacts.size()]->operator()(agent, args ...);
		}
	};
}
