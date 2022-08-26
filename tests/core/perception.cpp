#include <doctest/doctest.h>
#include <opack/core.hpp>
#include <iostream>


OPACK_SUB_PREFAB(MyAgent, opack::Agent);

OPACK_SUB_PREFAB(MySense, opack::Sense);

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
    CHECK(opack::perception(e1).perceive<MySense>(e2));

    auto rule = world.rule_builder()
        .term(flecs::ChildOf).src().var("Sense").second().var("Agent")
        .term(flecs::IsA).src().var("Sense").second<opack::Sense>()
        //.expr("$R($Agent, $Sense)")
    .build();

    std::cout << "Rule : " << rule.count() << " !\n";
    auto var = rule.find_var("Agent");
    std::cout << "Rule : " << rule.iter().set_var(var, e1).count() << " !\n";
    rule.iter().set_var(var, e1).iter([](flecs::iter& iter)
        {
            auto e = iter.get_var("Sense");
            std::cout << "Hello from entity : " << e.path() << " !\n";
        }
    );

    opack::run_with_webapp(world);
}