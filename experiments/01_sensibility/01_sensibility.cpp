#include <vector>

#include <opack/core.hpp>
#include <opack/operations/basic.hpp>
#include <opack/operations/influence_graph.hpp>
#include <opack/module/simple_agent.hpp>
#include <matplot/matplot.h>
#include <color/color.hpp>
#include <algorithm>
#include <numeric>
#include <random>

template<typename T>
double percentage(const T& min, const T& max, const T& value)
{
    return (value - min) / (max - min);
}

uint8_t random_color()
{
    static std::random_device dev;
    static std::mt19937 rng(dev());
    static std::uniform_int_distribution<std::mt19937::result_type> dist(0,255); // distribution in range [1, 6]
    return dist(rng);
}

using rgb = color::rgb<uint8_t>;
struct Color
{
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};

    explicit operator rgb() const { return rgb{ r, g, b }; }

    friend auto operator<=>(const Color&, const Color&) = default;
};

struct Memory
{
    std::vector<Color> actions_done;
};

OPACK_ACTION(Action);
struct SuitableActions : opack::operations::Union<opack::Action_t> {};
struct ActionSelection : opack::operations::SelectionByIGraph<SuitableActions> {};
struct Act : opack::operations::All<opack::df<ActionSelection, opack::Action_t>> {};

OPACK_BEHAVIOUR(Lazy);
OPACK_BEHAVIOUR(Stressed);

using namespace matplot;

void generate_actions_sequence(opack::World& world, const char * filename)
{
	std::tuple<vector_2d, vector_2d, vector_2d> C;
	auto &[r, g, b] = C;

	std::vector<std::string> tick_labels{};
	world.filter<const Memory>().each([&r, &g, &b, &tick_labels](opack::Entity agent, const Memory& memory)
		{
			auto sub_r = vector_1d{};
			auto sub_g = vector_1d{};
			auto sub_b = vector_1d{};
			fmt::print("Agent has done : \n");
			for(auto& color : memory.actions_done)
			{
				fmt::print(" - ({},{},{})\n", color.r, color.g, color.b);
				sub_r.push_back(color.r);
				sub_g.push_back(color.g);
				sub_b.push_back(color.b);
			}
			r.push_back(sub_r);
			g.push_back(sub_g);
			b.push_back(sub_b);
			tick_labels.push_back(agent.name().c_str());
		}
	);
	image(C);
    grid(on);

	std::vector<double> ticks(opack::count_instance<simple::Agent>(world));
	std::iota(ticks.begin(), ticks.end(),1);
	yticks(ticks);
	yticklabels(tick_labels);
	show();
    save(filename);
}

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
    opack::on_action_begin<Action>(world, [](opack::Entity action)
    {
            auto agent = opack::initiator(action);
            if (agent.has<Memory>())
                agent.get_mut<Memory>()->actions_done.push_back(*action.get<Color>());
    		std::cout << action << " is being done\n";
    });

    world.plecs_from_file("configuration.flecs");

    // =========================================================================== 
    // Flow definition
    // =========================================================================== 
	opack::operation<simple::Flow, SuitableActions, ActionSelection, Act>(world);
    opack::default_impact<SuitableActions>(world,
        [](opack::Entity e, auto& inputs)
			{
				auto world = e.world();
                world.filter_builder<const Color>()
					.term(flecs::Prefab)
					.build()
        			.each([&inputs](opack::Entity action_prefab, const Color&)
                    {
                        SuitableActions::iterator(inputs) = opack::action(action_prefab);
                    });

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
    // Behaviour definition
    // =========================================================================== 
    opack::behaviour<Lazy>(world, opack::with<Lazy>);
    opack::impact<ActionSelection, Lazy>(world, 
        [](opack::Entity agent, ActionSelection::inputs& inputs)
        {
        	const auto id = ActionSelection::get_influencer(inputs);
        	auto& actions = ActionSelection::get_choices(inputs);
        	auto& graph = ActionSelection::get_graph(inputs);
			for (auto& a : actions)
			{
                auto c = rgb(*a.get<Color>());
                if(c == ::color::constant::black_t{}) 
					graph.positive_influence(id, a);
			}
			return opack::make_outputs<ActionSelection>();

        }
    );

    opack::behaviour<Stressed>(world, opack::with<Stressed>);
    opack::impact<ActionSelection, Stressed>(world, 
        [](opack::Entity agent, ActionSelection::inputs& inputs)
        {
        	const auto id = ActionSelection::get_influencer(inputs);
        	auto& actions = ActionSelection::get_choices(inputs);
        	auto& graph = ActionSelection::get_graph(inputs);
            const auto& actions_done = agent.get<Memory>()->actions_done;
			for (auto& a : actions)
			{
                auto color = *a.get<Color>();
                auto c = rgb(color);
                auto count = std::count_if(actions_done.begin(), actions_done.end(), 
                    [&](const Color& action_color)
                    {
                        return color == action_color;
                    }
                );
                if(color.r > 0  && count < 1) //Bug always one more ?
					graph.positive_influence(id, a);
                else if(color.r > 0  && count >= 1) 
					graph.negative_influence(id, a);
			}
			return opack::make_outputs<ActionSelection>();

        }
    );

    // =========================================================================== 
    // World population
    // =========================================================================== 
    world.plecs_from_file("configuration.flecs");

    opack::step_n(world, 10);
	//opack::run_with_webapp(world);

	generate_actions_sequence(world,"test");
    return 0;
}
