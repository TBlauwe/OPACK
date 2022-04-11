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

	SUBCASE("Agents")
	{
		// Agent without name
		auto agent_1 = sim.agent();
		CHECK(agent_1.has<opack::Agent>());
		CHECK(sim.count<opack::Agent>() == 1);

		// Agent with different ids, even without name
		auto agent_2 = sim.agent();
		auto agent_3 = sim.agent();
		CHECK(agent_1 != agent_2);
		CHECK(agent_2 != agent_3);
		CHECK(agent_1 != agent_3);
		CHECK(sim.count<opack::Agent>() == 3);

		// Agent of different types
		struct AgentA : opack::Agent {};
		sim.register_agent_type<AgentA>();
		auto agent_a = sim.agent<AgentA>();
		CHECK(agent_a.has<AgentA>());
		CHECK(sim.count<opack::Agent>() == 3);
		CHECK(sim.count<AgentA>() == 1);

		// Agent with name
		auto bob = sim.agent("Bob");
		CHECK(strcmp(bob.name().c_str(), "Bob") == 0);
	}

	SUBCASE("Artefacts")
	{
		// Artefact without name
		auto artefact_1 = sim.artefact();
		CHECK(artefact_1.has<opack::Artefact>());
		CHECK(sim.count<opack::Artefact>() == 1);

		// Artefact with different ids, even without name
		auto artefact_2 = sim.artefact();
		auto artefact_3 = sim.artefact();
		CHECK(artefact_1 != artefact_2);
		CHECK(artefact_2 != artefact_3);
		CHECK(artefact_1 != artefact_3);
		CHECK(sim.count<opack::Artefact>() == 3);

		// Artefact of different types
		struct ArtefactA : opack::Artefact {};
		sim.register_artefact_type<ArtefactA>();
		auto artefact_a = sim.artefact<ArtefactA>();
		CHECK(artefact_a.has<ArtefactA>());
		CHECK(sim.count<opack::Artefact>() == 3);
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
}

TEST_SUITE_END();
