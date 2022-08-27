#include <doctest/doctest.h>
#include <opack/core.hpp>

OPACK_SUB_PREFAB(MyAgent, opack::Agent);
OPACK_ACTUATOR(MyActuator);
OPACK_ACTION(MoveTo);

struct Location { opack::Entity value; };

TEST_CASE("Action API")
{
    // Initialization
    auto world = opack::create_world();
    opack::batch_init<
        MyAgent,
        MyActuator,
        MoveTo
    >(world);
    opack::add_actuator<MyActuator, MyAgent>(world);
    auto e1  = opack::spawn<MyAgent>(world);
    auto e2  = opack::spawn<MyAgent>(world);
    auto e3  = opack::spawn<MyAgent>(world);

    auto action = opack::spawn<MoveTo>(world);
    opack::act<MyActuator>(e1, action);
}