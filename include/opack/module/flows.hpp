/*****************************************************************//**
 * \file   flows.hpp
 * \brief  Some ready to use flows for quicker setup.
 * 
 * \author Tristan
 * \date   September 2022
 *********************************************************************/
#pragma once

#include <opack/core.hpp>
#include <opack/operations/basic.hpp>
#include <opack/operations/influence_graph.hpp>
#include <opack/module/adl.hpp>

template<std::derived_from<opack::Flow> T, typename... Activities>
struct ActivityFlowBuilder : public opack::FlowBuilder<T>
{
	struct SuitableActions : opack::operations::Union<opack::Action_t> {};
	struct ActionSelection : opack::operations::SelectionByIGraph<SuitableActions> {};
	struct Act : opack::operations::All<opack::df<ActionSelection, typename ActionSelection::output>> {};
	
	ActivityFlowBuilder(opack::World& world)
		: opack::FlowBuilder<T>(world)
	{
		opack::operation<T, SuitableActions, ActionSelection, Act>(world);
		opack::default_impact<SuitableActions>(world,
			[](flecs::entity agent, typename SuitableActions::inputs& inputs)
			{
				(add_activity<Activities>(agent, inputs), ...);
				return opack::make_outputs<SuitableActions>();
			}
			);

		opack::default_impact<ActionSelection>(world,
			[](flecs::entity agent, typename ActionSelection::inputs& inputs)
			{				
				auto graph = ActionSelection::get_graph(inputs);
				for (auto& a : ActionSelection::get_choices(inputs))
				{
					graph.entry(a);
				}
				return opack::make_outputs<ActionSelection>();
			}
		);

		//opack::default_impact<Act>(world,
		//	[](flecs::entity agent, Act::inputs& inputs)
		//	{				
		//		auto action = std::get<opack::df<ActionSelection, ActionSelection::output>&>(inputs).value;
		//		if (action)
		//		{
		//			std::cout << "Doing : " << action.path() << "\n";
		//			// TODO opack::act<Hand>(agent, action);
		//		}
		//		return opack::make_outputs<Act>();
		//	}
		//);
	}

private:
	template<typename U>
	static void add_activity(flecs::entity agent, typename SuitableActions::inputs& inputs)
	{
		agent.each<U>([&inputs](flecs::entity target)
			{
				adl::potential_actions(target, SuitableActions::iterator(inputs));
			});
	}
};
