#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/operations/basic.hpp>
#include <opack/operations/influence_graph.hpp>

struct MyAgent : public opack::Agent {using base_t = opack::Agent;};
OPACK_FLOW(MyFlow);
OPACK_BEHAVIOUR(B1);
OPACK_BEHAVIOUR(B2);
OPACK_BEHAVIOUR(B3);
struct Action1 : public opack::Action {using base_t = opack::Action;};
OPACK_ACTION(Action2);
OPACK_ACTION(Action3);
OPACK_ACTUATOR(MyActuator);


struct Data { int i{ 0 }; };
struct State { int i{ 0 }; };
struct Event {};

TEST_CASE("Operation API Basics")
{
	auto world = opack::create_world();

    opack::init<MyActuator>(world);
    opack::init<Action1>(world);
    opack::init<Action2>(world);
    opack::init<Action3>(world);
    opack::init<MyAgent>(world)
        .add<MyFlow>()
        .override<Data>().override<State>();
	opack::add_actuator<MyActuator, MyAgent>(world);
	auto a1 = opack::spawn<MyAgent>(world, "a1");
	auto a2 = opack::spawn<MyAgent>(world, "a2").set<State>({1});

	// B1 is always active
	opack::behaviour<B1>(world, [](opack::Entity) {return true; });
	// B2 is always active when state == 1;
	opack::behaviour<B2, const State>(world, [](opack::Entity, const State& state) {return state.i == 1; });
	// B2 is never active;
	opack::behaviour<B3>(world, [](opack::Entity) {return false; });

    opack::step(world); // To activate behaviours
    CHECK(a1.template has<opack::HasBehaviour, B1>());
    CHECK(!a1.template has<opack::HasBehaviour, B2>());
    CHECK(!a1.template has<opack::HasBehaviour, B3>());
    CHECK(a2.template has<opack::HasBehaviour, B1>());
    CHECK(a2.template has<opack::HasBehaviour, B2>());
    CHECK(!a2.template has<opack::HasBehaviour, B3>());

    opack::flow<MyFlow>(world);

    SUBCASE("Operation ALL")
    {
        struct All : opack::operations::All<Data> {};
        opack::operation<MyFlow, All>(world);

        // Default impact for Op
        opack::default_impact<All>(world,
            [](opack::Entity e, All::inputs& i1)
            {
                std::get<Data&>(i1).i++;
                return opack::make_outputs<All>();
            }
        );

        opack::impact<All, B1>(world,
            [](opack::Entity e, All::inputs& i1)
            {
                std::get<Data&>(i1).i += 2;
                return opack::make_outputs<All>();
            }
        );

        opack::impact<All, B2>(world,
            [](opack::Entity e, All::inputs& i1)
            {
                std::get<Data&>(i1).i++;
                return opack::make_outputs<All>();
            }
        );

        opack::impact<All, B3>(world,
            [](opack::Entity e, All::inputs& i1)
            {
                std::get<Data&>(i1).i += 100;
                return opack::make_outputs<All>();
            }
        );

        CHECK(a1.get<Data>()->i == 0);
        CHECK(a2.get<Data>()->i == 0);
        opack::step(world, 1.0f);
        CHECK(a1.get<Data>()->i == 3);
        CHECK(a2.get<Data>()->i == 4);
        opack::step(world, 1.0f);
        CHECK(a1.get<Data>()->i == 6);
        CHECK(a2.get<Data>()->i == 8);
        opack::step(world, 10.0f);
        CHECK(a1.get<Data>()->i == 9);
        CHECK(a2.get<Data>()->i == 12);
    }

    SUBCASE("Operation Union")
    {
        struct Union : opack::operations::Union<int> {};
        opack::operation<MyFlow, Union>(world);
        opack::default_impact<Union>(world,
            [](opack::Entity e, typename Union::inputs& i1)
            {
                Union::iterator(i1) = 0;
                return opack::make_outputs<Union>();
            }
        );
        opack::impact<Union, B1>(world,
            [](opack::Entity e, typename Union::inputs& i1)
            {
                Union::iterator(i1) = 1;
                return opack::make_outputs<Union>();
            }
        );
        opack::impact<Union, B2>(world,
            [](opack::Entity e, typename Union::inputs& i1)
            {
                Union::iterator(i1) = 2;
                return opack::make_outputs<Union>();
            }
        );
        opack::impact<Union, B3>(world,
            [](opack::Entity e, typename Union::inputs& i1)
            {
                Union::iterator(i1) = 3;
                return opack::make_outputs<Union>();
            }
        );	
        
        opack::step(world, 1.0f);
        {
            auto v = opack::dataflow<Union, std::vector<int>>(a1);
            CHECK(std::find(v.begin(), v.end(), 0) != v.end());
            CHECK(std::find(v.begin(), v.end(), 1) != v.end());
            CHECK(std::find(v.begin(), v.end(), 2) == v.end());
            CHECK(std::find(v.begin(), v.end(), 3) == v.end());
        }

        {
            auto v = opack::dataflow<Union, std::vector<int>>(a2);
            CHECK(std::find(v.begin(), v.end(), 0) != v.end());
            CHECK(std::find(v.begin(), v.end(), 1) != v.end());
            CHECK(std::find(v.begin(), v.end(), 2) != v.end());
            CHECK(std::find(v.begin(), v.end(), 3) == v.end());
        }
    }

    SUBCASE("Operation ActionSelection")
    {
        struct Op1 : opack::operations::Union<opack::Action_t> {};
        struct Op2 : opack::operations::SelectionByIGraph<Op1, Data> {};
        struct Op3 : opack::operations::All<opack::df<Op2, opack::Action_t>> {};

        opack::operation<MyFlow, Op1, Op2, Op3>(world);

        opack::impact<Op1, B1>(world,
            [](opack::Entity e, typename Op1::inputs& i1)
            {
                Op1::iterator(i1) = opack::action<Action1>(e);
                return opack::make_outputs<Op1>();
            }
        );
        opack::impact<Op1, B2>(world,
            [](opack::Entity e, typename Op1::inputs& i1)
            {
                Op1::iterator(i1) = opack::action<Action2>(e);
                return opack::make_outputs<Op1>();
            }
        );
        opack::impact<Op1, B3>(world,
            [](opack::Entity e, typename Op1::inputs& i1)
            {
                Op1::iterator(i1) = opack::action<Action3>(e);
                return opack::make_outputs<Op1>();
            }
        );	

        opack::default_impact<Op2>(world,
            [](opack::Entity e, typename Op2::inputs& i1)
            {
                const auto id = Op2::get_influencer(i1);
                auto& actions = Op2::get_choices(i1);
                auto& graph = Op2::get_graph(i1);
                fmt::print("Choices for {} :\n", e);
                for (auto& a : actions)
                {
                    graph.entry(a);
                    fmt::print("-- {} \n", a);
                }
                return opack::make_outputs<Op2>();
            }
        );

        opack::default_impact<Op3>(world,
            [](opack::Entity e, typename Op3::inputs& i1)
            {
                auto action = std::get<opack::df<Op2, opack::Action_t>&>(i1).value;
                fmt::print("Choosed {} \n", action);
                opack::act<MyActuator>(e, action);
                return opack::make_outputs<Op3>();
            }
        );
        
        opack::run_with_webapp(world);
        {
            auto action = opack::dataflow<Op2, opack::Action_t>(a1);
            CHECK(action.template has<Action1>());
        }
    }
}

TEST_CASE("API Flow w/ conditions")
{
	auto world = opack::create_world();

	opack::init<MyAgent>(world).add<MyFlow>().override<Data>().override<State>();
	auto a1 = opack::spawn<MyAgent>(world, "a1");
	auto a2 = opack::spawn<MyAgent>(world, "a2").set<State>({1});

	opack::FlowBuilder<MyFlow>(world).interval(2.0f).template has<Event>().build();

	// B1 is always active
	opack::behaviour<B1>(world, [](opack::Entity) {return true; });
	// B2 is always active when state == 1;
	opack::behaviour<B2, const State>(world, [](opack::Entity, const State& state) {return state.i == 1; });
	// B2 is never active;
	opack::behaviour<B3>(world, [](opack::Entity) {return false; });

    opack::step(world); // To activate behaviours
    CHECK(a1.template has<opack::HasBehaviour, B1>());
    CHECK(!a1.template has<opack::HasBehaviour, B2>());
    CHECK(!a1.template has<opack::HasBehaviour, B3>());
    CHECK(a2.template has<opack::HasBehaviour, B1>());
    CHECK(a2.template has<opack::HasBehaviour, B2>());
    CHECK(!a2.template has<opack::HasBehaviour, B3>());

	SUBCASE("Operation ALL")
	{
		struct Op : opack::operations::All<Data> {};
		opack::operation<MyFlow, Op>(world);

		// Default impact for Op
		opack::impact<Op, opack::Behaviour>(world,
			[](opack::Entity e, typename Op::inputs& i1)
			{
				std::get<Data&>(i1).i++;
				return opack::make_outputs<Op>();
			}
		);

		opack::impact<Op, B1>(world,
			[](opack::Entity e, typename Op::inputs& i1)
			{
				std::get<Data&>(i1).i += 2;
				return opack::make_outputs<Op>();
			}
		);

		opack::impact<Op, B2>(world,
			[](opack::Entity e, typename Op::inputs& i1)
			{
				std::get<Data&>(i1).i++;
				return opack::make_outputs<Op>();
			}
		);

		opack::impact<Op, B3>(world,
			[](opack::Entity e, typename Op::inputs& i1)
			{
				std::get<Data&>(i1).i += 100;
				return opack::make_outputs<Op>();
			}
		);

		CHECK(a1.template get<Data>()->i == 0);
		CHECK(a2.template get<Data>()->i == 0);
		opack::step(world, 2.0f);
		// No one has "Event" tag
		CHECK(a1.template get<Data>()->i == 0);
		CHECK(a2.template get<Data>()->i == 0);

		// Only a1 has it
		a1.template add<Event>();
		opack::step(world, 2.0f);
		CHECK(a1.template get<Data>()->i == 3);
		CHECK(a2.template get<Data>()->i == 0);

		// Only a2 has it
		a1.template remove<Event>();
		a2.template add<Event>();
		opack::step(world, 2.0f);
		CHECK(a1.template get<Data>()->i == 3);
		CHECK(a2.template get<Data>()->i == 4);

		// Both have it
		a1.template add<Event>();
		opack::step(world, 2.0f);
		CHECK(a1.template get<Data>()->i == 6);
		CHECK(a2.template get<Data>()->i == 8);
		opack::step(world, 10.0f);
		CHECK(a1.template get<Data>()->i == 9);
		CHECK(a2.template get<Data>()->i == 12);
	}
}
