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
	template<typename TOper, typename... Args>
	using selection_ig_t =
		opack::O<
		opack::Inputs<opack::df<TOper, typename TOper::container_t>, Args...>,
		opack::Outputs<typename TOper::type>,
		opack::Inputs<flecs::entity_view, opack::InfluenceGraph<flecs::entity_view, typename TOper::type>&>,
		opack::Outputs<>
		>;

	template<typename TOper, typename... Args>
	struct SelectionByIGraph : selection_ig_t<TOper, Args...>
	{
		using parent_t = selection_ig_t<TOper, Args...>;
		using type = typename TOper::type;
		using output = typename TOper::type;
		using prev_operation = TOper;
		using container = std::vector<type>;
		using input_dataflow = opack::df<TOper, container>&;
		using ig_t = opack::InfluenceGraph<flecs::entity_view, type>;
		using graph = opack::InfluenceGraph<flecs::entity_view, type>&;
		using id = flecs::entity_view;

		static container& get_choices(typename parent_t::inputs& tuple)
		{
			return std::get<input_dataflow>(tuple).value;
		}

		static id get_influencer(typename parent_t::inputs& tuple)
		{
			return std::get<id>(tuple);
		}

		static graph get_graph(typename parent_t::inputs& tuple)
		{
			return std::get<graph>(tuple);
		}

		template<typename T>
		struct Strategy : parent_t::template Strategy<T>
		{
			using parent_t::template Strategy<T>::Strategy;

			template<typename... Ts>
			typename T::operation_outputs compute(Ts&... args)
			{
				ig_t ig{ };
				for (size_t i{ 0 }; i < this->impacts.size(); i++)
				{
					const auto& impact = *this->impacts[i];
					auto inputs = opack::make_inputs<T>(args..., impact.behaviour, ig);
					impact.func(this->agent, inputs);
				}
				auto result = ig.compute();
				std::cout << "Scores : \n";
				for (const auto [u, score] : ig.get_scores())
				{
					std::cout << *u << " = " << score << "\n";
				}
				std::cout << "Positive influence : \n";
				for (const auto [u, v] : ig.positive_influences())
				{
					std::cout << *u << " -- + -- >" << *v << "\n";
				}
				std::cout << "\n";
				return std::make_tuple(result == nullptr ? type() : *result);
			}
		};
	};


}
