/*****************************************************************//**
 * \file   basic.hpp
 * \brief  Defines basic strategy that can be used for operations
 * 
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <iterator> // Needed for MSVC
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

			template<typename... Ts>
			typename TOper::operation_outputs compute(Ts&... args)
			{
				auto inputs = opack::make_inputs<TOper>(args...);
				for (const auto impact : this->impacts)
				{
					impact->func(this->agent, inputs);
				}
				return std::make_tuple();
			};
		};
	};

	template<typename T, typename... Args>
	using union_t =
		opack::O<
		opack::Inputs<Args...>,
		opack::Outputs<std::vector<T>>,
		opack::Inputs<std::back_insert_iterator<std::vector<T>>>,
		opack::Outputs<>
		>;

	template<typename T, typename... Args>
	struct Union : union_t<T, Args...>
	{
		using parent_t = union_t<T, Args...>;
		using container_t = std::vector<T>;
		using type = T;
		using output = container_t;
		using iterator_t = std::back_insert_iterator<container_t>;

		static iterator_t iterator(typename parent_t::inputs& tuple)
		{
			return std::get<iterator_t>(tuple);
		}

		template<typename TOper>
		struct Strategy : parent_t::template Strategy<TOper>
		{
			using parent_t::template Strategy<TOper>::Strategy;

			template<typename... Ts>
			typename TOper::operation_outputs compute(Ts&... args)
			{
				std::vector<T> container{};
				auto inputs = opack::make_inputs<TOper>(args..., std::back_inserter(container));
				for (const auto impact : this->impacts)
				{
					impact->func(this->agent, inputs);
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
