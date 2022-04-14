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
		auto e_2 = opack::entity<Test>(sim);
		CHECK(e_1 == e_2);

		auto c_1 = sim.world.component<Test>();
		CHECK(c_1 == e_2);

		auto p_1 = sim.world.prefab<Test>();
		CHECK(p_1 == e_2);

		CHECK(e_2 == opack::id<Test>(sim));
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
			auto e1 = check.template operator()<opack::Agent, Agent1>(register_agent_type<Agent1>(sim));
			struct Agent2 : opack::Agent {};
			auto e2 = check.template operator()<opack::Agent, Agent2>(register_agent_type<Agent2>(sim));
			CHECK(e1 != e2);
			struct Agent3 : opack::Agent {};
			auto e3 = check.template operator()<opack::Agent, Agent3>(register_agent_type<Agent3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Artefact1 : opack::Artefact {};
			auto e1 = check.template operator()<opack::Artefact, Artefact1>(register_artefact_type<Artefact1>(sim));
			struct Artefact2 : opack::Artefact {};
			auto e2 = check.template operator()<opack::Artefact, Artefact2>(register_artefact_type<Artefact2>(sim));
			CHECK(e1 != e2);
			struct Artefact3 : opack::Artefact {};
			auto e3 = check.template operator()<opack::Artefact, Artefact3>(register_artefact_type<Artefact3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Action1 : opack::Action {};
			auto e1 = check.template operator()<opack::Action, Action1>(register_action<Action1>(sim));
			struct Action2 : opack::Action {};
			auto e2 = check.template operator()<opack::Action, Action2>(register_action<Action2>(sim));
			CHECK(e1 != e2);
			struct Action3 : opack::Action {};
			auto e3 = check.template operator()<opack::Action, Action3>(register_action<Action3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Actuator1 : opack::Actuator {};
			auto e1 = check.template operator()<opack::Actuator, Actuator1>(register_actuator_type<Actuator1>(sim));
			struct Actuator2 : opack::Actuator {};
			auto e2 = check.template operator()<opack::Actuator, Actuator2>(register_actuator_type<Actuator2>(sim));
			CHECK(e1 != e2);
			struct Actuator3 : opack::Actuator {};
			auto e3 = check.template operator()<opack::Actuator, Actuator3>(register_actuator_type<Actuator3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Sense1 : opack::Sense {};
			auto e1 = check.template operator()<opack::Sense, Sense1>(register_sense<Sense1>(sim));
			struct Sense2 : opack::Sense {};
			auto e2 = check.template operator()<opack::Sense, Sense2>(register_sense<Sense2>(sim));
			CHECK(e1 != e2);
			struct Sense3 : opack::Sense {};
			auto e3 = check.template operator()<opack::Sense, Sense3>(register_sense<Sense3>(sim));
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
			auto e = agent<T>(sim, name);
			nb_agents++;
			if(name)
				CHECK(strcmp(e.name().c_str(), name) == 0);

			CHECK(e.template is_a<opack::Agent>());
			CHECK(e.template has<opack::Agent>());
			CHECK(e.template has<T>());
			CHECK(e.template is_a<T>());
			CHECK(sim.count(flecs::IsA, opack::id<opack::Agent>(sim)) == nb_agents);
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
		auto a_t = register_agent_type<AgentA>(sim);
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
		auto artefact_1 = artefact(sim);
		CHECK(artefact_1.is_a<opack::Artefact>());
		CHECK(artefact_1.has<opack::Artefact>());
		CHECK(sim.count(flecs::IsA, opack::id<opack::Artefact>(sim)) == 1);
		CHECK(sim.count<opack::Artefact>() == 1);

		// Artefact with different ids, even without name
		auto artefact_2 = artefact(sim);
		auto artefact_3 = artefact(sim);
		CHECK(artefact_1 != artefact_2);
		CHECK(artefact_2 != artefact_3);
		CHECK(artefact_1 != artefact_3);
		CHECK(sim.count(flecs::IsA, opack::id<opack::Artefact>(sim)) == 3);
		CHECK(sim.count<opack::Artefact>() == 3);

		// Artefact of different types
		struct ArtefactA : opack::Artefact {};
		auto a_t = register_artefact_type<ArtefactA>(sim);

		auto artefact_a = artefact<ArtefactA>(sim);
		CHECK(artefact_a.is_a<ArtefactA>());
		CHECK(artefact_a.has<ArtefactA>());
		CHECK(sim.count(flecs::IsA, id<opack::Artefact>(sim)) == 4);
		CHECK(sim.count(flecs::IsA, id<ArtefactA>(sim)) == 1);
		CHECK(sim.count<ArtefactA>() == 1);

		// Artefact with name
		auto artefact = opack::artefact(sim, "Some artefact");
		CHECK(strcmp(artefact.name().c_str(), "Some artefact") == 0);
	}

	SUBCASE("Percepts")
	{
		auto agent_1 = opack::agent(sim, "Agent_1");
		auto artefact_1 = opack::artefact(sim, "Artefact_1");

		// Add a sense
		struct MySense : opack::Sense {};
		auto sense = opack::register_sense<MySense>(sim);
		CHECK(sense.has<MySense>());

		// Add a perceivable component by sense
		struct A { float value{ 0 }; };
		struct B { int value{ 0 }; };
		struct C { double value{ 0 }; }; // Not perceived by MySense
		struct R { };

		opack::perceive<MySense, A>(sim);
		opack::perceive<MySense, B>(sim);
		opack::perceive<MySense, R>(sim);
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
		opack::perceive<MySense>(sim, agent_1, artefact_1);

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
		opack::perceive<MySense>(sim, agent_1, artefact_1);
		opack::perceive<MySense>(sim, agent_1, artefact_1);
		opack::perceive<MySense>(sim, agent_1, artefact_1);
		CHECK(sim.count<MySense>(flecs::Wildcard) == 1);

		// Retrieve percept only for a specific agent
		auto agent_2 = opack::agent(sim, "Agent_2");
		auto artefact_2 = opack::agent(sim, "Artefact_2");
		agent_2.add<R>(artefact_1);
		agent_2.add<R>(artefact_2);
		opack::perceive<MySense>(sim, agent_2, agent_1);
		opack::perceive<MySense>(sim, agent_1, agent_2);

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
		auto arthur		= opack::agent(sim, "Arthur");
		auto radio		= opack::agent(sim, "Radio");
		auto beatrice	= opack::agent(sim, "Beatrice");

		// Define actions
		// ===============
		struct Help : opack::Action {};
		auto help = opack::register_action<Help>(sim);
		CHECK(help.is_a<opack::Action>());

		// Define actuator
		// ===============
		struct UpperBody : opack::Actuator {};
		auto upper_body_actuator = opack::register_actuator_type<UpperBody>(sim);
		CHECK(upper_body_actuator.has(flecs::Exclusive));
		CHECK(upper_body_actuator.has(flecs::OneOf, opack::entity<opack::Action>(sim)));

		struct LowerBody : opack::Actuator {};
		auto lower_body_actuator = opack::register_actuator_type<LowerBody>(sim);
		CHECK(lower_body_actuator.has(flecs::Exclusive));
		CHECK(lower_body_actuator.has(flecs::OneOf, opack::entity<opack::Action>(sim)));

		// Launch actions
		// ==============
		auto help_inst = opack::action<Help>(sim);
		CHECK(help_inst.has(flecs::IsA, opack::entity<Help>(sim)));
		CHECK(help_inst.has(flecs::IsA, opack::entity<opack::Action>(sim)));

		//Action without initiator should be deleted
		CHECK(help_inst.is_alive());
		sim.step();
		CHECK(!help_inst.is_alive());

		// So recreate it and use it before next step().
		help_inst = opack::action<Help>(sim);

		opack::act<LowerBody>(sim, arthur, help_inst);
		opack::act<LowerBody>(sim, arthur, upper_body_actuator);

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
