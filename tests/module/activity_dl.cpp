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
	CHECK(adl::has_children(root));
	CHECK(adl::children_count(root) == 1);
	CHECK(adl::size(root) == 2);

	auto instance = adl::instantiate<Activity_A>(sim);
	CHECK(adl::children_count(instance) == 1);
	CHECK(!adl::is_satisfied(instance));
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

TEST_CASE("Reasonning")
{
	auto sim = opack::Simulation();
	sim.import<adl>();

	auto agent = opack::agent(sim);

	SUBCASE("IND AND")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::AND, adl::TemporalConstructor::IND);
		auto task_1 = adl::task("T1", root);
		auto task_2 = adl::task("T2", root);
		auto task_3 = adl::task("T3", root);

		std::vector<flecs::entity> actions{};
		SUBCASE("All tasks")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("Only T2 and T3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Only T2 and T3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		task_2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Only T3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		task_3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) == actions.end());
		}
	}

	SUBCASE("IND OR")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
		auto task_1 = adl::task("T1", root);
		auto task_2 = adl::task("T2", root);
		auto task_3 = adl::task("T3", root);

		std::vector<flecs::entity> actions{};
		SUBCASE("All tasks")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("Only T2 and T3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}
	}

	SUBCASE("SEQ AND")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ);
		auto task_1 = adl::task("T1", root);
		auto task_2 = adl::task("T2", root);
		auto task_3 = adl::task("T3", root);

		std::vector<flecs::entity> actions{};
		SUBCASE("All tasks")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}

		task_1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Only T2 and T3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}

		task_2.set<opack::End, opack::Timestamp>({ 0.0f });
		task_3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		task_3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}
	}

	SUBCASE("SEQ OR")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ);
		auto task_1 = adl::task("T1", root);
		auto task_2 = adl::task("T2", root);
		auto task_3 = adl::task("T3", root);

		std::vector<flecs::entity> actions{};
		SUBCASE("All tasks")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}

		task_1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}

	}	
	
	SUBCASE("SEQ AND")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
		auto task_1 = adl::task("T1", root);
		auto task_2 = adl::task("T2", root);
		auto task_3 = adl::task("T3", root);

		std::vector<flecs::entity> actions{};
		SUBCASE("Only T1")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) == actions.end());
		}

		task_1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}

		task_1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Only T2")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) == actions.end());
		}

		task_2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}

		task_2.set<opack::End, opack::Timestamp>({ 0.0f });
		task_3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("Only T3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), task_1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), task_3) != actions.end());
		}

		task_3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
		}
	}

}

TEST_SUITE_END();
