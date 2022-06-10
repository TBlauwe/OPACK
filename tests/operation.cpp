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

	opack::flow<_MyFlow_>(sim);
	auto a1 = opack::agent(sim).template add<_MyFlow_>().template add<Data>();
	auto a2 = opack::agent(sim).template add<_MyFlow_>().template add<Data>();

	struct _MyOp_ : opack::operations::All<Data> {};

	SUBCASE("Default behaviour")
	{
		opack::operation<_MyFlow_, _MyOp_>(sim);
		opack::impact<_MyOp_>::make(sim,
			[](flecs::entity e, typename _MyOp_::operation_inputs& i1, typename _MyOp_::impact_inputs& i2)
			{
				std::get<Data&>(i1).i++;
				return opack::make_output<_MyOp_>();
			}
		);
		sim.step(1.0f);
		sim.step();
		CHECK(a1.template get<Data>()->i == 1);
		CHECK(a2.template get<Data>()->i == 1);
		sim.step(1.0f);
		sim.step();
		CHECK(a1.template get<Data>()->i == 2);
		CHECK(a2.template get<Data>()->i == 2);
		sim.step(10.0f);
		sim.step();
		CHECK(a1.template get<Data>()->i == 3);
		CHECK(a2.template get<Data>()->i == 3);
	}
}

TEST_CASE_TEMPLATE_INVOKE(operation, EmptySim, SimpleSim);

TEST_SUITE_END();
