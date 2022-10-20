#include <doctest/doctest.h>
#include <fmt/ranges.h>

#include "../utils.hpp"
#include <opack/core.hpp>
#include <opack/module/adl.hpp>
#include <opack/module/flows.hpp>
#include <opack/module/simple_agent.hpp>


struct Satisfied {};

TEST_CASE("API Activity-DL")
{
	MESSAGE("SETUP ...");
	OPACK_FLOW(MyFlow);
	OPACK_SUB_ACTION(Action_1, adl::Action);
	OPACK_SUB_ACTION(Action_2, adl::Action);
	OPACK_SUB_ACTION(Action_3, adl::Action);
	ADL_ACTIVITY(Activity_A);
	ADL_ACTIVITY(Activity_B);
	struct Data { float value{ 0.0f }; };
	struct ReadyA {};
	struct ReadyB {};

	auto world = opack::create_world();
	opack::import<simple>(world);
	opack::import<adl>(world);

	opack::prefab<simple::Agent>(world).add<MyFlow>();

	opack::init<Action_1>(world).duration(1.0f).require<simple::Actuator>();
	opack::init<Action_2>(world).duration(1.0f).require<simple::Actuator>();
	opack::init<Action_3>(world).duration(1.0f).require<simple::Actuator>();

	ActivityFlowBuilder<MyFlow, adl::Activity>(world).build();
	auto agent = opack::spawn<simple::Agent>(world);

	std::vector<flecs::entity> actions{};
	MESSAGE("SETUP ... done");

	auto root = adl::activity<Activity_A>(world);
	auto action = adl::action<Action_1>(root);

	MESSAGE("Validity");
	CHECK(adl::logical_constructor(root) == adl::LogicalConstructor::AND);
	CHECK(adl::temporal_constructor(root) == adl::TemporalConstructor::SEQ_ORD);
	CHECK(!root.has<adl::Satisfaction>());
	CHECK(action.has<adl::Satisfaction>());
	CHECK(adl::has_children(root));
	CHECK(!adl::is_leaf(root));
	CHECK(adl::children_count(root) == 1);
	CHECK(adl::size(root) == 2);
	CHECK(adl::is_root(root));
	CHECK(!adl::is_satisfied(root));
	CHECK(!adl::is_finished(root));

	CHECK(!adl::is_root(action));
	CHECK(adl::is_leaf(action));
	CHECK(!adl::has_children(action));
	CHECK(!adl::is_satisfied(action));
	CHECK(!adl::is_finished(action));

	MESSAGE("Contextual Condition");
	adl::condition<adl::Contextual>(root, [](opack::EntityView task) { return adl::ctx_entity<opack::By>(task).has<ReadyA>(); });
	adl::condition<adl::Contextual>(action, [](opack::EntityView task) { return adl::ctx_entity<opack::By>(task).has<ReadyB>(); });
	adl::condition<adl::Satisfaction>(action, [](opack::EntityView task) { return task.has<Satisfied>(); });


	auto instance = opack::spawn<Activity_A>(world);
	CHECK(adl::logical_constructor(instance) == adl::LogicalConstructor::AND);
	CHECK(adl::temporal_constructor(instance) == adl::TemporalConstructor::SEQ_ORD);
	CHECK(adl::is_root(instance));
	CHECK(adl::children_count(instance) == 1);
	CHECK(adl::size(instance) == 2);
	CHECK(!adl::is_satisfied(instance));
	CHECK(!adl::is_finished(instance));

	MESSAGE("Context");
	adl::ctx_entity<opack::Agent>(instance, agent);
	adl::ctx_value<Data>(instance, { 3.0 });
	adl::ctx_entity<opack::By>(instance, agent);
	instance.children([&](flecs::entity child)
		{
			CHECK(adl::ctx_entity<opack::Agent>(child) == agent);
			CHECK(adl::ctx_value<Data>(child)->value == 3.0);
		});

	MESSAGE("Composition");
	{
		auto activity_b = adl::activity<Activity_B>(world);
		auto sub_instance = adl::compose<Activity_A>(activity_b);
		CHECK(adl::size(sub_instance) == 2);
		CHECK(adl::size(activity_b) == 3);
	}

	CHECK(instance.has<adl::Contextual>());
	instance.children([](opack::Entity e)
		{
			CHECK(e.has<adl::Contextual>());
			CHECK(e.has<adl::Satisfaction>());
		});

	MESSAGE("0 possible actions because agent is not ready A yet");
	{
		actions.clear();
		adl::compute_potential_actions(instance, std::back_inserter(actions));
		CHECK(!adl::is_satisfied(instance));
		CHECK(!adl::in_progress(instance));
		CHECK(actions.size() == 0);
	}

	agent.add<ReadyA>();
	MESSAGE("0 possible actions because agent is not ready B yet");
	{
		actions.clear();
		adl::compute_potential_actions(instance, std::back_inserter(actions));
		CHECK(!adl::is_satisfied(instance));
		CHECK(!adl::in_progress(instance));
		CHECK(actions.size() == 0);
	}

	agent.add<ReadyB>();
	MESSAGE("1 possible actions since agent is ready");
	{
		actions.clear();
		adl::compute_potential_actions(instance, std::back_inserter(actions));
		CHECK(!adl::is_satisfied(instance));
		CHECK(!adl::in_progress(instance));
		CHECK(actions.size() == 1);
	}

	agent.add<adl::Activity>(instance);
	MESSAGE("Check success");
	{
		MESSAGE("Fail because there are still potential actions."); // Since no task have been finished successfuly
		{
			actions.clear();
			adl::compute_potential_actions(instance, std::back_inserter(actions));
			CHECK(!adl::is_satisfied(instance));
			CHECK(!adl::in_progress(instance));
			CHECK(actions.size() == 1);
		}
		auto a = actions.at(0);


		opack::step(world, 0.9f);
		MESSAGE("Fail because there are still actions in progress."); // Since task is not finished
		{
			actions.clear();
			adl::compute_potential_actions(instance, std::back_inserter(actions));
			CHECK(!adl::is_satisfied(instance));
			CHECK(adl::in_progress(instance));
			CHECK(actions.size() == 0);
		}

		opack::step_n(world, 2, 0.1f);
		MESSAGE("Failed activity"); // Since task is finished but not satisfied
		{
			actions.clear();
			adl::compute_potential_actions(instance, std::back_inserter(actions));
			CHECK(!adl::is_satisfied(instance));
			CHECK(!adl::in_progress(instance));
			CHECK(actions.size() == 0);
		}

		a.add<Satisfied>();
		MESSAGE("Succeeded activity"); // Since task is finished  and satisfied
		{
			actions.clear();
			adl::compute_potential_actions(instance, std::back_inserter(actions));
			CHECK(adl::is_satisfied(instance));
			CHECK(!adl::in_progress(instance));
			CHECK(actions.size() == 0);
		}
	}

	SUBCASE("Traversal")
	{
		int counter{ 0 };
		adl::traverse_dfs(instance, [&counter](flecs::entity task) { counter++; });
		CHECK(counter == 2);
	}
}

void begin(flecs::entity e) { e.add<opack::Begin, opack::Timestamp>();}
void end(flecs::entity e) { e.add<opack::End, opack::Timestamp>();}
void satisfy(flecs::entity e) { e.add<Satisfied>();}


struct test
{
	std::string title {};	
	std::vector<flecs::entity>	expected_actions {};	
	bool in_progress {false};	
	bool is_satisfied {false};	
	bool is_finished {false};	

	void check(flecs::entity task)
	{
		MESSAGE("Step : ", title);
		std::vector<flecs::entity> actions;
		adl::compute_potential_actions(task, std::back_inserter(actions));

		CHECK(actions == expected_actions);
		CHECK(adl::in_progress(task) == in_progress);
		CHECK(adl::is_satisfied(task) == is_satisfied);
		CHECK(adl::is_finished(task) == is_finished);
	}
};

TEST_CASE("Activity-DL operators")
{
	MESSAGE("SETUP ...");
	OPACK_FLOW(MyFlow);
	OPACK_SUB_ACTION(Action_1, adl::Action);
	OPACK_SUB_ACTION(Action_2, adl::Action);
	OPACK_SUB_ACTION(Action_3, adl::Action);
	ADL_ACTIVITY(Activity_A);

	auto world = opack::create_world();
	opack::import<simple>(world);
	opack::import<adl>(world);

	opack::prefab<simple::Agent>(world).add<MyFlow>();

	opack::init<Action_1>(world).duration(1.0f).require<simple::Actuator>();
	opack::init<Action_2>(world).duration(1.0f).require<simple::Actuator>();
	opack::init<Action_3>(world).duration(1.0f).require<simple::Actuator>();

	ActivityFlowBuilder<MyFlow, adl::Activity>(world).build();
	auto agent = opack::spawn<simple::Agent>(world);

	MESSAGE("SETUP ... done");

	auto root = adl::activity<Activity_A>(world);
	auto action1 = adl::action<Action_1>(root);
	auto action2 = adl::action<Action_2>(root);
	auto action3 = adl::action<Action_3>(root);
	auto lambda = [](opack::EntityView task) {return task.has<Satisfied>(); };
	adl::condition<adl::Satisfaction>(action1, lambda);
	adl::condition<adl::Satisfaction>(action2, lambda);
	adl::condition<adl::Satisfaction>(action3, lambda);

	auto instance = opack::spawn<Activity_A>(world);
	auto children = adl::children(instance);
	auto a1 = children.at(1);
	auto a2 = children.at(2);
	auto a3 = children.at(3);

	SUBCASE("IND OR")
	{
		adl::temporal_constructor(instance, adl::TemporalConstructor::IND);
		adl::logical_constructor(instance, adl::LogicalConstructor::OR);

		test{
			.title = "START",
			.expected_actions {a1, a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a1);
		test{
			.title = "Begin a1",
			.expected_actions {a2, a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a2);
		test{
			.title = "Begin a2",
			.expected_actions {a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a1);
		test{
			.title = "End a1",
			.expected_actions {a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a2);
		test{
			.title = "End a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		satisfy(a2);
		test{
			.title = "Satisfy a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = false
		}.check(instance);

		begin(a3);
		test{
			.title = "Begin a3",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = true,
			.is_finished = false
		}.check(instance);

		end(a3);
		test{
			.title = "End a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true, 
		}.check(instance);

		satisfy(a1);
		test{
			.title = "Satisfy a1",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);

		satisfy(a3);
		test{
			.title = "Satisfy a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);
	}

	SUBCASE("IND AND")
	{
		adl::temporal_constructor(instance, adl::TemporalConstructor::IND);
		adl::logical_constructor(instance, adl::LogicalConstructor::AND);

		test{
			.title = "START",
			.expected_actions {a1, a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a1);
		test{
			.title = "Begin a1",
			.expected_actions {a2, a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a2);
		test{
			.title = "Begin a2",
			.expected_actions {a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a1);
		test{
			.title = "End a1",
			.expected_actions {a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = true,
		}.check(instance);

		end(a2);
		test{
			.title = "End a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true,
		}.check(instance);

		satisfy(a2);
		test{
			.title = "Satisfy a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true
		}.check(instance);

		begin(a3);
		test{
			.title = "Begin a3",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = true
		}.check(instance);

		end(a3);
		test{
			.title = "End a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true
		}.check(instance);

		satisfy(a1);
		test{
			.title = "Satisfy a1",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true
		}.check(instance);

		satisfy(a3);
		test{
			.title = "Satisfy a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);
	}

	SUBCASE("IND XOR")
	{
		adl::temporal_constructor(instance, adl::TemporalConstructor::IND);
		adl::logical_constructor(instance, adl::LogicalConstructor::XOR);

		test{
			.title = "START",
			.expected_actions {a1, a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a1);
		test{
			.title = "Begin a1",
			.expected_actions {a2, a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a2);
		test{
			.title = "Begin a2",
			.expected_actions {a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a1);
		test{
			.title = "End a1",
			.expected_actions {a3},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a2);
		test{
			.title = "End a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		satisfy(a2);
		test{
			.title = "Satisfy a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true 
		}.check(instance);

		begin(a3);
		test{
			.title = "Begin a3",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = true,
			.is_finished = true 
		}.check(instance);

		end(a3);
		test{
			.title = "End a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true, 
		}.check(instance);

		satisfy(a1);
		test{
			.title = "Satisfy a1",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);

		satisfy(a3);
		test{
			.title = "Satisfy a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);
	}

	SUBCASE("SEQ OR")
	{
		adl::temporal_constructor(instance, adl::TemporalConstructor::SEQ);
		adl::logical_constructor(instance, adl::LogicalConstructor::OR);

		test{
			.title = "START",
			.expected_actions {a1, a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a1);
		test{
			.title = "Begin a1",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a1);
		test{
			.title = "End a1",
			.expected_actions {a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a2);
		test{
			.title = "Begin a2",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a2);
		test{
			.title = "End a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		satisfy(a2);
		test{
			.title = "Satisfy a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = false 
		}.check(instance);

		begin(a3);
		test{
			.title = "Begin a3",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = true,
			.is_finished = false 
		}.check(instance);

		end(a3);
		test{
			.title = "End a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true, 
		}.check(instance);

		satisfy(a1);
		test{
			.title = "Satisfy a1",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);

		satisfy(a3);
		test{
			.title = "Satisfy a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);
	}

	SUBCASE("SEQ AND")
	{
		adl::temporal_constructor(instance, adl::TemporalConstructor::SEQ);
		adl::logical_constructor(instance, adl::LogicalConstructor::AND);

		test{
			.title = "START",
			.expected_actions {a1, a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a1);
		test{
			.title = "Begin a1",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a1);
		test{
			.title = "End a1",
			.expected_actions {a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true
		}.check(instance);

		begin(a2);
		test{
			.title = "Begin a2",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = true 
		}.check(instance);

		end(a2);
		test{
			.title = "End a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true 
		}.check(instance);

		satisfy(a2);
		test{
			.title = "Satisfy a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true 
		}.check(instance);

		begin(a3);
		test{
			.title = "Begin a3",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = true 
		}.check(instance);

		end(a3);
		test{
			.title = "End a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true, 
		}.check(instance);

		satisfy(a1);
		test{
			.title = "Satisfy a1",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = true
		}.check(instance);

		satisfy(a3);
		test{
			.title = "Satisfy a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);
	}

	SUBCASE("SEQ XOR")
	{
		adl::temporal_constructor(instance, adl::TemporalConstructor::SEQ);
		adl::logical_constructor(instance, adl::LogicalConstructor::XOR);

		test{
			.title = "START",
			.expected_actions {a1, a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		begin(a1);
		test{
			.title = "Begin a1",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false
		}.check(instance);

		end(a1);
		test{
			.title = "End a1",
			.expected_actions {a2, a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false 
		}.check(instance);

		begin(a2);
		test{
			.title = "Begin a2",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = false,
			.is_finished = false 
		}.check(instance);

		end(a2);
		test{
			.title = "End a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = false,
			.is_finished = false 
		}.check(instance);

		satisfy(a2);
		test{
			.title = "Satisfy a2",
			.expected_actions {a3},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true 
		}.check(instance);

		begin(a3);
		test{
			.title = "Begin a3",
			.expected_actions {},
			.in_progress = true,
			.is_satisfied = true,
			.is_finished = true 
		}.check(instance);

		end(a3);
		test{
			.title = "End a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true 
		}.check(instance);

		satisfy(a1);
		test{
			.title = "Satisfy a1",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);

		satisfy(a3);
		test{
			.title = "Satisfy a3",
			.expected_actions {},
			.in_progress = false,
			.is_satisfied = true,
			.is_finished = true
		}.check(instance);
	}
}

//TEST_CASE("API Activity-DL - Operators")
//{
//	OPACK_AGENT(MyAgent);
//	ADL_ACTIVITY(MyActivity);
//	struct Satisfied {};
//	struct Data { float value{ 1.0 }; };
//
//	OPACK_ACTION(Action1);
//	OPACK_ACTION(Action2);
//	OPACK_ACTION(Action3);
//
//	auto world = opack::create_world();
//	world.import<adl>();
//
//	opack::init<Action1>(world);
//	opack::init<Action2>(world);
//	opack::init<Action3>(world);
//	opack::init<MyAgent>(world);
//
//	auto agent = opack::spawn<MyAgent>(world);
//
//
//	auto root = adl::activity<MyActivity>(world);
//	auto action1 = adl::action<Action1>(root);
//	auto action2 = adl::action<Action2>(root);
//	auto action3 = adl::action<Action3>(root);
//	auto lambda = [](opack::EntityView task) {return task.has<Satisfied>(); };
//	adl::condition<adl::Satisfaction>(action1, lambda);
//	adl::condition<adl::Satisfaction>(action2, lambda);
//	adl::condition<adl::Satisfaction>(action3, lambda);
//
//	SUBCASE("IND AND")
//	{
//		auto instance = opack::spawn<MyActivity>(world);
//		instance.set<adl::Constructor>({ adl::LogicalConstructor::AND, adl::TemporalConstructor::IND });
//		auto children = adl::children(instance);
//		auto a1 = children.at(1);
//		auto a2 = children.at(2);
//		auto a3 = children.at(3);
//
//		std::vector<flecs::entity> actions{};
//		SUBCASE("All tasks")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 3);
//			CHECK(!success);
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("A2 & A3")
//		{
//			adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("T3")
//		{
//			adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(adl::in_progress(instance));
//			CHECK(!success);
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Finished but failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(instance));
//			CHECK(!success);
//		}
//
//		a1.add<Satisfied>();
//		SUBCASE("Finished but failed 2")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(instance));
//			CHECK(!success);
//		}
//
//		a2.add<Satisfied>();
//		SUBCASE("Finished but failed 3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(instance));
//			CHECK(!success);
//		}
//
//		a3.add<Satisfied>();
//		SUBCASE("Finished and succeeded")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(instance));
//			CHECK(success);
//		}
//	}
//
//	SUBCASE("IND OR")
//	{
//		auto instance = opack::spawn<MyActivity>(world);
//		instance.set<adl::Constructor>({ adl::LogicalConstructor::OR, adl::TemporalConstructor::IND });
//		auto children = adl::children(instance);
//		auto a1 = children.at(1);
//		auto a2 = children.at(2);
//		auto a3 = children.at(3);
//
//		std::vector<flecs::entity> actions{};
//		SUBCASE("All tasks")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("A2 & A3")
//		{
//			adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("T3")
//		{
//			adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(adl::in_progress(instance));
//			CHECK(!success);
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Finished but failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(instance));
//			CHECK(!success);
//		}
//
//		a1.add<Satisfied>();
//		SUBCASE("Finished and succeeded 1")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(instance));
//			CHECK(success);
//		}
//
//		a2.add<Satisfied>();
//		SUBCASE("Finished and succeeded 2")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(instance));
//			CHECK(success);
//		}
//
//		a3.add<Satisfied>();
//		SUBCASE("Finished and succeeded 3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!adl::in_progress(instance));
//			CHECK(success);
//		}
//	}
//
//	SUBCASE("SEQ AND")
//	{
//		auto instance = opack::spawn<MyActivity>(world);
//		instance.set<adl::Constructor>({ adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ });
//		auto children = adl::children(instance);
//		auto a1 = children.at(1);
//		auto a2 = children.at(2);
//		auto a3 = children.at(3);
//
//		std::vector<flecs::entity> actions{};
//
//		SUBCASE("A1 & A2 & A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a1.add<Satisfied>();
//		SUBCASE("A2 & A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a2.add<Satisfied>();
//		SUBCASE("A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.add<Satisfied>();
//
//		SUBCASE("Success")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(success);
//			CHECK(!adl::in_progress(instance));
//		}
//	}
//
//	SUBCASE("SEQ OR")
//	{
//		auto instance = opack::spawn<MyActivity>(world);
//		instance.set<adl::Constructor>({ adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ });
//		auto children = adl::children(instance);
//		auto a1 = children.at(1);
//		auto a2 = children.at(2);
//		auto a3 = children.at(3);
//
//		std::vector<flecs::entity> actions{};
//
//		SUBCASE("A1 & A2 & A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 2);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a1.add<Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a1.remove<Satisfied>();
//		SUBCASE("A2 & A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		SUBCASE("A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.add<Satisfied>();
//
//		SUBCASE("Success")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(success);
//			CHECK(!adl::in_progress(instance));
//		}
//	}
//
//	SUBCASE("SEQ_ORD OR")
//	{
//		auto instance = opack::spawn<MyActivity>(world);
//		instance.set<adl::Constructor>({ adl::LogicalConstructor::OR, adl::TemporalConstructor::SEQ_ORD });
//		auto children = adl::children(instance);
//		auto a1 = children.at(1);
//		auto a2 = children.at(2);
//		auto a3 = children.at(3);
//
//		std::vector<flecs::entity> actions{};
//
//		SUBCASE("A1")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a1.add<Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a1.remove<Satisfied>();
//		SUBCASE("A2")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed - left only A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.add<Satisfied>();
//
//		SUBCASE("Success")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(success);
//			CHECK(!adl::in_progress(instance));
//		}
//	}
//
//	SUBCASE("SEQ_ORD AND")
//	{
//		auto instance = opack::spawn<MyActivity>(world);
//		instance.set<adl::Constructor>({ adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD });
//		auto children = adl::children(instance);
//		auto a1 = children.at(1);
//		auto a2 = children.at(2);
//		auto a3 = children.at(3);
//
//		std::vector<flecs::entity> actions{};
//
//		SUBCASE("A1")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a2) == actions.end());
//			CHECK(std::find(actions.begin(), actions.end(), a3) == actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a1.add<Satisfied>();
//		SUBCASE("A2")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a2.add<Satisfied>();
//		SUBCASE("A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a3.add<Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(success);
//			CHECK(!adl::in_progress(instance));
//		}
//	}
//
//	SUBCASE("ORD AND")
//	{
//		auto instance = opack::spawn<MyActivity>(world);
//		instance.set<adl::Constructor>({ adl::LogicalConstructor::AND, adl::TemporalConstructor::ORD });
//		auto children = adl::children(instance);
//		auto a1 = children.at(1);
//		auto a2 = children.at(2);
//		auto a3 = children.at(3);
//
//		std::vector<flecs::entity> actions{};
//
//		SUBCASE("A1")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("A2")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a2.add<Satisfied>();
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a1.add<Satisfied>();
//		SUBCASE("A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("None")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a3.add<Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(success);
//			CHECK(!adl::in_progress(instance));
//		}
//	}
//
//	SUBCASE("ORD OR")
//	{
//		auto instance = opack::spawn<MyActivity>(world);
//		instance.set<adl::Constructor>({ adl::LogicalConstructor::OR, adl::TemporalConstructor::ORD });
//		auto children = adl::children(instance);
//		auto a1 = children.at(1);
//		auto a2 = children.at(2);
//		auto a3 = children.at(3);
//
//		std::vector<flecs::entity> actions{};
//
//		SUBCASE("A1")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a1) != actions.end());
//		}
//
//		a1.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("A2")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a2) != actions.end());
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a2.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Unfinished A3")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 1);
//			CHECK(std::find(actions.begin(), actions.end(), a3) != actions.end());
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a3.set<opack::Begin, opack::Timestamp>({ 0.0f });
//		SUBCASE("Unfinished none")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(adl::in_progress(instance));
//		}
//
//		a1.set<opack::End, opack::Timestamp>({ 0.0f });
//		a2.set<opack::End, opack::Timestamp>({ 0.0f });
//		a3.set<opack::End, opack::Timestamp>({ 0.0f });
//		SUBCASE("Failed")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(!success);
//			CHECK(!adl::in_progress(instance));
//		}
//
//		a3.add<Satisfied>();
//		SUBCASE("Success")
//		{
//			auto success = adl::compute_potential_actions(instance, std::back_inserter(actions));
//			CHECK(actions.size() == 0);
//			CHECK(success);
//			CHECK(!adl::in_progress(instance));
//		}
//	}
//}
//
//#include <opack/module/flows.hpp>
//TEST_CASE("Sample Activity-Tree")
//{
//	OPACK_AGENT(MyAgent);
//	ADL_ACTIVITY(MyActivity);
//
//	OPACK_ACTION(MyAction);
//	OPACK_ACTUATOR(MyActuator);
//	OPACK_FLOW(MyFlow);
//	struct Handling {};
//
//	auto world = opack::create_world();
//	world.import<adl>();
//
//	opack::init<MyAgent>(world).add<MyFlow>();
//	opack::init<MyActuator>(world);
//	opack::init<MyAction>(world).require<MyActuator>();
//	opack::add_actuator<MyActuator, MyAgent>(world);
//	using f = ActivityFlowBuilder<MyFlow, Handling>;
//	f(world).interval(2.0).build();
//	opack::default_impact<f::Act>(world,
//		[](flecs::entity agent, f::Act::inputs& inputs)
//		{				
//			auto action = std::get<opack::df<f::ActionSelection, f::ActionSelection::output>&>(inputs).value;
//			if (action)
//			{
//				fmt::print("Doing : {}\n", action.path());
//				opack::act(agent, action);
//			}
//			return opack::make_outputs<f::Act>();
//		}
//	);
//
//	auto agent = opack::spawn<MyAgent>(world);
//
//	// --- Creating activities
//	auto root = adl::activity<MyActivity>(world, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
//	auto inform_task = adl::task("Inform", root, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
//	auto a1 = adl::action<MyAction>(inform_task);
//	adl::condition<adl::Satisfaction>(a1, adl::is_finished);
//	auto a2 = adl::action<MyAction>(inform_task);
//	adl::condition<adl::Satisfaction>(a2, adl::is_finished);
//
//	auto confirm_task = adl::task("Confirm", root, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
//	auto a30 = adl::action<MyAction>(confirm_task);
//	adl::condition<adl::Satisfaction>(a30, adl::is_finished);
//	auto optional_task = adl::task("Optional", confirm_task, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
//	auto a31 = adl::action<MyAction>(optional_task);
//	auto a32 = adl::action<MyAction>(optional_task);
//	adl::condition<adl::Satisfaction>(a31, adl::is_finished);
//	adl::condition<adl::Satisfaction>(a32, adl::is_finished);
//
//	auto ask_clearance = adl::task("Ask clearance", root, adl::LogicalConstructor::AND, adl::TemporalConstructor::SEQ_ORD);
//	auto a4 = adl::action<MyAction>(ask_clearance);
//	adl::condition<adl::Satisfaction>(a4, adl::is_finished);
//	auto a5 = adl::action<MyAction>(ask_clearance);
//	adl::condition<adl::Satisfaction>(a5, adl::is_finished);
//
//	auto task = adl::task("Task", root, adl::LogicalConstructor::OR, adl::TemporalConstructor::IND);
//	auto a6 = adl::action<MyAction>(task);
//	adl::condition<adl::Satisfaction>(a6, adl::is_finished);
//	auto a7 = adl::action<MyAction>(task);
//	adl::condition<adl::Satisfaction>(a7, adl::is_finished);
//
//	auto instance = opack::spawn<MyActivity>(world);
//	agent.add<Handling>(instance);
//}
