#include <doctest/doctest.h>
#include <opack/core.hpp>



TEST_CASE("Action API")
{
    OPACK_AGENT(MyAgent);
    OPACK_ACTUATOR(MyActuator);
    OPACK_ACTION(MoveTo);

    struct HasBegun {};
    struct HasUpdated {};
    struct HasEnded {};

    int counter = 0;

    auto world = opack::create_world();
    opack::init<MyAgent>(world);
    opack::init<MyActuator>(world);
    opack::init<MoveTo>(world).require<MyActuator>();
    opack::add_actuator<MyActuator, MyAgent>(world);
    auto e1  = opack::spawn<MyAgent>(world);

    auto action = opack::spawn<MoveTo>(world);
    opack::act(e1, action);
    CHECK(opack::current_action<MyActuator>(e1) == action);

    opack::on_action_begin<MoveTo>(world, [](opack::Entity action)
        {
            opack::initiator(action).add<HasBegun>();
        }
    );
    opack::on_action_update<MoveTo>(world, [&counter](opack::Entity action, float delta_time)
        {
            counter++;
            opack::initiator(action).add<HasUpdated>();
        }
    );
    opack::on_action_end<MoveTo>(world, [](opack::Entity action)
        {
            opack::initiator(action).add<HasEnded>();
        }
    );

    opack::step(world);
    CHECK(e1.has<HasBegun>());
    CHECK(e1.has<HasUpdated>());
    CHECK(e1.has<HasEnded>());
    CHECK(!action.is_valid());
    CHECK(counter == 1);

    MESSAGE("Checking action status w/ do not clean.");
    action = opack::spawn<MoveTo>(world).add<opack::DoNotClean>();
    auto e2  = opack::spawn<MyAgent>(world);
    opack::act(e2, action);
    opack::step(world);

    CHECK(e2.has<HasBegun>());
    CHECK(e2.has<HasUpdated>());
    CHECK(e2.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(action.has<opack::End, opack::Timestamp>());

    MESSAGE("Delay");
    action = opack::spawn<MoveTo>(world)
        .add<opack::DoNotClean>()
        .set<opack::Delay>({3.0})
    ;
    auto e3  = opack::spawn<MyAgent>(world);
    opack::act(e3, action);
    opack::step(world, 2.9f);
    CHECK(!e3.has<HasBegun>());
    CHECK(!e3.has<HasUpdated>());
    CHECK(!e3.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(!action.has<opack::Begin, opack::Timestamp>());
    CHECK(!action.has<opack::End, opack::Timestamp>());

    opack::step(world, 0.1f);
    CHECK(e3.has<HasBegun>());
    CHECK(e3.has<HasUpdated>());
    CHECK(e3.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(action.has<opack::End, opack::Timestamp>());

    MESSAGE("Timer");
    action = opack::spawn<MoveTo>(world)
        .add<opack::DoNotClean>()
        .set<opack::Timer>({3.0})
    ;
    opack::on_action_update<MoveTo>(world, [&counter](opack::Entity action, float delta_time)
        {
            counter++;
            CHECK(delta_time == 1.0f);
            opack::initiator(action).add<HasUpdated>();
        }
    );
    auto e4  = opack::spawn<MyAgent>(world);
    opack::act(e4, action);

    counter = 0;
    opack::step_n(world, 2, 1.0f);
    CHECK(e4.has<HasBegun>());
    CHECK(e4.has<HasUpdated>());
    CHECK(!e4.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(!action.has<opack::End, opack::Timestamp>());
    CHECK(counter == 2);

    opack::step(world, 1.0f);
    CHECK(e4.has<HasBegun>());
    CHECK(e4.has<HasUpdated>());
    CHECK(e4.has<HasEnded>());
    CHECK(action.is_valid());
    CHECK(action.has<opack::Begin, opack::Timestamp>());
    CHECK(action.has<opack::End, opack::Timestamp>());
    CHECK(counter == 3);
}