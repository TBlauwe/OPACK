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

	template<typename TInputs, typename TOutputs, typename TOtherInputs>
	struct every;                     
 
	template
	<
		template<typename...> typename TInputs, typename... TInput, 
		template<typename...> typename TOutputs, typename... TOutput,
		template<typename...> typename TOtherInputs, typename... TOtherInput
	>
	struct every<TInputs<TInput...>, TOutputs<TOutput...>, TOtherInputs<TOtherInput...> >
	{
		using inputs = std::tuple<TInput...>;
		using other_inputs = std::tuple<TOtherInput...>;
		using outputs = std::tuple<TOutput...>;

		static outputs run(flecs::entity agent, const std::vector<const Impact<inputs, outputs, other_inputs>*>& impacts, TInput& ... args)
		{
			std::cout << "-- every\n";
			for (const auto impact : impacts)
			{
				impact->func(agent, args...);
			}
			return std::make_tuple<TOutput...>();
		};
	};

	/**
	 * Strategy with no output that will trigger every impact with adequate inputs.
	 */
	//template<typename TOutput, typename ... TInputs>
	//struct random 
	//{
	//	TOutput operator()(flecs::entity agent, const opack::Impacts<TOutput, TInputs ...>& impacts, TInputs& ... args) const
	//	{
	//		impacts[rand() % impacts.size()]->operator()(agent, args ...);
	//	}
	//};
}
