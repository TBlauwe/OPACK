/*****************************************************************//**
 * \file   influence_graph.hpp
 * \brief  Strategy to choose one element from a set using influence graph
 * 
 * \author Tristan
 * \date   June 2022
 *********************************************************************/
#pragma once

#include <flecs.h>
#include <opack/core/types.hpp>
#include <opack/algorithm/influence_graph.hpp>

namespace opack::strat
{
	/**
	 * Strategy with no output that will trigger every impact with adequate inputs.
	 */
	template<typename ... TInputs>
	void influence_graph(flecs::entity agent, Dataflow& dataflow, const opack::Impacts<void, TInputs ...>& impacts, TInputs& ... args)
	{
		opack::InfluenceGraph<opack::Impact<void, TInputs ...>, flecs::entity> ig { };

		for (auto impact : impacts)
		{
			impact->func(agent, dataflow, args ...);
		}
	}
}
