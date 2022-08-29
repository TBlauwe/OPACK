#include <doctest/doctest.h>
#include <opack/core.hpp>

OPACK_SUB_PREFAB(MyAgent, opack::Agent);
OPACK_SUB_PREFAB(MySense, opack::Sense);

struct Test { float value{ 1.0 }; };
struct B {};
struct R {};

TEST_CASE("Perception API")
{
    // Initialization
    auto world = opack::create_world();
    opack::init<MyAgent>(world);
    auto my_sense = opack::init<MySense>(world);
    opack::add_sense<MySense, MyAgent>(world);
    auto e1  = opack::spawn<MyAgent>(world, "e1");
    auto e2  = opack::spawn<MyAgent>(world, "e2");
    auto e3  = opack::spawn<MyAgent>(world, "e3");

    CHECK(e1.has<MySense>(flecs::Wildcard));
    CHECK(e2.has<MySense>(flecs::Wildcard));
    CHECK(e3.has<MySense>(flecs::Wildcard));

    // Make sure that target are instantiated for each instance. 
    CHECK(e1.target<MySense>() != e2.target<MySense>());
    CHECK(e1.target<MySense>() != e3.target<MySense>());
    CHECK(e2.target<MySense>() != e3.target<MySense>());

    // Make sure that target is a MySense instance
    CHECK(opack::is_a<MySense>(e1.target<MySense>() ));
    CHECK(opack::is_a<MySense>(e2.target<MySense>() ));
    CHECK(opack::is_a<MySense>(e3.target<MySense>() ));

    opack::perceive<MySense>(e1, e2);
    auto p = opack::perception(e1);
    CHECK(p.perceive<MySense>(e2));

    opack::perceive<MySense, Test, R>(world);

    CHECK(!p.perceive<MySense, Test>(e2));
    e2.set<Test>({2.0});

    CHECK(p.perceive<MySense, Test>(e2));
    CHECK(p.value<MySense, Test>(e2) != nullptr);
    CHECK(p.value<MySense, Test>(e2)->value == 2.0);

    CHECK(!p.perceive<MySense, R>(e2));
    e2.add<R>(e3);
    CHECK(!p.perceive<MySense, R>(e2, e3));
    opack::perceive<MySense>(e1, e3);
    CHECK(p.perceive<MySense, R>(e2, e3));

    p.each<MyAgent>([&](opack::Entity subject)
        {
            if (subject == e2)
                CHECK(p.perceive<MySense, Test>(subject));
            if (subject == e3)
                CHECK(!p.perceive<MySense, Test>(subject));
        });
    //opack::run_with_webapp(world);
}