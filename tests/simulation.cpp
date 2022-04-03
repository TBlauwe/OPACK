#include <doctest/doctest.h>
#include <opack/core/simulation.hpp>

TEST_SUITE_BEGIN("Simulation");

TEST_CASE("Basics")
{
	auto sim = opack::Simulation();
	REQUIRE(sim.tick() == 0);

	SUBCASE("Controls - setting")
	{
		float value = 10.f;
		sim.target_fps(value);
		CHECK(sim.target_fps() == value);

		value = 60.f;
		sim.target_fps(value);
		CHECK(sim.target_fps() == value);

		value = 0.5f;
		sim.time_scale(value);
		CHECK(sim.time_scale() == value);

		value = 1.0f;
		sim.time_scale(value);
		CHECK(sim.time_scale() == value);
	}

	SUBCASE("Controls - steps")
	{
		sim.step();
		CHECK(sim.tick() == 1);
		sim.step_n(10);
		CHECK(sim.tick() == 11);

		float delta_time{ 1.0f };
		float total_time{ 0.0f };
		sim.step(delta_time);
		CHECK(sim.delta_time() == delta_time);
		total_time += delta_time;
		CHECK(sim.time() >= total_time);

		delta_time = 3.0f;
		sim.step(delta_time);
		CHECK(sim.delta_time() == delta_time);
		total_time += delta_time;
		CHECK(sim.time() >= total_time);

		delta_time = 1.0f;
		sim.step_n(10, 1.0f);
		CHECK(sim.tick() == 23);
		CHECK(sim.delta_time() == delta_time);
		total_time += 10*delta_time;
		CHECK(sim.time() >= total_time);

	}

	SUBCASE("Controls - shutdown")
	{
		sim.stop();
	}
}

TEST_SUITE_END();
