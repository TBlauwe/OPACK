#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/examples/all.hpp>
#include <opack/operations/influence_graph.hpp>

TEST_SUITE_BEGIN("Operation");

TEST_CASE_TEMPLATE_DEFINE("Simulation construction", T, operation)
{
	auto sim = T();

	REQUIRE(sim.world.template has<opack::concepts>());
	REQUIRE(sim.world.template has<opack::dynamics>());

	struct _MyFlow_ : opack::Flow{};
	struct Data { int i{ 0 }; };
	struct State { int i{ 0 }; };

	opack::flow<_MyFlow_>(sim);
	auto a1 = opack::agent(sim).template add<_MyFlow_>().template add<Data>().template add<State>();
	auto a2 = opack::agent(sim).template add<_MyFlow_>().template add<Data>().template set<State>({1});

	struct _B1_	: opack::Behaviour {};
	struct _B2_	: opack::Behaviour {};
	struct _B3_	: opack::Behaviour {};

	// B1 is always active
	opack::behaviour<_B1_>(sim, [](flecs::entity) {return true; });
	// B2 is always active when state == 1;
	opack::behaviour<_B2_, const State>(sim, [](flecs::entity, const State& state) {return state.i == 1; });
	// B2 is never active;
	opack::behaviour<_B3_>(sim, [](flecs::entity) {return false; });

	SUBCASE("Behaviours")
	{
		sim.step(); // To activate behaviours
		CHECK(a1.template has<opack::Active, _B1_>());
		CHECK(!a1.template has<opack::Active, _B2_>());
		CHECK(!a1.template has<opack::Active, _B3_>());
		CHECK(a2.template has<opack::Active, _B1_>());
		CHECK(a2.template has<opack::Active, _B2_>());
		CHECK(!a2.template has<opack::Active, _B3_>());
	}

	SUBCASE("Operation ALL")
	{
		struct Op : opack::operations::All<Data> {};
		opack::operation<_MyFlow_, Op>(sim);

		// Default impact for Op
		opack::impact<Op, opack::Behaviour>(sim,
			[](flecs::entity e, typename Op::impact_inputs& i1)
			{
				std::get<Data&>(i1).i++;
				return opack::make_output<Op>();
			}
		);

		opack::impact<Op, _B1_>(sim,
			[](flecs::entity e, typename Op::impact_inputs& i1)
			{
				std::get<Data&>(i1).i += 2;
				return opack::make_output<Op>();
			}
		);

		opack::impact<Op, _B2_>(sim,
			[](flecs::entity e, typename Op::impact_inputs& i1)
			{
				std::get<Data&>(i1).i++;
				return opack::make_output<Op>();
			}
		);

		opack::impact<Op, _B3_>(sim,
			[](flecs::entity e, typename Op::impact_inputs& i1)
			{
				std::get<Data&>(i1).i += 100;
				return opack::make_output<Op>();
			}
		);

		CHECK(a1.template get<Data>()->i == 0);
		CHECK(a2.template get<Data>()->i == 0);
		sim.step(1.0f);
		sim.step();
		CHECK(a1.template get<Data>()->i == 3);
		CHECK(a2.template get<Data>()->i == 4);
		sim.step(1.0f);
		sim.step();
		CHECK(a1.template get<Data>()->i == 6);
		CHECK(a2.template get<Data>()->i == 8);
		sim.step(10.0f);
		sim.step();
		CHECK(a1.template get<Data>()->i == 9);
		CHECK(a2.template get<Data>()->i == 12);
	}

	SUBCASE("Operation Join")
	{
		struct Op : opack::operations::Join<int> {};
		opack::operation<_MyFlow_, Op>(sim);
		opack::default_impact<Op>(sim,
			[](flecs::entity e, typename Op::impact_inputs& i1)
			{
				Op::iterator(i1) = 0;
				return opack::make_output<Op>();
			}
		);
		opack::impact<Op, _B1_>(sim,
			[](flecs::entity e, typename Op::impact_inputs& i1)
			{
				Op::iterator(i1) = 1;
				return opack::make_output<Op>();
			}
		);
		opack::impact<Op, _B2_>(sim,
			[](flecs::entity e, typename Op::impact_inputs& i1)
			{
				Op::iterator(i1) = 2;
				return opack::make_output<Op>();
			}
		);
		opack::impact<Op, _B3_>(sim,
			[](flecs::entity e, typename Op::impact_inputs& i1)
			{
				Op::iterator(i1) = 3;
				return opack::make_output<Op>();
			}
		);	
		
		sim.step(1.0f);
		sim.step();

		{
			auto v = opack::dataflow<Op, std::vector<int>>(a1);
			CHECK(std::find(v.begin(), v.end(), 0) != v.end());
			CHECK(std::find(v.begin(), v.end(), 1) != v.end());
			CHECK(std::find(v.begin(), v.end(), 2) == v.end());
			CHECK(std::find(v.begin(), v.end(), 3) == v.end());
		}

		{
			auto v = opack::dataflow<Op, std::vector<int>>(a2);
			CHECK(std::find(v.begin(), v.end(), 0) != v.end());
			CHECK(std::find(v.begin(), v.end(), 1) != v.end());
			CHECK(std::find(v.begin(), v.end(), 2) != v.end());
			CHECK(std::find(v.begin(), v.end(), 3) == v.end());
		}
	}

	SUBCASE("Operation Join")
	{
		struct Op1 : opack::operations::Join<opack::Action_t> {};
		struct Op2 : opack::operations::SelectionByIGraph<opack::Action_t, Op1, Data> {};
		struct Op3 : opack::operations::All<opack::df<Op2, opack::Action_t>> {};

		struct Action1 : opack::Action {};
		struct Action2 : opack::Action {};
		struct Action3 : opack::Action {};
		struct Actuator : opack::Actuator {};

		opack::reg_n<Action1, Action2, Action3, Actuator>(sim);

		opack::operation<_MyFlow_, Op1, Op2, Op3>(sim);

		opack::default_impact<Op1>(sim,
			[](flecs::entity e, typename Op1::impact_inputs& i1)
			{
				return opack::make_output<Op1>();
			}
		);
		opack::impact<Op1, _B1_>(sim,
			[](flecs::entity e, typename Op1::impact_inputs& i1)
			{
				Op1::iterator(i1) = opack::action<Action1>(e);
				return opack::make_output<Op1>();
			}
		);
		opack::impact<Op1, _B2_>(sim,
			[](flecs::entity e, typename Op1::impact_inputs& i1)
			{
				Op1::iterator(i1) = opack::action<Action2>(e);
				return opack::make_output<Op1>();
			}
		);
		opack::impact<Op1, _B3_>(sim,
			[](flecs::entity e, typename Op1::impact_inputs& i1)
			{
				Op1::iterator(i1) = opack::action<Action3>(e);
				return opack::make_output<Op1>();
			}
		);	

		opack::default_impact<Op2>(sim,
			[](flecs::entity e, typename Op2::impact_inputs& i1)
			{
				const auto id = Op2::get_influencer(i1);
				auto& actions = Op2::get_choices(i1);
				auto& graph = Op2::get_graph(i1);
				for (auto& a : actions)
				{
					graph.entry(a);
					//if(a.template has<Action1>())
				}
				return opack::make_output<Op2>();
			}
		);

		opack::default_impact<Op3>(sim,
			[](flecs::entity e, typename Op3::impact_inputs& i1)
			{
				auto action = std::get<opack::df<Op2, opack::Action_t>&>(i1).value;
				opack::act<Actuator>(e, action);
				return opack::make_output<Op3>();
			}
		);
		
		sim.step(1.0f);
		sim.step();
		{
			auto action = opack::dataflow<Op2, opack::Action_t>(a1);
			CHECK(action.template has<Action1>());
		}
	}
}

TEST_CASE_TEMPLATE_INVOKE(operation, EmptySim, SimpleSim);

TEST_SUITE_END();
