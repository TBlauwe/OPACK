#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <string.h> // For const char * comparisons


void test_step(opack::World& world, size_t n = 1, float delta_time = 1.0f, float time_scale = 1.0f)
{
	float total_time{ n * (delta_time * time_scale) };
	sim.time_scale(time_scale);
	if (n == 1)
		sim.step(delta_time);
	else
		sim.step_n(n, delta_time);
	CHECK(sim.tick() == n);
	CHECK(sim.delta_time() == time_scale * delta_time);
	CHECK(sim.time() >= total_time);
}

TEST_CASE("Simulation API")
{
	opack::Simulation sim;
	REQUIRE(sim.tick() == 0);
	REQUIRE(sim.time() == 0.0f);

	REQUIRE(sim.world.template has<opack::concepts>());
	REQUIRE(sim.world.template has<opack::dynamics>());

	SUBCASE("Configuration")
	{
		{
			float target_fps = 10.f;
			sim.target_fps(target_fps);
			CHECK(sim.target_fps() == target_fps);
			CHECK(sim.world.get_target_fps() == target_fps);

			float time_scale = 0.5f;
			sim.time_scale(time_scale);
			CHECK(sim.time_scale() == time_scale);
			CHECK(sim.world.get_time_scale() == time_scale);
		}
		{
			float target_fps = 1.f;
			sim.target_fps(target_fps);
			CHECK(sim.target_fps() == target_fps);
			CHECK(sim.world.get_target_fps() == target_fps);

			float time_scale = 1.f;
			sim.time_scale(time_scale);
			CHECK(sim.time_scale() == time_scale);
			CHECK(sim.world.get_time_scale() == time_scale);
		}
	}

	SUBCASE("Controls")
	{
		SUBCASE("Step")
		{
			test_step(sim);
		}

		SUBCASE("Step_n")
		{
			test_step(sim, 10);
		}

		SUBCASE("Step w/ delta_time")
		{
			test_step(sim, 1, 0.5f);
		}

		SUBCASE("Step_n w/ delta_time")
		{
			test_step(sim, 10, 0.5f);
		}

		SUBCASE("Step w/ delta_time w/time_scale 0.5")
		{
			test_step(sim, 1, 0.5f, 0.5);
		}

		SUBCASE("Step_n w/ delta_time w/time_scale 0.5")
		{
			test_step(sim, 10, 0.5f, 0.5);
		}

		SUBCASE("Step w/ delta_time w/time_scale 2")
		{
			test_step(sim, 1, 2.f, 2.f);
		}

		SUBCASE("Step w/ delta_time w/time_scale 2")
		{
			test_step(sim, 10, 2.f, 2.f);
		}
	}

	SUBCASE("Entity id equality")
	{
		struct Test {};
		auto e_1 = sim.world.template entity<Test>();
		auto e_2 = opack::entity<Test>(sim);
		CHECK(e_1 == e_2);

		auto c_1 = sim.world.template component<Test>();
		CHECK(c_1 == e_2);

		auto p_1 = sim.world.template prefab<Test>();
		CHECK(p_1 == e_2);

		CHECK(e_2 == opack::id<Test>(sim));
	}

	SUBCASE("Prefab register")
	{
		auto check = []<typename Base, std::derived_from<Base> Derived>(flecs::entity e) -> flecs::entity
		{
			CHECK(e.template is_a<Base>());
			CHECK(e.template has<Base>());
			return e;
		};

		{
			struct Agent1 : opack::Agent {};
			auto e1 = check.template operator() < opack::Agent, Agent1 > (register_agent<Agent1>(sim));
			struct Agent2 : opack::Agent {};
			auto e2 = check.template operator() < opack::Agent, Agent2 > (register_agent<Agent2>(sim));
			CHECK(e1 != e2);
			struct Agent3 : opack::Agent {};
			auto e3 = check.template operator() < opack::Agent, Agent3 > (register_agent<Agent3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Artefact1 : opack::Artefact {};
			auto e1 = check.template operator() < opack::Artefact, Artefact1 > (register_artefact<Artefact1>(sim));
			struct Artefact2 : opack::Artefact {};
			auto e2 = check.template operator() < opack::Artefact, Artefact2 > (register_artefact<Artefact2>(sim));
			CHECK(e1 != e2);
			struct Artefact3 : opack::Artefact {};
			auto e3 = check.template operator() < opack::Artefact, Artefact3 > (register_artefact<Artefact3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Action1 : opack::Action {};
			auto e1 = check.template operator() < opack::Action, Action1 > (register_action<Action1>(sim));
			struct Action2 : opack::Action {};
			auto e2 = check.template operator() < opack::Action, Action2 > (register_action<Action2>(sim));
			CHECK(e1 != e2);
			struct Action3 : opack::Action {};
			auto e3 = check.template operator() < opack::Action, Action3 > (register_action<Action3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}
	}

	SUBCASE("Type register")
	{
		auto check = []<typename Base, std::derived_from<Base> Derived>(flecs::entity e) -> flecs::entity
		{
			CHECK(e.template is_a<Base>());
			return e;
		};

		{
			struct Actuator1 : opack::Actuator {};
			auto e1 = check.template operator() < opack::Actuator, Actuator1 > (register_actuator<Actuator1>(sim));
			struct Actuator2 : opack::Actuator {};
			auto e2 = check.template operator() < opack::Actuator, Actuator2 > (register_actuator<Actuator2>(sim));
			CHECK(e1 != e2);
			struct Actuator3 : opack::Actuator {};
			auto e3 = check.template operator() < opack::Actuator, Actuator3 > (register_actuator<Actuator3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}

		{
			struct Sense1 : opack::Sense {};
			auto e1 = check.template operator() < opack::Sense, Sense1 > (register_sense<Sense1>(sim));
			struct Sense2 : opack::Sense {};
			auto e2 = check.template operator() < opack::Sense, Sense2 > (register_sense<Sense2>(sim));
			CHECK(e1 != e2);
			struct Sense3 : opack::Sense {};
			auto e3 = check.template operator() < opack::Sense, Sense3 > (register_sense<Sense3>(sim));
			CHECK(e3 != e1);
			CHECK(e3 != e2);
		}
	}
}

template<typename Base, std::derived_from<Base> Derived = Base>
flecs::entity test_entity_creation(flecs::world& world, const char* name = "")
{
	size_t counter{ opack::count<Base>(world) };
	size_t second_counter{ opack::count<Derived>(world) };

	auto e = opack::instantiate<Derived>(world, name);
	counter++;
	second_counter++;
	if (name)
		CHECK(strcmp(e.doc_name(), name) == 0);

	CHECK(e.is_a(opack::prefab<Base>(world)));
	CHECK(e.template has<Base>());
	CHECK(e.is_a(opack::prefab<Derived>(world)));
	CHECK(e.template has<Derived>());
	// TODO CHECK(opack::count(world, flecs::IsA, opack::prefab<Base>(world)) == counter);
	// TODO CHECK(opack::count(world, flecs::IsA, opack::prefab<Derived>(world)) == second_counter);
	CHECK(opack::count<Base>(world) == counter);
	CHECK(opack::count<Derived>(world) == second_counter);
	return e;
}

template<typename Base, std::derived_from<Base> Derived>
void test_entities_creation(flecs::world& world)
{
	// Entity without name
	auto e1 = test_entity_creation<Base>(world);

	// Entities with different ids, even without name
	auto e2 = test_entity_creation<Base>(world);
	auto e3 = test_entity_creation<Base>(world);
	CHECK(e1 != e2);
	CHECK(e2 != e3);
	CHECK(e1 != e3);

	auto e4 = test_entity_creation<Base, Derived>(world);

	// Entities with name
	auto ne1 = test_entity_creation<Base, Derived>(world, "entity_1");
	auto ne2 = test_entity_creation<Base, Derived>(world, "entity_2");
	CHECK(ne1 != ne2);
}

TEST_CASE_TEMPLATE_DEFINE("Simulation entity construction", T, sim_entity_construction)
{
	auto sim = T();
	REQUIRE(sim.world.tick() == 0);

	REQUIRE(sim.world.template has<opack::concepts>());
	REQUIRE(sim.world.template has<opack::dynamics>());

	REQUIRE(sim.world.template has<opack::queries::perception::Component>());
	REQUIRE(sim.world.template has<opack::queries::perception::Relation>());

	SUBCASE("Agents")
	{
		struct AgentA : opack::Agent {};
		opack::register_agent<AgentA>(sim);
		test_entities_creation<opack::Agent, AgentA>(sim);
	}

	SUBCASE("Artefacts")
	{
		struct ArtefactA : opack::Artefact {};
		opack::register_artefact<ArtefactA>(sim);
		test_entities_creation<opack::Artefact, ArtefactA>(sim);
	}
}

TEST_CASE_TEMPLATE_DEFINE("Simulation manipulation", T, sim_manipulation)
{
	auto sim = opack::Simulation();
	//auto sim = T();
	REQUIRE(sim.world.tick() == 0);

	REQUIRE(sim.world.template has<opack::concepts>());
	REQUIRE(sim.world.template has<opack::dynamics>());

	REQUIRE(sim.world.template has<opack::queries::perception::Component>());
	REQUIRE(sim.world.template has<opack::queries::perception::Relation>());

	// Add a sense
	struct MySense : opack::Sense {};
	auto sense = opack::register_sense<MySense>(sim);

	// Add perceivable component by sense
	struct A { float value{ 0 }; };
	struct B { int value{ 0 }; };
	struct C { double value{ 0 }; }; // Not perceived by MySense
	struct R { };
	opack::perceive<MySense, A>(sim);
	opack::perceive<MySense, B>(sim);
	opack::perceive<MySense, R>(sim);

	REQUIRE(sense.template has<opack::Sense, A>());
	REQUIRE(sense.template has<opack::Sense, B>());
	REQUIRE(!sense.template has<opack::Sense, C>());
	REQUIRE(sense.template has<opack::Sense, R>());

	SUBCASE("Perception")
	{
		auto agent_1 = opack::agent(sim);
		auto agent_2 = opack::agent(sim);
		auto artefact_1 = opack::artefact(sim);
		auto artefact_2 = opack::agent(sim);

		agent_1.set<A>({ 2.0f });	// perceivable
		agent_1.set<C>({ 2.0 });	// not perceivable
		agent_2.add<R>(artefact_1);
		agent_2.add<R>(artefact_2);
		artefact_1.set<A>({ 3.0f });// perceivable
		artefact_1.set<B>({ 3 });	// perceivable
		artefact_1.set<C>({ 3.0 });	// not perceivable

		// Tells that the agent can now perceive the artefact thourgh MySense.
		opack::perceive<MySense>(agent_1, artefact_1);

		// Visualisation
		// -------------
		// Mysense			--Sense-->		A
		//					--Sense-->		B
		//					--Sense-->		R
		// artefact_1		[A, B, C]
		// artefact_2		
		// agent_1			[A, C]
		//					--MySense-->	artefact_1
		// agent_2		    --R-->		    artefact_1	
		CHECK(agent_1.template has<MySense>(artefact_1));
		CHECK(opack::count<MySense>(sim, flecs::Wildcard) == 1);
		//CHECK(sim.count<opack::Sense>(flecs::Wildcard) == 1); // Sadly, there is no transitivity 'count'. 
		auto percepts = opack::query_percepts<MySense>(agent_1);
		for (auto p : percepts)
		{
			CHECK(p.template with_sense<MySense>());
			CHECK(p.subject_is(artefact_1));
			if (p.template predicate_is<A>())
				CHECK(p.template value<A>()->value == 3.0f);
			else if (p.template predicate_is<B>())
				CHECK(p.template value<B>()->value == 3);
			else
				CHECK_MESSAGE(false, "A percept do not have a correct value ! Check above test.");
		}
		//opack::each_perceived<opack::Sense, opack::Artefact>(e1, [&artefact_1](flecs::entity subject) { CHECK(subject == artefact_1); });
		//opack::each_perceived<MySense, opack::Artefact>(e1, [&artefact_1](flecs::entity subject) { CHECK(subject == artefact_1); });
		CHECK(opack::does_perceive(agent_1, artefact_1));
		CHECK(opack::does_perceive<void, MySense>(agent_1, artefact_1));
		CHECK(opack::does_perceive<A, MySense>(agent_1, artefact_1));
		CHECK(opack::does_perceive<A>(agent_1, artefact_1));
		CHECK(opack::does_perceive<B>(agent_1, artefact_1));
		CHECK(opack::does_perceive<B>(agent_1, artefact_1));
		CHECK(!opack::does_perceive<C>(agent_1, artefact_1));

		// Multiple add shouldn't be taken into account
		opack::perceive<MySense>(agent_1, artefact_1);
		opack::perceive<MySense>(agent_1, artefact_1);
		opack::perceive<MySense>(agent_1, artefact_1);
		CHECK(opack::count<MySense>(sim, flecs::Wildcard) == 1);

		// Retrieve percept only for a specific agent
		opack::perceive<MySense>(agent_2, agent_1);
		opack::perceive<MySense>(agent_1, agent_2);

		// Visualisation
		// -------------
		// Mysense			--Sense-->		A
		//					--Sense-->		B
		//					--Sense-->		R
		// artefact_1		[A, B, C]
		// e1			[A, C]
		//					--MySense-->	artefact_1
		//					--MySense-->	e2
		// artefact_2		
		// e2		    --R-->		    artefact_1	
		//        		    --R-->		    artefact_2	
		//					--MySense-->	e1
		CHECK(opack::count<MySense>(sim, flecs::Wildcard) == 3);
		percepts = opack::query_percepts<MySense>(agent_1);
		for (auto p : percepts)
		{
			CHECK(p.template with_sense<MySense>());
		}
		CHECK(percepts.size() == 4);

		CHECK(opack::does_perceive<R>(agent_1, agent_2));
		CHECK(opack::does_perceive<R>(agent_1, agent_2, artefact_1));
		CHECK(!opack::does_perceive<R>(agent_1, agent_2, artefact_2));
		CHECK(opack::query_percepts<MySense>(agent_2).size() == 1);

		// Visualisation
		// -------------
		// Mysense			--Sense-->		A
		//					--Sense-->		B
		//					--Sense-->		R
		// artefact_1		[A, B, C]
		// artefact_2		
		// e2		    --R-->		    artefact_1	
		//        		    --R-->		    artefact_2	
		agent_1.destruct(); // All percepts (of this agent) should be also deleted (and anything associated with them).
		// Check deletion of percepts when object is removed
		CHECK(opack::query_percepts(agent_2).size() == 0);
		CHECK(opack::count<MySense>(sim, flecs::Wildcard) == 0);
	}

	SUBCASE("Actions")
	{
		auto arthur = opack::agent(sim, "Arthur");
		auto radio = opack::agent(sim, "Radio");
		auto beatrice = opack::agent(sim, "Beatrice");

		// Define actions
		// ===============
		struct Help : opack::Action {};
		auto help = opack::register_action<Help>(sim);
		CHECK(opack::is_a<opack::Action>(help));

		// Define actuator
		// ===============
		struct UpperBody : opack::Actuator {};
		auto upper_body_actuator = opack::register_actuator<UpperBody>(sim);
		CHECK(upper_body_actuator.has(flecs::Exclusive));

		struct LowerBody : opack::Actuator {};
		auto lower_body_actuator = opack::register_actuator<LowerBody>(sim);
		CHECK(lower_body_actuator.has(flecs::Exclusive));

		// Launch actions
		// ==============
		auto help_inst = opack::action<Help>(sim);
		CHECK(opack::is_a<opack::Action>(help_inst));
		CHECK(opack::is_a<Help>(help_inst));

		//Action without initiator should be deleted
		CHECK(help_inst.is_alive());
		sim.step();
		CHECK(!help_inst.is_alive());

		// So recreate it and use it before next step().
		help_inst = opack::action<Help>(sim);

		opack::act<LowerBody>(arthur, help_inst);
		opack::act<LowerBody>(arthur, upper_body_actuator);

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

TEST_CASE_TEMPLATE_INVOKE(sim_construction, EmptySim, SimpleSim);
TEST_CASE_TEMPLATE_INVOKE(sim_entity_construction, EmptySim, SimpleSim);
TEST_CASE_TEMPLATE_INVOKE(sim_manipulation, EmptySim, SimpleSim);

TEST_SUITE_END();
