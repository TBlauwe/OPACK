#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/activity_dl.hpp>
#include <iostream>

TEST_CASE("API Activity-DL")
{
    OPACK_AGENT(MyAgent);
    ADL_ACTIVITY(Activity_A);
    ADL_ACTIVITY(Activity_B);
    struct ReadyA {};
    struct ReadyB {};

    OPACK_ACTION(Action1);
    OPACK_ACTION(Action2);
    OPACK_ACTION(Action3);

	auto world = opack::create_world();
	adl::import(world);
	world.component<ReadyA>();
	world.component<ReadyB>();

	opack::init<Action1>(world);
	opack::init<MyAgent>(world);
	auto agent = opack::spawn<MyAgent>(world);

	auto root = adl::activity<Activity_A>(world);
	root.emplace<adl::ContextualCondition>([](flecs::entity task, flecs::entity agent) {return agent.has<ReadyA>(); });
	CHECK(adl::children_count(root) == 0);
	CHECK(root.get<adl::Constructor>()->logical_constructor == adl::LogicalConstructor::AND);
	CHECK(root.get<adl::Constructor>()->temporal_constructor == adl::TemporalConstructor::SEQ_ORD);
	CHECK(adl::size(root) == 1);

	auto task_1 = adl::action<Action1>(root);
	task_1.emplace<adl::ContextualCondition>([](flecs::entity task, flecs::entity agent) {return agent.has<ReadyB>();});
	CHECK(adl::has_children(root));
	CHECK(adl::children_count(root) == 1);
	CHECK(adl::size(root) == 2);

	SUBCASE("Instanciation")
	{
        auto instance = adl::instantiate<Activity_A>(world);
        CHECK(instance.get<adl::Constructor>()->logical_constructor == adl::LogicalConstructor::AND);
        CHECK(instance.get<adl::Constructor>()->temporal_constructor == adl::TemporalConstructor::SEQ_ORD);
        CHECK(adl::children_count(instance) == 1);
        CHECK(!adl::is_satisfied(instance, agent));
	}

    SUBCASE("Composition")
    {
        auto activity_b = adl::activity<Activity_B>(world);
        auto instance = adl::compose<Activity_A>(activity_b);
        CHECK(adl::size(instance) == 2);
        CHECK(adl::size(activity_b) == 3);
    }

	std::vector<flecs::entity> actions{};

	SUBCASE("0 possible actions because agent is not ready yet") 
	{
		auto success = adl::potential_actions(root, std::back_inserter(actions), agent);
		CHECK(actions.size() == 0);
		CHECK(!success);
		CHECK(!adl::has_task_in_progress(root));
	}

	agent.add<ReadyA>();
	SUBCASE("0 possible actions because agent is not ready yet") 
	{
		auto success = adl::potential_actions(root, std::back_inserter(actions), agent);
		CHECK(actions.size() == 0);
		CHECK(!success);
		CHECK(!adl::has_task_in_progress(root));
	}

	agent.add<ReadyB>();
	SUBCASE("1 possible actions since agent is ready") 
	{
		auto success = adl::potential_actions(root, std::back_inserter(actions), agent);
		CHECK(actions.size() == 1);
		CHECK(!success);
		CHECK(!adl::has_task_in_progress(root));
	}

	SUBCASE("Check success")
	{
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::AND, adl::TemporalConstructor::IND);
		auto a1 = adl::action<Action1>(root);

		SUBCASE("Fail because there are still potential actions") // Since no task have been finished successfuly
		{
			auto success = adl::potential_actions(root, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(!success);
			CHECK(!adl::has_task_in_progress(root));
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("Fail because are still potential actions") // Since task is not finished
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

	SUBCASE("Traversal")
	{
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

		auto instance = adl::instantiate<Activity_A>(world);

		int counter { 0 };
		adl::traverse_dfs(instance, [&counter](flecs::entity task) { counter++; });
		CHECK(counter == 4);
	}

	SUBCASE("IND AND")
	{
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::AND, adl::TemporalConstructor::IND);
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
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

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
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

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
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

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
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ_ORD);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

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
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

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
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::AND, adl::TemporalConstructor::ORD);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

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
		auto root = adl::activity<Activity_A>(world, adl::LogicalConstructor::OR, adl::TemporalConstructor::ORD);
		auto a1 = adl::action<Action1>(root);
		auto a2 = adl::action<Action2>(root);
		auto a3 = adl::action<Action3>(root);

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
