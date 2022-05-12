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

TEST_CASE("API")
{
	auto sim = opack::Simulation();
	sim.world.import<adl>();
	struct Activity_A : adl::Activity {};
	struct Activity_B : adl::Activity {};

	SUBCASE("Activity Tree creation")
	{
		auto root = adl::task<Activity_A>(sim, "Root");
		CHECK(root.has<Activity_A>());
		CHECK(adl::is_task_in<Activity_A>(root));

		auto subroot_1 = adl::task<Activity_A>(sim, "Subroot_1", root);
		CHECK(subroot_1.has<Activity_A>(root));
		CHECK(adl::is_task_in<Activity_A>(subroot_1));
		CHECK(adl::is_child_of<Activity_A>(subroot_1, root));
		CHECK(adl::is_parent_of<Activity_A>(root, subroot_1));
		CHECK(adl::parent_of<Activity_A>(subroot_1) == root);

		auto subroot_2 = adl::task<Activity_A>(sim, "Subroot_2", root);
		auto subroot_3 = adl::task<Activity_A>(sim, "Subroot_3", root);
		adl::children_of<Activity_A>(root, [](flecs::entity entity) {std::cout << entity.name() << "\n"; });

		auto new_root = adl::task<Activity_A>(sim, "Root");
		adl::parent<Activity_A>(root, new_root);
		CHECK(adl::is_child_of<Activity_A>(root, new_root));
		CHECK(adl::is_parent_of<Activity_A>(new_root, root));
	}
}
