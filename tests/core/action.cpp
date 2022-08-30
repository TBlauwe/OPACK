#include <doctest/doctest.h>
#include <opack/core.hpp>



TEST_CASE("Action API")
{
    OPACK_SUB_PREFAB(MyAgent, opack::Agent);
    OPACK_ACTUATOR(MyActuator);
    OPACK_ACTION(MoveTo);
    struct Done {};

    auto world = opack::create_world();
    opack::batch_init<
        MyAgent,
        MyActuator,
        MoveTo
    >(world);
    opack::add_actuator<MyActuator, MyAgent>(world);
    auto e1  = opack::spawn<MyAgent>(world);

    auto action = opack::spawn<MoveTo>(world);
    opack::act<MyActuator>(e1, action);
    CHECK(opack::current_action<MyActuator>(e1) == action);

    opack::on_action_begin<MoveTo>(world, [](opack::Entity agent, opack::Entity actuator, opack::Entity action)
        {
            agent.add<Done>();
        }
    );

    opack::step(world);
    CHECK(e1.has<Done>());
}