#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/activity_dl.hpp>
#include <iostream>

TEST_SUITE_BEGIN("Module : Activity-DL");

TEST_CASE("Initialization")
{
	auto sim = opack::Simulation();
	auto module = sim.import<adl>();
	CHECK(sim.world.has<adl>());
}

struct Activity_A : adl::Activity {};
TEST_CASE("Basics")
{
	auto sim = opack::Simulation();
	sim.import<adl>();

	auto root = adl::activity<Activity_A>(sim);
	CHECK(adl::children_count(root) == 0);
	CHECK(adl::size(root) == 1);

	auto task_1 = adl::task("task_1", root);
	CHECK(adl::children_count(root) == 1);
	CHECK(adl::size(root) == 2);

	auto instance = adl::instantiate<Activity_A>(sim);
	CHECK(adl::children_count(instance) == 1);
	CHECK(adl::is_satisfied(instance));
}

struct Activity_B : adl::Activity {};
TEST_CASE("Composition")
{
	auto sim = opack::Simulation();
	sim.import<adl>();

	auto root = adl::activity<Activity_A>(sim);
	auto task_1 = adl::task("task_1", root);

	auto activity_b = adl::activity<Activity_B>(sim);
	auto instance = adl::compose<Activity_A>(activity_b);
	CHECK(adl::size(instance) == 2);
	CHECK(adl::size(activity_b) == 3);
}

TEST_SUITE_END();
