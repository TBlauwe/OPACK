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

namespace opack::operations
{
	template<typename T, typename TOper, typename... Args>
	using selection_ig_t =
		opack::O<
		opack::Inputs<opack::df<TOper, std::vector<T>>, Args...>,
		opack::Outputs<T>,
		opack::Inputs<flecs::entity_view, opack::InfluenceGraph<flecs::entity_view, T>&>,
		opack::Outputs<>
		>;

	template<typename T, typename TInput, typename... Args>
	struct SelectionByIGraph : selection_ig_t<T, TInput, Args...>
	{
		using parent_t = selection_ig_t<T, TInput, Args...>;
		using type = T;
		using prev_operation = TInput;
		using container = std::vector<T>;
		using input_dataflow = opack::df<TInput, container>&;
		using ig_t = opack::InfluenceGraph<flecs::entity_view, T>;
		using graph = opack::InfluenceGraph<flecs::entity_view, T>&;
		using id = flecs::entity_view;

		static container& get_choices(typename parent_t::impact_inputs& tuple)
		{
			return std::get<input_dataflow>(tuple).value;
		}

		static id get_influencer(typename parent_t::impact_inputs& tuple)
		{
			return std::get<id>(tuple);
		}

		static graph get_graph(typename parent_t::impact_inputs& tuple)
		{
			return std::get<graph>(tuple);
		}

		template<typename TOper>
		struct Strategy : parent_t::template Strategy<TOper>
		{
			using parent_t::template Strategy<TOper>::Strategy;

			template<typename... Ts>
			typename TOper::operation_outputs compute(Ts&... args)
			{
				ig_t ig{ };
				for (size_t i{ 0 }; i < this->impacts.size(); i++)
				{
					const auto& impact = *this->impacts[i];
					auto inputs = opack::make_input<TOper>(args..., impact.behaviour, ig);
					impact.func(this->agent, inputs);
				}
				auto result = ig.compute();
				return std::make_tuple(result == nullptr ? T() : *result);
			}
		};
	};


}
