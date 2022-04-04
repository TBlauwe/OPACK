#include <doctest/doctest.h>
#include <opack/core/simulation.hpp>
#include <string.h> // For const char * comparisons

TEST_SUITE_BEGIN("Simulation");

TEST_CASE("Basics")
{
	auto sim = opack::Simulation();
	REQUIRE(sim.tick() == 0);
	REQUIRE(sim.count<opack::Agent>() == 0);
	REQUIRE(sim.count<opack::Percept>() == 0);

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
		struct PerceptA : opack::Percept {};
		sim.register_percept_type<PerceptA>();

		// Add percept
		auto agent = sim.agent();
		auto percept = sim.percept<PerceptA>(agent);
		CHECK(percept.has<PerceptA>());
		CHECK(percept.has(flecs::ChildOf, agent));
		CHECK(agent.has<PerceptA>(percept));
		CHECK(sim.count<PerceptA>() == 1);

		// Retrieve percept only for a specific agent
		auto another_agent = sim.agent();
		sim.percept<PerceptA>(another_agent);
		sim.percept<PerceptA>(another_agent);
		sim.percept<PerceptA>(another_agent);
		sim.percept<PerceptA>(agent);
		CHECK(sim.query_perceptions_of(agent).size() == 2);
		CHECK(sim.query_perceptions_of(another_agent).size() == 3);

		// Check deletion 
		agent.destruct(); // All percepts should be also deleted (and anything associated with them).
		CHECK(percept.is_alive() == false);
	}
}

TEST_SUITE_END();
