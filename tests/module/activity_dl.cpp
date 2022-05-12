#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/activity_dl.hpp>
#include <iostream>

TEST_SUITE_BEGIN("Module : Activity-DL");

TEST_CASE("Initialization")
{
	auto sim = opack::Simulation();
	auto module = sim.world.import<adl>();
	CHECK(sim.world.has<adl>());
}

struct Activity_A : adl::Activity {};
TEST_CASE("Basics")
{
	auto sim = opack::Simulation();
	sim.world.import<adl>();

	auto root = adl::activity<Activity_A>(sim);
	CHECK(adl::children_count(root) == 0);
	CHECK(adl::size(root) == 1);

	auto task_1 = adl::task("task_1", root);
	CHECK(adl::children_count(root) == 1);
	CHECK(adl::size(root) == 2);

	auto instance = adl::instantiate<Activity_A>(sim);
	CHECK(adl::children_count(instance) == 1);
	CHECK(adl::size(root) == 2);
}

struct Activity_B : adl::Activity {};
TEST_CASE("Composition")
{
	auto sim = opack::Simulation();
	sim.world.import<adl>();

	auto root = adl::activity<Activity_A>(sim);
	auto task_1 = adl::task("task_1", root);

	auto activity_b = adl::activity<Activity_B>(sim);
	auto instance = adl::compose<Activity_A>(activity_b);
	CHECK(adl::size(instance) == 2);
	CHECK(adl::size(activity_b) == 3);
}

//TEST_CASE("API")
//{
//	auto sim = opack::Simulation();
//	sim.world.import<adl>();
//
//	auto root = adl::task<Activity_A>(sim, "");
//	auto subroot_1 = adl::task<Activity_A>(sim, "Subroot_1", root);
//	auto subroot_2 = adl::task<Activity_A>(sim, "Subroot_2", root);
//	auto subroot_3 = adl::task<Activity_A>(sim, "Subroot_3", root);
//
//	SUBCASE("Creation")
//	{
//		CHECK(subroot_1.has<Activity_A>(root));
//		CHECK(adl::is_task_in<Activity_A>(subroot_1));
//		CHECK(adl::is_child_of<Activity_A>(subroot_1, root));
//		CHECK(adl::is_parent_of<Activity_A>(root, subroot_1));
//		CHECK(adl::parent_of<Activity_A>(subroot_1) == root);
//		CHECK(adl::children_count<Activity_A>(root) == 3);
//
//		CHECK(root.get_object<adl::Order>(0) == subroot_1);
//		CHECK(root.get_object<adl::Order>(1) == subroot_2);
//		CHECK(root.get_object<adl::Order>(2) == subroot_3);
//	}
//
//	SUBCASE("Removal")
//	{
//		auto subsubroot_1 = adl::task<Activity_A>(sim, "SubSubroot_1", subroot_1);
//		auto subsubroot_2 = adl::task<Activity_A>(sim, "SubSubroot_2", subroot_2);
//		adl::remove<Activity_A>(root);
//		CHECK(!root.has<Activity_A>());
//		CHECK(!adl::is_task_in<Activity_A>(root));
//		CHECK(adl::children_count<Activity_A>(root) == 0);
//		CHECK(adl::children_count<Activity_A>(subroot_1) == 0);
//		CHECK(adl::children_count<Activity_A>(subroot_2) == 0);
//
//		CHECK(!subroot_1.has<Activity_A>(flecs::Wildcard));
//		CHECK(!subroot_2.has<Activity_A>(flecs::Wildcard));
//		CHECK(!subroot_3.has<Activity_A>(flecs::Wildcard));
//		CHECK(!subsubroot_1.has<Activity_A>(flecs::Wildcard));
//		CHECK(!subsubroot_2.has<Activity_A>(flecs::Wildcard));
//		CHECK(!adl::is_task_in<Activity_A>(subroot_1));
//		CHECK(!adl::is_task_in<Activity_A>(subroot_2));
//		CHECK(!adl::is_task_in<Activity_A>(subroot_3));
//		CHECK(!adl::is_task_in<Activity_A>(subsubroot_1));
//		CHECK(!adl::is_task_in<Activity_A>(subsubroot_2));
//	}
//
//	SUBCASE("Reparenting")
//	{
//		auto new_root = adl::task<Activity_A>(sim, "New Root");
//		adl::parent<Activity_A>(root, new_root);
//		CHECK(adl::is_child_of<Activity_A>(root, new_root));
//		CHECK(adl::is_parent_of<Activity_A>(new_root, root));
//		CHECK(adl::children_count<Activity_A>(new_root) == 1);
//		CHECK(adl::children_count<Activity_A>(root) == 3);
//	}
//
//	SUBCASE("cloning")
//	{
//		auto model = adl::instantiate<Activity_A>(sim);
//		std::cout << model.to_json() << "\n";
//	}
//}
