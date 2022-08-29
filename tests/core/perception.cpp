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
    opack::batch_init<
        MyAgent,
        MySense
    >(world);
    opack::add_sense<MySense, MyAgent>(world);
    auto e1  = opack::spawn<MyAgent>(world);
    auto e2  = opack::spawn<MyAgent>(world);
    auto e3  = opack::spawn<MyAgent>(world);

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

    fmt::print("===== RULE ====\n");
    auto rule = world.rule_builder()
        .term(flecs::IsA).src().var("Sensor").second().var("Sense")
        .term().first().var("Sense").src().var("Observer").second().var("Sensor")
        .term().first().var("Subject").src().var("Sensor")
        .term<opack::Sense>().src().var("Sense").second().var("Predicate")
        .term().first().var("Predicate").src().var("Subject").or_()
        .term().first().var("Predicate").src().var("Subject").second().var("Object").or_()
        .build();
    fmt::print("to string : {} \n", rule.str());
    rule.iter().set_var("Observer", e1).iter(
        [](flecs::iter& it)
        {
            auto observer = it.get_var("Observer");
            auto sense = it.get_var("Sense");
            auto subject = it.get_var("Subject");
            auto predicate = it.get_var("Predicate");
            auto object = it.get_var("Object");
            if(object)
                fmt::print("{} with sense {} perceives {} with {} and {}\n", observer.path(), sense.path(), subject.path(), predicate.path(), object.path());
            else
                fmt::print("{} with sense {} perceives {} with {}\n", observer.path(), sense.path(), subject.path(), predicate.path());
        }
    );
    fmt::print("===============\n");
    opack::run_with_webapp(world);
}