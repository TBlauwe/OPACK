#include <vector>

#include <opack/core.hpp>
#include <opack/operations/basic.hpp>
#include <opack/operations/influence_graph.hpp>
#include <opack/module/simple_agent.hpp>
#include <matplot/matplot.h>
#include <color/color.hpp>

struct Color
{
    uint8_t r{ 0 };
    uint8_t g{ 0 };
    uint8_t b{ 0 };
};

struct Memory
{
    std::vector<Color> actions_done;
};

OPACK_ACTION(Action);
struct SuitableActions : opack::operations::Union<opack::Action_t> {};
struct ActionSelection : opack::operations::SelectionByIGraph<SuitableActions> {};
struct Act : opack::operations::All<opack::df<ActionSelection, opack::Action_t>> {};

using namespace matplot;

int main()
{

    // =========================================================================== 
    // Parameters
    // =========================================================================== 
    
    // =========================================================================== 
    // World definition
    // =========================================================================== 
	auto world = opack::create_world();
	world.import<simple>();
    world.component<Color>()
        .member<uint8_t>("r")
        .member<uint8_t>("g")
        .member<uint8_t>("b")
        ;

    opack::prefab<simple::Agent>(world).add<Memory>().add<simple::Flow>();

    // Instantiate "Action" prefab
    opack::init<Action>(world).require<simple::Actuator>().add<Color>();
    opack::on_action_begin<Action>(world, [](opack::Entity action) {std::cout << action << " is being done\n"; });

    world.plecs_from_file("configuration.flecs");

    // =========================================================================== 
    // Flow definition
    // =========================================================================== 
	opack::operation<simple::Flow, SuitableActions, ActionSelection, Act>(world);
    opack::default_impact<SuitableActions>(world,
        [](opack::Entity e, auto& inputs)
			{
        		SuitableActions::iterator(inputs) = opack::action<Action>(e);
                return opack::make_outputs<SuitableActions>();	
			}
        );


	opack::default_impact<ActionSelection>(world,
		[](opack::Entity e, auto& inputs)
		{
			const auto id = ActionSelection::get_influencer(inputs);
			auto& actions = ActionSelection::get_choices(inputs);
			auto& graph = ActionSelection::get_graph(inputs);
			for (auto& a : actions)
			{
				graph.entry(a);
			}
			return opack::make_outputs<ActionSelection>();
		}
	);

    opack::default_impact<Act>(world,
        [](opack::Entity e, auto& inputs)
        {
            auto action = std::get<opack::df<ActionSelection, opack::Action_t>&>(inputs).value;
            opack::act(e, action);
            return opack::make_outputs<Act>();
        }
    );

    // =========================================================================== 
    // World population
    // =========================================================================== 
	opack::spawn<simple::Agent>(world);

    std::vector<std::vector<double>> C = {
        {0, 2, 4, 6}, {8, 10, 12, 14}, {16, 18, 20, 22}};
    image(C, false);
    colorbar();
	opack::run_with_webapp(world);
    //opack::step_n(world, 10);
    return 0;
}
