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

namespace opack::operations
{
	template<typename... Args>
	using all_t =
		opack::O<
		opack::Inputs<Args...>,
		opack::Outputs<>,
		opack::Inputs<>,
		opack::Outputs<>
		>;

	template<typename... Args>
	struct All : all_t<Args...>
	{
		using type = all_t<Args...>;

		template<typename TOper>
		struct Strategy : type::template Strategy<TOper>
		{
			using type::template Strategy<TOper>::Strategy;

			typename TOper::operation_outputs compute(typename TOper::operation_inputs& args)
			{
				auto inputs = std::make_tuple();
				for (const auto impact : this->impacts)
				{
					impact->func(this->agent, args, inputs);
				}
				return std::make_tuple();
			};
		};
	};

	template<typename T, typename... Args>
	using join_t =
		opack::O<
		opack::Inputs<Args...>,
		opack::Outputs<std::vector<T>>,
		opack::Inputs<std::back_insert_iterator<std::vector<T>>>,
		opack::Outputs<>
		>;

	template<typename T, typename... Args>
	struct Join : join_t<T, Args...>
	{
		using type = join_t<T, Args...>;
		using container = std::vector<T>;
		using iterator = std::back_insert_iterator<container>;

		template<typename TOper>
		struct Strategy : type::template Strategy<TOper>
		{
			using type::template Strategy<TOper>::Strategy;

			typename TOper::operation_outputs compute(typename TOper::operation_inputs& args)
			{
				std::vector<T> container{};
				auto inputs = std::make_tuple(std::back_inserter(container));
				for (const auto impact : this->impacts)
				{
					impact->func(this->agent, args, inputs);
				}
				return std::make_tuple(container);
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
