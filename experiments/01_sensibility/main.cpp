#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <opack/core.hpp>
#include <opack/operations/basic.hpp>
#include <opack/operations/influence_graph.hpp>
#include <opack/utils/ring_buffer.hpp>
#include <opack/module/simple_agent.hpp>
#include <matplot/matplot.h>
#include <color/color.hpp>

template<typename T>
double percentage(const T& min, const T& max, const T& value)
{
    return (value - min) / (max - min);
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

struct InspectMemory {};
struct InspectIGraph {};
struct InspectCurrentAction {};

struct Configuration
{
    size_t turns{ 10 };
};

OPACK_ACTION(Action);
struct SuitableActions : opack::operations::Union<opack::Action_t> {};
struct ActionSelection : opack::operations::SelectionByIGraph<SuitableActions> {};
struct Act : opack::operations::All<opack::df<ActionSelection, opack::Action_t>> {};

OPACK_BEHAVIOUR(Lazy);
OPACK_BEHAVIOUR(Stressed);
OPACK_BEHAVIOUR(Eager);

using namespace matplot;

void generate_actions_sequence(opack::World& world, const char * filename)
{
	std::tuple<vector_2d, vector_2d, vector_2d> C;
	auto &[r, g, b] = C;

	std::vector<std::string> tick_labels{};
	world.filter<const opack::LastActionPrefabs>().each([&r, &g, &b, &tick_labels](opack::Entity actuator, const opack::LastActionPrefabs& memory)
		{
			auto sub_r = vector_1d{};
			auto sub_g = vector_1d{};
			auto sub_b = vector_1d{};
			for(auto action : memory.previous_prefabs_done)
			{
                auto color = action.get<Color>();
				sub_r.push_back(color->r);
				sub_g.push_back(color->g);
				sub_b.push_back(color->b);
			}
			r.push_back(sub_r);
			g.push_back(sub_g);
			b.push_back(sub_b);
			tick_labels.push_back(actuator.parent().name().c_str());
		}
	);
	image(C);
    grid(on);

	std::vector<double> ticks(opack::count_instance<simple::Agent>(world));
	std::iota(ticks.begin(), ticks.end(),1);
	yticks(ticks);
	yticklabels(tick_labels);
	show();
    save(fmt::format("img/{}.tex", filename));
    save(fmt::format("img/{}.html", filename));
    save(fmt::format("img/{}.gif", filename));
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

    world.component<Configuration>()
        .member<size_t>("turns")
        ;
    world.add<Configuration>();

    world.system<const opack::LastActionPrefabs>()
        .term<InspectMemory>().parent()
        .kind(flecs::PostUpdate)
        .each([](opack::Entity actuator, const opack::LastActionPrefabs& memory)
            {
                fmt::print("[INSPECTION] - {} has done : [", actuator.parent().name());
                for(auto& action : memory.previous_prefabs_done)
                {
                    fmt::print("{} -", action.path());
                }
                fmt::print("]\n");
            });

    world.system()
        .kind(flecs::PreFrame)
        .iter([](flecs::iter& iter)
            {
                fmt::print(" ------------------- Turn : {} -------------------- \n", iter.world().tick());
            });

    world.system()
        .term<InspectCurrentAction>()
		.term(flecs::IsA).second<simple::Agent>()
        .kind(flecs::PostUpdate)
        .each([](opack::Entity agent)
            {
                auto action = opack::current_action<simple::Actuator>(agent);
                if(action)
					fmt::print("{} is doing \"{}\"\n", agent.name(), action.doc_name());
                else
					fmt::print("{} is doing nothing.\n", agent.name());
            });

    world.system<flecs::pair<ActionSelection, ActionSelection::ig_t>>()
        .term<InspectIGraph>()
		.term(flecs::IsA).second<simple::Agent>()
        .kind(flecs::PostUpdate)
        .each([](opack::Entity agent, ActionSelection::ig_t& ig)
            {
                fmt::print("[INSPECTION] - {} influence graph : \n", agent.name());
                ig.print([](flecs::entity_view e) {return e.name(); }, [](flecs::entity_view e) {return e.name(); });
           });

    world.system()
        .kind(flecs::PostFrame)
        .iter([](flecs::iter& iter)
            {
                fmt::print("\n");
            });

    opack::prefab<simple::Agent>(world).add<simple::Flow>();

    // Instantiate "Action" prefab
    opack::init<Action>(world).require<simple::Actuator>().add<Color>();

     // =========================================================================== 
    // Flow definition
    // =========================================================================== 
	opack::operation<simple::Flow, SuitableActions, ActionSelection, Act>(world);
    opack::default_impact<SuitableActions>(world,
        [](opack::Entity e, auto& inputs)
			{
                e.world().filter_builder<const Color>()
					.term(flecs::Prefab)
					.build()
        			.each([&inputs](opack::Entity action_prefab, const Color&)
                    {
                        SuitableActions::iterator(inputs) = action_prefab;
                    });

                return opack::make_outputs<SuitableActions>();	
			}
        );


	opack::default_impact<ActionSelection>(world,
		[](opack::Entity e, auto& inputs)
		{
			auto graph = ActionSelection::get_graph(inputs);
			for (auto& a : ActionSelection::get_choices(inputs))
			{
				graph.entry(a);
			}
			return opack::make_outputs<ActionSelection>();
		}
	);

    opack::default_impact<Act>(world,
        [](opack::Entity agent, auto& inputs)
        {
            auto action = std::get<opack::df<ActionSelection, opack::Action_t>&>(inputs).value;
            opack::act(agent, action);
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
            ActionSelection::each(inputs, [](auto& graph, const auto& action)
                {
                    auto c = rgb(*action.template get<Color>());
                    if (c == ::color::constant::black_t{})
                        graph.positive_influence(action);
                });
			return opack::make_outputs<ActionSelection>();

        }
    );

    opack::behaviour<Stressed>(world, opack::with<Stressed>);
    opack::impact<ActionSelection, Stressed>(world, 
        [](opack::Entity agent, ActionSelection::inputs& inputs)
        {
        	auto graph = ActionSelection::get_graph(inputs);
            const auto& actions_done = simple::get_actuator(agent).get<opack::LastActionPrefabs>()->previous_prefabs_done;
			for (auto& a : ActionSelection::get_choices(inputs))
			{
                auto color = *a.get<Color>();
                if(const auto count = std::count_if(actions_done.begin(), actions_done.end(), 
                                                    [&a](const flecs::entity_view action)
                                                    {
	                                                    return action == a;
                                                    }
                ); color.r > 0  && count < 2)
					graph.positive_influence(a);
                else if(color.r > 0  && count >= 2) 
					graph.negative_influence(a);
			}
			return opack::make_outputs<ActionSelection>();
        }
    );

    opack::behaviour<Eager>(world, opack::with<Eager>);
    opack::impact<ActionSelection, Eager>(world, 
        [](opack::Entity agent, ActionSelection::inputs& inputs)
        {
        	auto graph = ActionSelection::get_graph(inputs);
            const auto& actions_done = simple::get_actuator(agent).get<opack::LastActionPrefabs>();
			for (auto& a :  ActionSelection::get_choices(inputs))
			{
                auto color = *a.get<Color>();
                auto c = rgb(color);
                if (const auto has_done = actions_done->has_done(a); c != ::color::constant::black_t{} && !has_done)
					graph.positive_influence(a);
                else if (has_done)
					graph.negative_influence(a);
			}
			return opack::make_outputs<ActionSelection>();

        }
    );

    // =========================================================================== 
    // World population
    // =========================================================================== 
    world.plecs_from_file("configuration.flecs");

	opack::entity<simple::Actuator>(world)
		.track(world.get<Configuration>()->turns + 1);
    //opack::step_n(world, world.get<Configuration>()->turns + 1);

	opack::run_with_webapp(world);

	generate_actions_sequence(world,"test");
    return 0;
}
