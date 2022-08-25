#include <doctest/doctest.h>
#include <opack/core.hpp>


void test_step(opack::World& world, size_t n = 1, float delta_time = 1.0f, float time_scale = 1.0f)
{
    float total_time{delta_time * time_scale * n};
    opack::time_scale(world, time_scale);
    if (n == 1)
        opack::step(world, delta_time);
    else
        opack::step_n(world, n, delta_time);
    CHECK(opack::tick(world) == n);
    CHECK(opack::delta_time(world) == time_scale * delta_time);
    CHECK(opack::time(world) >= total_time);
}

TEST_CASE("Simulation API")
{
    opack::World world = opack::create_world();
    REQUIRE(opack::tick(world) == 0);
    REQUIRE(opack::time(world) == 0.0f);

    SUBCASE("Configuration")
    {
        {
            float target_fps = 10.f;
            opack::target_fps(world, target_fps);
            CHECK(opack::target_fps(world) == target_fps);

            float time_scale = 0.5f;
            opack::time_scale(world, time_scale);
            CHECK(opack::time_scale(world) == time_scale);
        }
        {
            float target_fps = 1.f;
            opack::target_fps(world, target_fps);
            CHECK(opack::target_fps(world) == target_fps);

            float time_scale = 1.f;
            opack::time_scale(world, time_scale);
            CHECK(opack::time_scale(world) == time_scale);
        }
    }

    SUBCASE("Controls")
    {
        SUBCASE("Step")
        {
            test_step(world);
        }

        SUBCASE("Step_n")
        {
            test_step(world, 10);
        }

        SUBCASE("Step w/ delta_time")
        {
            test_step(world, 1, 0.5f);
        }

        SUBCASE("Step_n w/ delta_time")
        {
            test_step(world, 10, 0.5f);
        }

        SUBCASE("Step w/ delta_time w/time_scale 0.5")
        {
            test_step(world, 1, 0.5f, 0.5);
        }

        SUBCASE("Step_n w/ delta_time w/time_scale 0.5")
        {
            test_step(world, 10, 0.5f, 0.5);
        }

        SUBCASE("Step w/ delta_time w/time_scale 2")
        {
            test_step(world, 1, 2.f, 2.f);
        }

        SUBCASE("Step w/ delta_time w/time_scale 2")
        {
            test_step(world, 10, 2.f, 2.f);
        }
    }
}
