/*****************************************************************//**
 * \file   influence_graph.hpp
 * \brief  Strategy to choose one element from a set using influence graph
 * 
 * \author Tristan
 * \date   June 2022
 *********************************************************************/
#pragma once

#include <flecs.h>

#include <opack/core/api_types.hpp>
#include <opack/algorithm/influence_graph.hpp>

namespace opack::operations
{
	template<typename TOper, typename... Args>
	using selection_ig_t =
		opack::O<
		opack::Inputs<opack::df<TOper, typename TOper::container_t>, Args...>,
		opack::Outputs<typename TOper::type>,
		opack::Inputs<flecs::entity_view, typename opack::IPGraph<flecs::entity_view, typename TOper::type>::UNode>,
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
		using ig_t = opack::IPGraph<flecs::entity_view, type>;
		using graph = typename opack::IPGraph<flecs::entity_view, type>::UNode;
		using id = flecs::entity_view;

		static container& get_choices(typename parent_t::inputs& tuple)
		{
			return std::get<input_dataflow>(tuple).value;
		}

		static graph get_graph(typename parent_t::inputs& tuple)
		{
			return std::get<graph>(tuple);
		}

		template<typename T>
		struct Strategy : parent_t::template Strategy<T>
		{
			using parent_t::template Strategy<T>::Strategy;
			using output_dataflow = opack::df<T, output>;

			template<typename... Ts>
			typename T::operation_outputs compute(Ts&... args)
			{
				for (const auto impact : this->impacts)
				{
					auto inputs = opack::make_inputs<T>(args..., impact->behaviour, ig.scope(impact->behaviour));
					impact->func(this->agent, inputs);
				}
				auto result = ig.compute();
				this->agent.template set<T, ig_t>({ig});
				return std::make_tuple(result ? result.value() : flecs::entity::null());
			}

			ig_t ig{ };
		};
	};
}
