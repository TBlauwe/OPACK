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
	struct every : Strategy<TOutput, TInputs...>
	{
		TOutput operator()(flecs::entity agent, const opack::Impacts<TOutput, TInputs ...>& impacts, TInputs& ... args) const override
		{
			for (auto impact : impacts)
			{
				impact->func(agent, args ...);
			}
		}
	};

	/**
	 * Strategy with no output that will trigger every impact with adequate inputs.
	 */
	//template<typename TOutput, typename ... TInputs>
	//void random(flecs::entity agent, const opack::Impacts<TOutput, TInputs ...>& impacts, TInputs& ... args)
	//{
	//	impacts[rand() % impacts.size()]->func(agent, args ...);
	//}
}
