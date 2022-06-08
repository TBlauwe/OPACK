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
	opack::flow<_MyFlow_>(sim);
	opack::agent(sim).template add<_MyFlow_>();
	opack::agent(sim).template add<_MyFlow_>();

	struct _MyOp_ : opack::O<opack::Inputs<>, opack::Outputs<>> {};

	SUBCASE("Default behaviour")
	{
		bool called{ false };
		opack::operation<_MyFlow_, _MyOp_>::template make<opack::strat::every>(sim);
		opack::impact<_MyOp_>::make(sim,
			[&called](flecs::entity e)
			{
				called = true;
				return opack::make_output<_MyOp_>();
			}
		);
		sim.step();
		CHECK(called);
	}
}

TEST_CASE_TEMPLATE_INVOKE(operation, EmptySim, SimpleSim);

TEST_SUITE_END();
