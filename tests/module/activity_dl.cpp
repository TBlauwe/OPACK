#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/adl.hpp>

TEST_CASE("API Activity-DL")
{
	OPACK_AGENT(MyAgent);
	ADL_ACTIVITY(Activity_A);
	ADL_ACTIVITY(Activity_B);
	struct ReadyA {};
	struct ReadyB {};
	struct Satisfied {};
	struct Data { float value{ 1.0 }; };

	OPACK_ACTION(Action1);
	OPACK_ACTION(Action2);
	OPACK_ACTION(Action3);

	auto world = opack::create_world();
	adl::import(world);

	opack::init<Action1>(world);
	opack::init<Action2>(world);
	opack::init<Action3>(world);
	opack::init<MyAgent>(world);

	auto agent = opack::spawn<MyAgent>(world);

	std::vector<flecs::entity> actions{};

    auto root = adl::activity<Activity_A>(world);
    auto action= adl::action<Action1>(root);
	SUBCASE("Simple tree")
	{
		adl::condition<adl::Satisfaction>(root, adl::is_finished);
		adl::condition<adl::Satisfaction>(action, adl::is_finished);

		CHECK(root.get<adl::Constructor>()->logical == adl::LogicalConstructor::AND);
		CHECK(root.get<adl::Constructor>()->temporal == adl::TemporalConstructor::SEQ_ORD);
		CHECK(adl::has_children(root));
		CHECK(adl::children_count(root) == 1);
		CHECK(adl::size(root) == 2);
		CHECK(adl::is_root(root));
		CHECK(!adl::is_root(action));

		auto instance = opack::spawn<Activity_A>(world);
		CHECK(instance.get<adl::Constructor>()->logical == adl::LogicalConstructor::AND);
		CHECK(instance.get<adl::Constructor>()->temporal == adl::TemporalConstructor::SEQ_ORD);
		CHECK(adl::is_root(instance));
		CHECK(adl::children_count(instance) == 1);
		CHECK(adl::size(instance) == 2);
		CHECK(!adl::is_satisfied(instance));
	}

    SUBCASE("Context")
    {
		auto instance = opack::spawn<Activity_A>(world);
        adl::ctx_entity<opack::Agent>(instance, agent);
        adl::ctx_value<Data>(instance, { 3.0 });
        instance.children([&](flecs::entity child)
            {
                CHECK(adl::ctx_entity<opack::Agent>(child) == agent);
                CHECK(adl::ctx_value<Data>(child)->value == 3.0);
            });
    }

    SUBCASE("Composition")
    {
        auto activity_b = adl::activity<Activity_B>(world);
        auto instance = adl::compose<Activity_A>(activity_b);
        CHECK(adl::size(instance) == 2);
        CHECK(adl::size(activity_b) == 3);
    }

	SUBCASE("Contextual Condition")
	{
        adl::condition<adl::Contextual>(root, [](opack::Entity task) { return adl::ctx_entity<opack::By>(task).has<ReadyA>(); });
        adl::condition<adl::Contextual>(action, [](opack::Entity task) { return adl::ctx_entity<opack::By>(task).has<ReadyB>(); });
        adl::condition<adl::Satisfaction>(action, [](opack::Entity task) { return task.has<Satisfied>(); });
		auto instance = opack::spawn<Activity_A>(world);
        adl::ctx_entity<opack::By>(instance, agent);

        CHECK(instance.has<adl::Contextual>());
        instance.children([](opack::Entity e)
            {
                CHECK(e.has<adl::Contextual>());
                CHECK(e.has<adl::Satisfaction>());
            });

        MESSAGE("0 possible actions because agent is not ready A yet");
        {
            auto success = adl::potential_actions(instance, std::back_inserter(actions));
            CHECK(actions.size() == 0);
            CHECK(!success);
            CHECK(!adl::in_progress(instance));
        }

        agent.add<ReadyA>();
        MESSAGE("0 possible actions because agent is not ready B yet");
        {
            auto success = adl::potential_actions(instance, std::back_inserter(actions));
            CHECK(actions.size() == 0);
            CHECK(!success);
            CHECK(!adl::in_progress(instance));
        }

        agent.add<ReadyB>();
        MESSAGE("1 possible actions since agent is ready");
        {
            auto success = adl::potential_actions(instance, std::back_inserter(actions));
            CHECK(actions.size() == 1);
            CHECK(!success);
            CHECK(!adl::in_progress(instance));
        }

        MESSAGE("Check success");
        {
            MESSAGE("Fail because there are still potential actions."); // Since no task have been finished successfuly
            {
                actions.clear();
                auto success = adl::potential_actions(instance, std::back_inserter(actions));
                CHECK(actions.size() == 1);
                CHECK(!success);
                CHECK(!adl::in_progress(instance));
            }

            auto a = actions.at(0);
            a.set<opack::Begin, opack::Timestamp>({ 0.0f });
            MESSAGE("Fail because there are still actions in progress."); // Since task is not finished
            {
                actions.clear();
                auto success = adl::potential_actions(instance, std::back_inserter(actions));
                CHECK(actions.size() == 0);
                CHECK(!success);
                CHECK(adl::in_progress(instance));
            }

            a.set<opack::End, opack::Timestamp>({0.0f});
            MESSAGE("Failed activity"); // Since task is finished but not satisfied
            {
                actions.clear();
                auto success = adl::potential_actions(instance, std::back_inserter(actions));
                CHECK(actions.size() == 0);
                CHECK(!success);
                CHECK(!adl::in_progress(instance));
            }

            a.add<Satisfied>();
            MESSAGE("Succeeded activity"); // Since task is finished  and satisfied
            {
                actions.clear();
                auto success = adl::potential_actions(instance, std::back_inserter(actions));
                CHECK(actions.size() == 0);
                CHECK(success);
                CHECK(!adl::in_progress(instance));
            }
        }
	}

	SUBCASE("Traversal")
	{
		auto instance = opack::spawn<Activity_A>(world);

		int counter { 0 };
		adl::traverse_dfs(instance, [&counter](flecs::entity task) { counter++; });
		CHECK(counter == 2);
	}
}
//
//
//
//
//	SUBCASE("IND AND")
//	{
//		auto root = adl::instantiate<Activity_A>(world);
//		root.add(adl::LogicalConstructor::AND).add(adl::TemporalConstructor::IND);
//		auto a1 = adl::action<Action1>(root);
//		auto a2 = adl::action<Action2>(root);
//		auto a3 = adl::action<Action3>(root);
//
//		SUBCASE("All tasks")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("A2 & A3")
//		{
//			adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("T3")
//		{
//			adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(adl::in_progress(root));
//			CHECK(!success);
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Finished but failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(root));
//			CHECK(!success);
//		}
//
//		a1.add<adl::Satisfied>();
//		SUBCASE("Finished but failed 2")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(root));
//			CHECK(!success);
//		}
//
//		a2.add<adl::Satisfied>();
//		SUBCASE("Finished but failed 3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(root));
//			CHECK(!success);
//		}
//
//		a3.add<adl::Satisfied>();
//		SUBCASE("Finished and succeeded")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(root));
//			CHECK(success);
//		}
//	}
//
//	SUBCASE("IND OR")
//	{
//		auto root = adl::instantiate<Activity_A>(world);
//		root.add(adl::LogicalConstructor::OR).add(adl::TemporalConstructor::IND);
//		auto a1 = adl::action<Action1>(root);
//		auto a2 = adl::action<Action2>(root);
//		auto a3 = adl::action<Action3>(root);
//
//		SUBCASE("All tasks")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("A2 & A3")
//		{
//			adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("T3")
//		{
//			adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(adl::in_progress(root));
//			CHECK(!success);
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Finished but failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(root));
//			CHECK(!success);
//		}
//
//		a1.add<adl::Satisfied>();
//		SUBCASE("Finished and succeeded 1")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(root));
//			CHECK(success);
//		}
//
//		a2.add<adl::Satisfied>();
//		SUBCASE("Finished and succeeded 2")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(root));
//			CHECK(success);
//		}
//
//		a3.add<adl::Satisfied>();
//		SUBCASE("Finished and succeeded 3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(root));
//			CHECK(success);
//		}
//	}
//
//	SUBCASE("SEQ AND")
//	{
//		auto root = adl::instantiate<Activity_A>(world);
//		root.add(adl::LogicalConstructor::AND).add(adl::TemporalConstructor::SEQ);
//		auto a1 = adl::action<Action1>(root);
//		auto a2 = adl::action<Action2>(root);
//		auto a3 = adl::action<Action3>(root);
//
//		SUBCASE("A1 & A2 & A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a1.add<adl::Satisfied>();
//		SUBCASE("A2 & A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a2.add<adl::Satisfied>();
//		SUBCASE("A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.add<adl::Satisfied>();
//
//		SUBCASE("Success")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(success);
//			CHECK(!adl::in_progress(root));
//		}
//	}
//
//	SUBCASE("SEQ OR")
//	{
//		auto root = adl::instantiate<Activity_A>(world);
//		root.add(adl::LogicalConstructor::OR).add(adl::TemporalConstructor::SEQ);
//		auto a1 = adl::action<Action1>(root);
//		auto a2 = adl::action<Action2>(root);
//		auto a3 = adl::action<Action3>(root);
//
//		SUBCASE("A1 & A2 & A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==2);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a1.add<adl::Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a1.remove<adl::Satisfied>();
//		SUBCASE("A2 & A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		SUBCASE("A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.add<adl::Satisfied>();
//
//		SUBCASE("Success")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(success);
//			CHECK(!adl::in_progress(root));
//		}
//	}	
//	
//	SUBCASE("SEQ_ORD OR")
//	{
//		auto root = adl::instantiate<Activity_A>(world);
//		root.add(adl::LogicalConstructor::OR).add(adl::TemporalConstructor::SEQ_ORD);
//		auto a1 = adl::action<Action1>(root);
//		auto a2 = adl::action<Action2>(root);
//		auto a3 = adl::action<Action3>(root);
//
//		SUBCASE("A1")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a1.add<adl::Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a1.remove<adl::Satisfied>();
//		SUBCASE("A2")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed - left only A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.add<adl::Satisfied>();
//
//		SUBCASE("Success")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(success);
//			CHECK(!adl::in_progress(root));
//		}
//	}	
//
//	SUBCASE("SEQ_ORD AND")
//	{
//		auto root = adl::instantiate<Activity_A>(world);
//		root.add(adl::LogicalConstructor::AND).add(adl::TemporalConstructor::SEQ_ORD);
//		auto a1 = adl::action<Action1>(root);
//		auto a2 = adl::action<Action2>(root);
//		auto a3 = adl::action<Action3>(root);
//
//		SUBCASE("A1")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a1.add<adl::Satisfied>();
//		SUBCASE("A2")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a2.add<adl::Satisfied>();
//		SUBCASE("A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a3.add<adl::Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(success);
//			CHECK(!adl::in_progress(root));
//		}
//	}
//
//	SUBCASE("ORD AND")
//	{
//		auto root = adl::instantiate<Activity_A>(world);
//		root.add(adl::LogicalConstructor::AND).add(adl::TemporalConstructor::ORD);
//		auto a1 = adl::action<Action1>(root);
//		auto a2 = adl::action<Action2>(root);
//		auto a3 = adl::action<Action3>(root);
//
//		SUBCASE("A1")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("A2")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a2.add<adl::Satisfied>();
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a1.add<adl::Satisfied>();
//		SUBCASE("A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a3.add<adl::Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(success);
//			CHECK(!adl::in_progress(root));
//		}
//	}
//
//	SUBCASE("ORD OR")
//	{
//		auto root = adl::instantiate<Activity_A>(world);
//		root.add(adl::LogicalConstructor::OR).add(adl::TemporalConstructor::ORD);
//		auto a1 = adl::action<Action1>(root);
//		auto a2 = adl::action<Action2>(root);
//		auto a3 = adl::action<Action3>(root);
//
//		SUBCASE("A1")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("A2")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Unfinished A3")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==1);
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("Unfinished none")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(adl::in_progress(root));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(root));
//		}
//
//		a3.add<adl::Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::potential_actions(root, std::back_inserter(actions));
//			CHECK(actions.size()==0);
//			CHECK(success);
//			CHECK(!adl::in_progress(root));
//		}
//	}
//}
