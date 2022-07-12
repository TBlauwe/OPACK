#include <flecs.h>
#include <iostream>


struct A {};
struct B {};

int main(int, char* [])
{
    flecs::world ecs;

    auto prefab = ecs.prefab().override<A>();
    auto sub_prefab = ecs.prefab().is_a(prefab).override<B>();
    auto entity = ecs.entity().is_a(sub_prefab);
    std::cout << "prefab has A : " << prefab.has<A>() << "\n";
    std::cout << "sub_prefab has A : " << sub_prefab.has<A>() << "\n";
    std::cout << "sub_prefab has A : " << sub_prefab.has<B>() << "\n";
    std::cout << "entity has A : " << entity.has<A>() << "\n";
    std::cout << "entity has B : " << entity.has<B>() << "\n";
}