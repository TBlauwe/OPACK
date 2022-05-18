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

	auto task_1 = adl::task("a1", root);
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
	auto task_1 = adl::task("a1", root);

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

	// Define actions
	// ===============
	struct Action1 : opack::Action {};
	opack::register_action<Action1>(sim);

	struct Action2 : opack::Action {};
	opack::register_action<Action2>(sim);

	struct Action3 : opack::Action {};
	opack::register_action<Action3>(sim);

	SUBCASE("Check success")
	{
		std::vector<flecs::entity> actions{};

		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::AND, adl::TemporalConstructor::IND);
		auto a1 = adl::action<Action1>(root);

		SUBCASE("Fail because there are still potential actions") // Since no task have been finished successfuly
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("Fail because there are still potential actions") // Since task is not finished
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed activity") // Since task is finished but not satisfied
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.add<adl::Satisfied>();
		SUBCASE("Succeeded activity") // Since task is finished  and satisfied
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}
	}

	SUBCASE("IND AND")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::AND, adl::TemporalConstructor::IND);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		std::vector<flecs::entity> actions{};
		SUBCASE("All tasks")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("A2 & A3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("T3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(adl::has_task_in_progress(root));
			CHECK(!success);
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Finished but failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::has_task_in_progress(root));
			CHECK(!success);
		}

		a1.add<adl::Satisfied>();
		SUBCASE("Finished but failed 2")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::has_task_in_progress(root));
			CHECK(!success);
		}

		a2.add<adl::Satisfied>();
		SUBCASE("Finished but failed 3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::has_task_in_progress(root));
			CHECK(!success);
		}

		a3.add<adl::Satisfied>();
		SUBCASE("Finished and succeeded")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::has_task_in_progress(root));
			CHECK(success);
		}
	}

	SUBCASE("IND OR")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		std::vector<flecs::entity> actions{};
		SUBCASE("All tasks")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("A2 & A3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("T3")
		{
			adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(adl::has_task_in_progress(root));
			CHECK(!success);
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Finished but failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::has_task_in_progress(root));
			CHECK(!success);
		}

		a1.add<adl::Satisfied>();
		SUBCASE("Finished and succeeded 1")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::has_task_in_progress(root));
			CHECK(success);
		}

		a2.add<adl::Satisfied>();
		SUBCASE("Finished and succeeded 2")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::has_task_in_progress(root));
			CHECK(success);
		}

		a3.add<adl::Satisfied>();
		SUBCASE("Finished and succeeded 3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::has_task_in_progress(root));
			CHECK(success);
		}
	}

	SUBCASE("SEQ AND")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		std::vector<flecs::entity> actions{};
		SUBCASE("A1 & A2 & A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.add<adl::Satisfied>();
		SUBCASE("A2 & A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a2.add<adl::Satisfied>();
		SUBCASE("A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}
		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.add<adl::Satisfied>();

		SUBCASE("Success")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}
	}

	SUBCASE("SEQ OR")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		std::vector<flecs::entity> actions{};
		SUBCASE("A1 & A2 & A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==2);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.add<adl::Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.remove<adl::Satisfied>();
		SUBCASE("A2 & A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		SUBCASE("A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}
		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.add<adl::Satisfied>();

		SUBCASE("Success")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}
	}	
	
	SUBCASE("SEQ_ORD OR")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ_ORD);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		std::vector<flecs::entity> actions{};
		SUBCASE("A1")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.add<adl::Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.remove<adl::Satisfied>();
		SUBCASE("A2")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed - left only A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}
		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.add<adl::Satisfied>();

		SUBCASE("Success")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}
	}	

	SUBCASE("SEQ_ORD AND")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		std::vector<flecs::entity> actions{};
		SUBCASE("A1")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.add<adl::Satisfied>();
		SUBCASE("A2")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a2.add<adl::Satisfied>();
		SUBCASE("A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}
		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });

		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a3.add<adl::Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}
	}

	SUBCASE("ORD AND")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::AND, adl::TemporalConstructor::ORD);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		std::vector<flecs::entity> actions{};
		SUBCASE("A1")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("A2")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a2.add<adl::Satisfied>();
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.add<adl::Satisfied>();
		SUBCASE("A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a3.add<adl::Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}
	}

	SUBCASE("ORD OR")
	{
		auto root = adl::activity<Activity_A>(sim, adl::LogicalConstructor::OR, adl::TemporalConstructor::ORD);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		std::vector<flecs::entity> actions{};
		SUBCASE("A1")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("A2")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Unfinished A3")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==1);
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("Unfinished none")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(adl::has_task_in_progress(root));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a3.add<adl::Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size()==0);
			CHECK(success);
			CHECK(!adl::has_task_in_progress(root));
		}
	}
}

TEST_SUITE_END();
