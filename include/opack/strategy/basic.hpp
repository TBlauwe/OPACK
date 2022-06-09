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

	//template<typename TOper>
	//struct every : opack::Strategy<TOper, std::tuple<>, std::tuple<>>
	//{
	//	using opack::Strategy<TOper, std::tuple<>, std::tuple<>>::Strategy;

	//	typename TOper::operation_outputs compute(const typename TOper::operation_inputs& args)
	//	{
	//		auto result = typename TOper::operation_outputs();
	//		for (const auto impact : this->impacts)
	//		{
	//			result = impact->func(this->agent, args, this->inputs);
	//		}
	//		return result;
	//	};
	struct every : opack::Strategy<opack::Inputs<>, opack::Outputs<>>
	{
		using strategy_t = opack::Strategy<opack::Inputs<>, opack::Outputs<>>;

		template<typename TOper>
		struct Algorithm : strategy_t::Algorithm<TOper>
		{
			using strategy_t::Algorithm<TOper>::Algorithm;

			typename TOper::operation_outputs compute(typename TOper::operation_inputs& args)
			{
				for (const auto impact : this->impacts)
				{
					impact->func(this->agent, args, this->inputs);
				}
				return typename TOper::operation_outputs();
			};
		};
	};

//};

	struct accumulator : opack::Strategy<opack::Inputs<>, opack::Outputs<>>
	{
		using strategy_t = opack::Strategy<opack::Inputs<>, opack::Outputs<>>;

		template<typename TOper>
		struct Algorithm : strategy_t::Algorithm<TOper>
		{
			using strategy_t::Algorithm<TOper>::Algorithm;

			typename TOper::operation_outputs compute(typename TOper::operation_inputs& args)
			{
				auto result = typename TOper::operation_outputs();
				for (const auto impact : this->impacts)
				{
					auto impact_result = impact->func(this->agent, args, this->inputs);
					//(std::get<TOutput>(result).push_back() }), ...); // Should be set from strategy result
				}
				return result;
			};
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
