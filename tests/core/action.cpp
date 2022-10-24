#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <opack/module/simple_agent.hpp>

TEST_CASE("Action API")
{
    MESSAGE("Setup");
    OPACK_ACTION(MoveTo);

    struct HasBegun {};
    struct HasUpdated {};
    struct HasEnded {};

    auto world = opack::create_world();
    world.import<simple>();
    auto e1  = opack::spawn<simple::Agent>(world);
    opack::init<MoveTo>(world)
        .require<simple::Actuator>()
        .on_action_begin<opack::Action>([](flecs::entity action) { opack::initiator(action).add<HasBegun>(); })
        .on_action_update<opack::Action>([](flecs::entity action, float dt) { opack::initiator(action).add<HasUpdated>(); })
        .on_action_end<opack::Action>([](flecs::entity action) { opack::initiator(action).add<HasEnded>(); })
	;

    MESSAGE("One-shot action");
    auto action = opack::spawn<MoveTo>(world);
    opack::act(e1, action);
    CHECK(opack::current_action<simple::Actuator>(e1) == action);
    opack::step(world);
    CHECK(e1.has<HasBegun>());
    CHECK(e1.has<HasUpdated>());
    CHECK(e1.has<HasEnded>());
    CHECK(!action.is_valid());

    MESSAGE("One-shot action w/ DoNotClean");
    action = opack::spawn<MoveTo>(world).add<opack::DoNotClean>();
    auto e2  = opack::spawn<simple::Agent>(world);
    opack::act(e2, action);
    CHECK(opack::current_action<simple::Actuator>(e2) == action);

    opack::step(world);
    CHECK(e2.has<HasBegun>());
    CHECK(e2.has<HasUpdated>());
    CHECK(e2.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(action.has<opack::End, opack::Timestamp>());
    CHECK(opack::action_status(action) == opack::ActionStatus::finished);

    MESSAGE("Continous action");
    action = opack::spawn<MoveTo>(world)
        .set<opack::Duration>({3.0})
    ;
    auto e3  = opack::spawn<simple::Agent>(world);
    opack::act(e3, action);
    CHECK(opack::effective_duration(action) == 0.0f);

    opack::step(world);
    CHECK(e3.has<HasBegun>());
    CHECK(e3.has<HasUpdated>());
    CHECK(!e3.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(!action.has<opack::End, opack::Timestamp>());
    CHECK(opack::action_status(action) == opack::ActionStatus::running);
    CHECK(opack::effective_duration(action) == doctest::Approx(0.0).epsilon(0.1));

    opack::step(world, 1.0f);
    CHECK(e3.has<HasBegun>());
    CHECK(e3.has<HasUpdated>());
    CHECK(!e3.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(!action.has<opack::End, opack::Timestamp>());
    CHECK(opack::action_status(action) == opack::ActionStatus::running);
    CHECK(opack::effective_duration(action) == doctest::Approx(1.0).epsilon(0.1));

	e3.remove<HasUpdated>();
    opack::step(world, 2.f);
    opack::step(world); // We didn't add a sync point for this use case
    CHECK(e3.has<HasBegun>());
    CHECK(e3.has<HasUpdated>());
    CHECK(e3.has<HasEnded>());
    CHECK(!action.is_valid());

    MESSAGE("Continous action w/ DoNotClean");
    action = opack::spawn<MoveTo>(world)
        .add<opack::DoNotClean>()
        .set<opack::Duration>({3.0})
    ;
    auto e4  = opack::spawn<simple::Agent>(world);
    opack::act(e4, action);
    CHECK(opack::effective_duration(action) == 0.0f);

    opack::step(world);
    CHECK(e4.has<HasBegun>());
    CHECK(e4.has<HasUpdated>());
    CHECK(!e4.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(!action.has<opack::End, opack::Timestamp>());
    CHECK(opack::action_status(action) == opack::ActionStatus::running);
    CHECK(opack::effective_duration(action) == doctest::Approx(0.0).epsilon(0.1));

    opack::step(world, 1.0f);
    CHECK(e4.has<HasBegun>());
    CHECK(e4.has<HasUpdated>());
    CHECK(!e4.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(!action.has<opack::End, opack::Timestamp>());
    CHECK(opack::action_status(action) == opack::ActionStatus::running);
    CHECK(opack::effective_duration(action) == doctest::Approx(1.0).epsilon(0.1));

	e4.remove<HasUpdated>();
    opack::step(world, 2.f);
    opack::step(world); // We didn't add a sync point for this use case
    CHECK(e4.has<HasBegun>());
    CHECK(e4.has<HasUpdated>());
    CHECK(e4.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(action.has<opack::End, opack::Timestamp>());
    CHECK(opack::action_status(action) == opack::ActionStatus::finished);
    CHECK(opack::effective_duration(action) == doctest::Approx(3.0).epsilon(0.1));

    MESSAGE("Continous action 0.0f ending in same cycle");
    action = opack::spawn<MoveTo>(world)
        .set<opack::Duration>({0.0})
    ;
    auto e5  = opack::spawn<simple::Agent>(world);
    opack::act(e5, action);
    CHECK(opack::effective_duration(action) == 0.0f);

    opack::step(world);
    CHECK(e5.has<HasBegun>());
    CHECK(e5.has<HasUpdated>());
    CHECK(e5.has<HasEnded>());
    CHECK(!action.is_valid());

    MESSAGE("Continous action 5.0f s ending in same cycle");
    action = opack::spawn<MoveTo>(world)
        .set<opack::Duration>({5.0})
    ;
    auto e6  = opack::spawn<simple::Agent>(world);
    opack::act(e6, action);
    CHECK(opack::duration(action) == 5.0f);

    opack::step(world, 4.9f);
    CHECK(opack::effective_duration(action) == doctest::Approx(4.9f));
    opack::step(world, 0.1f);
    CHECK(e6.has<HasBegun>());
    CHECK(e6.has<HasUpdated>());
    CHECK(e6.has<HasEnded>());
    CHECK(!action.is_valid());
}

TEST_CASE("Action API : tracking")
{
    OPACK_ACTION(SomeAction);

    auto world = opack::create_world();
    world.import<simple>();
    auto action_prefab = opack::init<SomeAction>(world).require<simple::Actuator>();

    int buffer_size{ 0 };
    SUBCASE("Buffer size 1"){ buffer_size = 1;}
    SUBCASE("Buffer size 3"){ buffer_size = 3;}
	opack::entity<simple::Actuator>(world).track(buffer_size);

    auto e1 = opack::spawn<simple::Agent>(world, "my_agent");


    auto action = spawn(action_prefab);
    opack::act(e1, action);

    CHECK(opack::current_action<simple::Actuator>(e1) == action);
    CHECK(opack::is_a<SomeAction>(opack::current_action<simple::Actuator>(e1) ));

    opack::step(world);

    CHECK(opack::has_done<SomeAction>(e1));
    CHECK(opack::last_action_prefab<simple::Actuator>(e1) == action_prefab);
}
