#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/adl.hpp>
#include <opack/module/flows.hpp>

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
	auto action = adl::action<Action1>(root);
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

			a.set<opack::End, opack::Timestamp>({ 0.0f });
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

		int counter{ 0 };
		adl::traverse_dfs(instance, [&counter](flecs::entity task) { counter++; });
		CHECK(counter == 2);
	}
}

TEST_CASE("API Activity-DL - Operators")
{
	OPACK_AGENT(MyAgent);
	ADL_ACTIVITY(MyActivity);
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


	auto root = adl::activity<MyActivity>(world);
	auto action1 = adl::action<Action1>(root);
	auto action2 = adl::action<Action2>(root);
	auto action3 = adl::action<Action3>(root);
	auto lambda = [](opack::Entity task) {return task.has<Satisfied>(); };
	adl::condition<adl::Satisfaction>(action1, lambda);
	adl::condition<adl::Satisfaction>(action2, lambda);
	adl::condition<adl::Satisfaction>(action3, lambda);

	SUBCASE("IND AND")
	{
		auto instance = opack::spawn<MyActivity>(world);
		instance.set<adl::Constructor>({ adl::LogicalConstructor::AND, adl::TemporalConstructor::IND });
		auto children = adl::children(instance);
		auto a1 = children.at(1);
		auto a2 = children.at(2);
		auto a3 = children.at(3);

		std::vector<flecs::entity> actions{};
		SUBCASE("All tasks")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 3);
			CHECK(!success);
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("A2 & A3")
		{
			adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("T3")
		{
			adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(adl::in_progress(instance));
			CHECK(!success);
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Finished but failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::in_progress(instance));
			CHECK(!success);
		}

		a1.add<Satisfied>();
		SUBCASE("Finished but failed 2")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::in_progress(instance));
			CHECK(!success);
		}

		a2.add<Satisfied>();
		SUBCASE("Finished but failed 3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::in_progress(instance));
			CHECK(!success);
		}

		a3.add<Satisfied>();
		SUBCASE("Finished and succeeded")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::in_progress(instance));
			CHECK(success);
		}
	}

	SUBCASE("IND OR")
	{
		auto instance = opack::spawn<MyActivity>(world);
		instance.set<adl::Constructor>({ adl::LogicalConstructor::OR, adl::TemporalConstructor::IND });
		auto children = adl::children(instance);
		auto a1 = children.at(1);
		auto a2 = children.at(2);
		auto a3 = children.at(3);

		std::vector<flecs::entity> actions{};
		SUBCASE("All tasks")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("A2 & A3")
		{
			adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("T3")
		{
			adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(adl::in_progress(instance));
			CHECK(!success);
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Finished but failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::in_progress(instance));
			CHECK(!success);
		}

		a1.add<Satisfied>();
		SUBCASE("Finished and succeeded 1")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::in_progress(instance));
			CHECK(success);
		}

		a2.add<Satisfied>();
		SUBCASE("Finished and succeeded 2")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::in_progress(instance));
			CHECK(success);
		}

		a3.add<Satisfied>();
		SUBCASE("Finished and succeeded 3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!adl::in_progress(instance));
			CHECK(success);
		}
	}

	SUBCASE("SEQ AND")
	{
		auto instance = opack::spawn<MyActivity>(world);
		instance.set<adl::Constructor>({ adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ });
		auto children = adl::children(instance);
		auto a1 = children.at(1);
		auto a2 = children.at(2);
		auto a3 = children.at(3);

		std::vector<flecs::entity> actions{};

		SUBCASE("A1 & A2 & A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a1.add<Satisfied>();
		SUBCASE("A2 & A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a2.add<Satisfied>();
		SUBCASE("A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}
		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.add<Satisfied>();

		SUBCASE("Success")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::in_progress(instance));
		}
	}

	SUBCASE("SEQ OR")
	{
		auto instance = opack::spawn<MyActivity>(world);
		instance.set<adl::Constructor>({ adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ });
		auto children = adl::children(instance);
		auto a1 = children.at(1);
		auto a2 = children.at(2);
		auto a3 = children.at(3);

		std::vector<flecs::entity> actions{};

		SUBCASE("A1 & A2 & A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 2);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a1.add<Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::in_progress(instance));
		}

		a1.remove<Satisfied>();
		SUBCASE("A2 & A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		SUBCASE("A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}
		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.add<Satisfied>();

		SUBCASE("Success")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::in_progress(instance));
		}
	}

	SUBCASE("SEQ_ORD OR")
	{
		auto instance = opack::spawn<MyActivity>(world);
		instance.set<adl::Constructor>({ adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ_ORD });
		auto children = adl::children(instance);
		auto a1 = children.at(1);
		auto a2 = children.at(2);
		auto a3 = children.at(3);

		std::vector<flecs::entity> actions{};

		SUBCASE("A1")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a1.add<Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::in_progress(instance));
		}

		a1.remove<Satisfied>();
		SUBCASE("A2")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed - left only A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}
		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.add<Satisfied>();

		SUBCASE("Success")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::in_progress(instance));
		}
	}

	SUBCASE("SEQ_ORD AND")
	{
		auto instance = opack::spawn<MyActivity>(world);
		instance.set<adl::Constructor>({ adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD });
		auto children = adl::children(instance);
		auto a1 = children.at(1);
		auto a2 = children.at(2);
		auto a3 = children.at(3);

		std::vector<flecs::entity> actions{};

		SUBCASE("A1")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a1.add<Satisfied>();
		SUBCASE("A2")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a2.add<Satisfied>();
		SUBCASE("A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}
		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });

		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a3.add<Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::in_progress(instance));
		}
	}

	SUBCASE("ORD AND")
	{
		auto instance = opack::spawn<MyActivity>(world);
		instance.set<adl::Constructor>({ adl::LogicalConstructor::AND, adl::TemporalConstructor::ORD });
		auto children = adl::children(instance);
		auto a1 = children.at(1);
		auto a2 = children.at(2);
		auto a3 = children.at(3);

		std::vector<flecs::entity> actions{};

		SUBCASE("A1")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("A2")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a2.add<Satisfied>();
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a1.add<Satisfied>();
		SUBCASE("A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("None")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a3.add<Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::in_progress(instance));
		}
	}

	SUBCASE("ORD OR")
	{
		auto instance = opack::spawn<MyActivity>(world);
		instance.set<adl::Constructor>({ adl::LogicalConstructor::OR, adl::TemporalConstructor::ORD });
		auto children = adl::children(instance);
		auto a1 = children.at(1);
		auto a2 = children.at(2);
		auto a3 = children.at(3);

		std::vector<flecs::entity> actions{};

		SUBCASE("A1")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
		}

		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("A2")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Unfinished A3")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 1);
			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
		SUBCASE("Unfinished none")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(adl::in_progress(instance));
		}

		a1.set<opack::End, opack::Timestamp>({ 0.0f });
		a2.set<opack::End, opack::Timestamp>({ 0.0f });
		a3.set<opack::End, opack::Timestamp>({ 0.0f });
		SUBCASE("Failed")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(!success);
			CHECK(!adl::in_progress(instance));
		}

		a3.add<Satisfied>();
		SUBCASE("Success")
		{
			auto success = adl::potential_actions(instance, std::back_inserter(actions));
			CHECK(actions.size() == 0);
			CHECK(success);
			CHECK(!adl::in_progress(instance));
		}
	}
}

#include <opack/module/flows.hpp>
TEST_CASE("Sample Activity-Tree")
{
	OPACK_AGENT(MyAgent);
	ADL_ACTIVITY(MyActivity);

	OPACK_ACTION(MyAction);
	OPACK_ACTUATOR(MyActuator);
	OPACK_FLOW(MyFlow);
	struct Handling {};

	auto world = opack::create_world();
	adl::import(world);

	opack::init<MyAction>(world);
	opack::init<MyAgent>(world).add<MyFlow>();
	opack::init<MyActuator>(world);
	opack::add_actuator<MyActuator, MyAgent>(world);
	using f = ActivityFlowBuilder<MyFlow, Handling>;
	f(world).interval(2.0).build();
	opack::default_impact<f::Act>(world,
		[](flecs::entity agent, f::Act::inputs& inputs)
		{				
			auto action = std::get<opack::df<f::ActionSelection, f::ActionSelection::output>&>(inputs).value;
			if (action)
			{
				fmt::print("Doing : {}\n", action.path());
				opack::act<MyActuator>(agent, action);
			}
			return opack::make_outputs<f::Act>();
		}
	);

	auto agent = opack::spawn<MyAgent>(world);

	// --- Creating activities
	auto root = adl::activity<MyActivity>(world, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
	auto inform_task = adl::task("Inform", root, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
	auto a1 = adl::action<MyAction>(inform_task);
	adl::condition<adl::Satisfaction>(a1, adl::is_finished);
	auto a2 = adl::action<MyAction>(inform_task);
	adl::condition<adl::Satisfaction>(a2, adl::is_finished);

	auto confirm_task = adl::task("Confirm", root, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
	auto a3 = adl::action<MyAction>(confirm_task);
	adl::condition<adl::Satisfaction>(a3, adl::is_finished);

	auto ask_clearance = adl::task("Ask clearance", root, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
	auto a4 = adl::action<MyAction>(ask_clearance);
	adl::condition<adl::Satisfaction>(a4, adl::is_finished);
	auto a5 = adl::action<MyAction>(ask_clearance);
	adl::condition<adl::Satisfaction>(a5, adl::is_finished);

	auto task = adl::task("Task", root, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
	auto a6 = adl::action<MyAction>(task);
	adl::condition<adl::Satisfaction>(a6, adl::is_finished);
	auto a7 = adl::action<MyAction>(task);
	adl::condition<adl::Satisfaction>(a7, adl::is_finished);

	auto instance = opack::spawn<MyActivity>(world);
	agent.add<Handling>(instance);

	opack::run_with_webapp(world);
}
