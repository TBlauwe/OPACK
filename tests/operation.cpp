#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/examples/all.hpp>

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
		opack::impact<Op>::make(sim,
			[](flecs::entity e, typename Op::operation_inputs& i1, typename Op::impact_inputs& i2)
			{
				std::get<Data&>(i1).i++;
				return opack::make_output<Op>();
			}
		);

		opack::impact<Op, _B1_>::make(sim,
			[](flecs::entity e, typename Op::operation_inputs& i1, typename Op::impact_inputs& i2)
			{
				std::get<Data&>(i1).i += 2;
				return opack::make_output<Op>();
			}
		);

		opack::impact<Op, _B2_>::make(sim,
			[](flecs::entity e, typename Op::operation_inputs& i1, typename Op::impact_inputs& i2)
			{
				std::get<Data&>(i1).i++;
				return opack::make_output<Op>();
			}
		);

		opack::impact<Op, _B3_>::make(sim,
			[](flecs::entity e, typename Op::operation_inputs& i1, typename Op::impact_inputs& i2)
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
		opack::impact<Op>::make(sim,
			[](flecs::entity e, typename Op::operation_inputs& i1, typename Op::impact_inputs& i2)
			{
				std::get<typename Op::iterator>(i2) = 0;
				return opack::make_output<Op>();
			}
		);
		opack::impact<Op, _B1_>::make(sim,
			[](flecs::entity e, typename Op::operation_inputs& i1, typename Op::impact_inputs& i2)
			{
				std::get<typename Op::iterator>(i2) = 1;
				return opack::make_output<Op>();
			}
		);
		opack::impact<Op, _B2_>::make(sim,
			[](flecs::entity e, typename Op::operation_inputs& i1, typename Op::impact_inputs& i2)
			{
				std::get<typename Op::iterator>(i2) = 2;
				return opack::make_output<Op>();
			}
		);
		opack::impact<Op, _B3_>::make(sim,
			[](flecs::entity e, typename Op::operation_inputs& i1, typename Op::impact_inputs& i2)
			{
				std::get<typename Op::iterator>(i2) = 3;
				return opack::make_output<Op>();
			}
		);	
		
		sim.step(1.0f);
		sim.step();

		{
			auto v = a1.template get<opack::df<Op, std::vector<int>>>()->value;
			CHECK(std::find(v.begin(), v.end(), 0) != v.end());
			CHECK(std::find(v.begin(), v.end(), 1) != v.end());
			CHECK(std::find(v.begin(), v.end(), 2) == v.end());
			CHECK(std::find(v.begin(), v.end(), 3) == v.end());
		}

		{
			auto v = a2.template get<opack::df<Op, std::vector<int>>>()->value;
			CHECK(std::find(v.begin(), v.end(), 0) != v.end());
			CHECK(std::find(v.begin(), v.end(), 1) != v.end());
			CHECK(std::find(v.begin(), v.end(), 2) != v.end());
			CHECK(std::find(v.begin(), v.end(), 3) == v.end());
		}
	}
}

TEST_CASE_TEMPLATE_INVOKE(operation, EmptySim, SimpleSim);

TEST_SUITE_END();
