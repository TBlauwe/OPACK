#include <doctest/doctest.h>
#include <opack/core/simulation.hpp>
#include <string.h> // For const char * comparisons

TEST_SUITE_BEGIN("Simulation");

TEST_CASE("Basics")
{
	auto sim = opack::Simulation();
	REQUIRE(sim.tick() == 0);
	REQUIRE(sim.count<opack::Agent>() == 0);
	REQUIRE(sim.count<opack::Sense>() == 0);

	SUBCASE("Controls - setting")
	{
		float value = 10.f;
		sim.target_fps(value);
		CHECK(sim.target_fps() == value);

		value = 60.f;
		sim.target_fps(value);
		CHECK(sim.target_fps() == value);

		value = 0.5f;
		sim.time_scale(value);
		CHECK(sim.time_scale() == value);

		value = 1.0f;
		sim.time_scale(value);
		CHECK(sim.time_scale() == value);
	}

	SUBCASE("Controls - steps")
	{
		sim.step();
		CHECK(sim.tick() == 1);
		sim.step_n(10);
		CHECK(sim.tick() == 11);

		float delta_time{ 1.0f };
		float total_time{ 0.0f };
		sim.step(delta_time);
		CHECK(sim.delta_time() == delta_time);
		total_time += delta_time;
		CHECK(sim.time() >= total_time);

		delta_time = 3.0f;
		sim.step(delta_time);
		CHECK(sim.delta_time() == delta_time);
		total_time += delta_time;
		CHECK(sim.time() >= total_time);

		delta_time = 1.0f;
		sim.step_n(10, 1.0f);
		CHECK(sim.tick() == 23);
		CHECK(sim.delta_time() == delta_time);
		total_time += 10*delta_time;
		CHECK(sim.time() >= total_time);
	}

	SUBCASE("Controls - shutdown")
	{
		sim.stop();
	}

	SUBCASE("Entity equality")
	{
		struct Test {};
		auto e_1 = sim.world.entity<Test>();
		auto e_2 = sim.entity<Test>();
		CHECK(e_1 == e_2);

		auto c_1 = sim.world.component<Test>();
		CHECK(c_1 == e_2);

		auto p_1 = sim.world.prefab<Test>();
		CHECK(p_1 == e_2);

		CHECK(e_2 == sim.id<Test>());
	}

	SUBCASE("Type register")
	{
		auto check = []<typename Base, std::derived_from<Base> Derived>(flecs::entity e) -> flecs::entity
		{
			CHECK(e.template is_a<Base>());
			CHECK(e.template has<Base>());
			CHECK(e.template has<Derived>());
			return e;
		};

		{
			struct Agent1 : opack::Agent {};
			auto e1 = check.template operator()<opack::Agent, Agent1>(sim.register_agent_type<Agent1>());
			struct Agent2 : opack::Agent {};
			auto e2 = check.template operator()<opack::Agent, Agent2>(sim.register_agent_type<Agent2>());
			CHECK(e1 != e2);
			struct Agent3 : opack::Agent {};
			auto e3 = check.template operator()<opack::Agent, Agent3>(sim.register_agent_type<Agent3>());
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Artefact1 : opack::Artefact {};
			auto e1 = check.template operator()<opack::Artefact, Artefact1>(sim.register_artefact_type<Artefact1>());
			struct Artefact2 : opack::Artefact {};
			auto e2 = check.template operator()<opack::Artefact, Artefact2>(sim.register_artefact_type<Artefact2>());
			CHECK(e1 != e2);
			struct Artefact3 : opack::Artefact {};
			auto e3 = check.template operator()<opack::Artefact, Artefact3>(sim.register_artefact_type<Artefact3>());
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Action1 : opack::Action {};
			auto e1 = check.template operator()<opack::Action, Action1>(sim.register_action<Action1>());
			struct Action2 : opack::Action {};
			auto e2 = check.template operator()<opack::Action, Action2>(sim.register_action<Action2>());
			CHECK(e1 != e2);
			struct Action3 : opack::Action {};
			auto e3 = check.template operator()<opack::Action, Action3>(sim.register_action<Action3>());
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Actuator1 : opack::Actuator {};
			auto e1 = check.template operator()<opack::Actuator, Actuator1>(sim.register_actuator_type<Actuator1>());
			struct Actuator2 : opack::Actuator {};
			auto e2 = check.template operator()<opack::Actuator, Actuator2>(sim.register_actuator_type<Actuator2>());
			CHECK(e1 != e2);
			struct Actuator3 : opack::Actuator {};
			auto e3 = check.template operator()<opack::Actuator, Actuator3>(sim.register_actuator_type<Actuator3>());
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Sense1 : opack::Sense {};
			auto e1 = check.template operator()<opack::Sense, Sense1>(sim.register_sense<Sense1>());
			struct Sense2 : opack::Sense {};
			auto e2 = check.template operator()<opack::Sense, Sense2>(sim.register_sense<Sense2>());
			CHECK(e1 != e2);
			struct Sense3 : opack::Sense {};
			auto e3 = check.template operator()<opack::Sense, Sense3>(sim.register_sense<Sense3>());
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}
	}

	SUBCASE("Agents")
	{
		size_t nb_agents{ 0 };
		size_t nb_prefabs{ 0 };
		auto check_construction = [&nb_agents, &nb_prefabs, &sim]<std::derived_from<opack::Agent> T = opack::Agent>(const char * name = "") -> flecs::entity
		{
			auto e = sim.agent<T>(name);
			nb_agents++;
			if(name)
				CHECK(strcmp(e.name().c_str(), name) == 0);

			CHECK(e.template is_a<opack::Agent>());
			CHECK(e.template has<opack::Agent>());
			CHECK(e.template has<T>());
			CHECK(e.template is_a<T>());
			CHECK(sim.count(flecs::IsA, sim.id<opack::Agent>()) == nb_agents);
			CHECK(sim.count<opack::Agent>() == nb_agents + nb_prefabs);
			return e;
		};

		// Agent without name
		auto agent_1 = check_construction.template operator()<>();

		// Agent with different ids, even without name
		auto agent_2 = check_construction.template operator()<>();
		auto agent_3 = check_construction.template operator()<>();
		CHECK(agent_1 != agent_2);
		CHECK(agent_2 != agent_3);
		CHECK(agent_1 != agent_3);

		// Agent of different types
		struct AgentA : opack::Agent {};
		auto a_t = sim.register_agent_type<AgentA>();
		nb_prefabs++; // Sadly prefab are also counted, except the root "Agent" :thinking:. 

		auto agent_a_1 = check_construction.template operator()<AgentA>();
		CHECK(sim.count<AgentA>() == 1 + 1); // Plus the prefab so.

		// Agent with name
		auto agent_named_1 = check_construction.template operator()<>("Agent_1");
		auto agent_named_2 = check_construction.template operator()<>("Agent_2");
		CHECK(agent_named_1 != agent_named_2);
	}

	SUBCASE("Artefacts")
	{
		// TODO Same tests than agents.
		// Artefact without name
		auto artefact_1 = sim.artefact();
		CHECK(artefact_1.is_a<opack::Artefact>());
		CHECK(artefact_1.has<opack::Artefact>());
		CHECK(sim.count(flecs::IsA, sim.id<opack::Artefact>()) == 1);
		CHECK(sim.count<opack::Artefact>() == 1);

		// Artefact with different ids, even without name
		auto artefact_2 = sim.artefact();
		auto artefact_3 = sim.artefact();
		CHECK(artefact_1 != artefact_2);
		CHECK(artefact_2 != artefact_3);
		CHECK(artefact_1 != artefact_3);
		CHECK(sim.count(flecs::IsA, sim.id<opack::Artefact>()) == 3);
		CHECK(sim.count<opack::Artefact>() == 3);

		// Artefact of different types
		struct ArtefactA : opack::Artefact {};
		auto a_t = sim.register_artefact_type<ArtefactA>();

		auto artefact_a = sim.artefact<ArtefactA>();
		CHECK(artefact_a.is_a<ArtefactA>());
		CHECK(artefact_a.has<ArtefactA>());
		CHECK(sim.count(flecs::IsA, sim.id<opack::Artefact>()) == 4);
		CHECK(sim.count(flecs::IsA, sim.id<ArtefactA>()) == 1);
		CHECK(sim.count<ArtefactA>() == 1);

		// Artefact with name
		auto artefact = sim.artefact("Some artefact");
		CHECK(strcmp(artefact.name().c_str(), "Some artefact") == 0);
	}

	SUBCASE("Percepts")
	{
		auto agent_1 = sim.agent("Agent_1");
		auto artefact_1 = sim.artefact("Artefact_1");

		// Add a sense
		struct MySense : opack::Sense {};
		auto sense = sim.register_sense<MySense>();
		CHECK(sense.has<MySense>());

		// Add a perceivable component by sense
		struct A { float value{ 0 }; };
		struct B { int value{ 0 }; };
		struct C { double value{ 0 }; }; // Not perceived by MySense
		struct R { };

		sim.perceive<MySense, A>();
		sim.perceive<MySense, B>();
		sim.perceive<MySense, R>();
		// Visualisation
		// =============
		// Mysense			--Sense-->		A
		//					--Sense-->		B
		//					--Sense-->		R
		CHECK(sense.has<opack::Sense, A>());
		CHECK(sense.has<opack::Sense, B>());
		CHECK(!sense.has<opack::Sense, C>());
		CHECK(sense.has<opack::Sense, R>());

		// Add the component to some entities
		agent_1.set<A>({ 2.0f }); // perceivable
		agent_1.set<B>({ 2 }); // not perceivable
		artefact_1.set<A>({ 3.0f }); //perceivable
		artefact_1.set<B>({ 3 }); //perceivable

		// Tells that the agent can now perceive the artefact thourgh MySense.
		sim.perceive<MySense>(agent_1, artefact_1);

		// Visualisation
		// =============
		// Mysense			--Sense-->		A
		//					--Sense-->		B
		//					--Sense-->		R
		// artefact_1		[A, B]
		// agent_1			[A, B]
		//					--MySense-->	artefact_1
		CHECK(agent_1.has<MySense>(artefact_1));
		CHECK(sim.count<MySense>(flecs::Wildcard) == 1);
		// CHECK(sim.count<opack::Percept>(flecs::Wildcard) == 1); // Sadly, there is no transitivity 'count'. 
		auto percepts = sim.query_percepts<MySense>(agent_1);
		for (auto p : percepts)
		{
			CHECK(p.use<MySense>());
			CHECK(p.subject == artefact_1);
			if (p.is_pred<A>())
				CHECK(p.fetch<A>()->value == 3.0f);
			else if (p.is_pred<B>())
				CHECK(p.fetch<B>()->value == 3);
			else
				CHECK_MESSAGE(false, "A percept do not have a correct value ! Check above test.");
		}

		// Multiple add shouldn't be taken into account
		sim.perceive<MySense>(agent_1, artefact_1);
		sim.perceive<MySense>(agent_1, artefact_1);
		sim.perceive<MySense>(agent_1, artefact_1);
		CHECK(sim.count<MySense>(flecs::Wildcard) == 1);

		// Retrieve percept only for a specific agent
		auto agent_2 = sim.agent("Agent_2");
		auto artefact_2 = sim.agent("Artefact_2");
		agent_2.add<R>(artefact_1);
		agent_2.add<R>(artefact_2);
		sim.perceive<MySense>(agent_2, agent_1);
		sim.perceive<MySense>(agent_1, agent_2);

		// Visualisation
		// =============
		// Mysense			--Sense-->		A
		//					--Sense-->		B
		//					--Sense-->		R
		// artefact_1		[A, B]
		// agent_1			[A, B]
		//					--MySense-->	artefact_1
		//					--MySense-->	agent_2
		// artefact_2		
		// agent_2		    --R-->		    artefact_1	
		//        		    --R-->		    artefact_2	
		//					--MySense-->	agent_1
		CHECK(sim.count<MySense>(flecs::Wildcard) == 3);
		percepts = sim.query_percepts<MySense>(agent_1);
		for (auto p : percepts)
		{
			CHECK(p.use<MySense>());
		}

		CHECK(percepts.size() == 4);
		CHECK(sim.query_percepts<MySense>(agent_2).size() == 2);

		// Deletion 
		// ======== 
		// Mysense			--Sense-->		A
		//					--Sense-->		B
		//					--Sense-->		R
		// artefact_1		[A, B]
		// artefact_2		
		// agent_2		    --R-->		    artefact_1	
		//        		    --R-->		    artefact_2	
		agent_1.destruct(); // All percepts (of this agent) should be also deleted (and anything associated with them).
		// Check deletion of percepts when object is removed
		CHECK(sim.query_percepts(agent_2).size() == 0);
		CHECK(sim.count<MySense>(flecs::Wildcard) == 0);
	}

	SUBCASE("Actions")
	{
		auto arthur		= sim.agent("Arthur");
		auto radio		= sim.agent("Radio");
		auto beatrice	= sim.agent("Beatrice");

		// Define actions
		// ===============
		struct Help : opack::Action {};
		auto help = sim.register_action<Help>();
		help.add<opack::Continuous>();
		CHECK(help.is_a<opack::Action>());

		// Define actuator
		// ===============
		struct UpperBody : opack::Actuator {};
		auto upper_body_actuator = sim.register_actuator_type<UpperBody>();
		CHECK(upper_body_actuator.has(flecs::Exclusive));
		CHECK(upper_body_actuator.has(flecs::OneOf, sim.entity<opack::Action>()));

		struct LowerBody : opack::Actuator {};
		auto lower_body_actuator = sim.register_actuator_type<LowerBody>();
		CHECK(lower_body_actuator.has(flecs::Exclusive));
		CHECK(lower_body_actuator.has(flecs::OneOf, sim.entity<opack::Action>()));

		// Launch actions
		// ==============
		auto help_inst = sim.action<Help>();
		CHECK(help_inst.has(flecs::IsA, sim.entity<Help>()));
		CHECK(help_inst.has(flecs::IsA, sim.entity<opack::Action>()));

		sim.act<LowerBody>(arthur, help_inst);

		//CHECK(arthur.has<LowerBody>(radio));
		//arthur.add<UpperBody>(radio);
		//CHECK(arthur.has<UpperBody>(radio));
		//CHECK(arthur.has<LowerBody>(radio)); // Check if multiple different actuator can be added

		//arthur.add<LowerBody>(beatrice);
		//CHECK(!arthur.has<LowerBody>(radio)); // Check if relation is exclusive
		//CHECK(arthur.has<LowerBody>(beatrice));
		//arthur.add<UpperBody>(beatrice);
		//CHECK(!arthur.has<UpperBody>(radio)); // Check if relation is exclusive
		//CHECK(arthur.has<UpperBody>(beatrice));
	}
}

TEST_SUITE_END();
