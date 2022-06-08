#include <iostream>
#include <flecs.h>

int main(int, char* [])
{
    flecs::world ecs;
    auto p1 = ecs.prefab();
    auto p2 = ecs.prefab().is_a(p1);
    auto e = ecs.entity().is_a(p2);
    std::cout << e.to_json() << std::endl;
    ecs.app().enable_rest().run();
}