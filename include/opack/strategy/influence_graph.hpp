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
	template<typename TInputs, typename TOutputs, typename TOtherInputs>
	struct influence_graph;                     
 
	template
	<
		template<typename...> typename TInputs, typename... TInput, 
		template<typename...> typename TOutputs, typename... TOutput,
		template<typename...> typename TOtherInputs, typename... TOtherInput
	>
	struct influence_graph<TInputs<TInput...>, TOutputs<TOutput...>, TOtherInputs<TOtherInput...> >
	{
		using inputs_t = std::tuple<TInput...>;
		using outputs_t = std::tuple<TOutput...>;
		using other_inputs_t = std::tuple<TOtherInput...>;
		using impact_t = opack::Impact<inputs_t, outputs_t, other_inputs_t>;

		static outputs_t run(flecs::entity agent, const std::vector<const impact_t*>& impacts, TInput& ... args)
		{
			auto result = std::tuple<TOutput...>();
			opack::InfluenceGraph<impact_t, flecs::entity> ig { };
			for (const auto impact : impacts)
			{
				result = impact->func(agent, args...);
			}
			return result;
		};
	};


}
