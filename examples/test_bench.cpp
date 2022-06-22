#include <iostream>
#include <flecs.h>

struct R{}; // Relation
struct A{}; // An object

int main(int, char* [])
{
    flecs::world ecs;
    auto object     = ecs.component<A>();
    auto prefab     = ecs.prefab().add<R, A>();
    auto instance   = ecs.entity().is_a(prefab);

    std::cout << prefab.has<R>(flecs::Wildcard) << "\n"; // true;
    std::cout << (prefab.get_object<R>() == object) << "\n"; // true;
    std::cout << instance.has<R>(flecs::Wildcard) << "\n"; // true;
    std::cout << (instance.get_object<R>() == object) << "\n"; // false; Could this be true ?
    std::cout << (instance.get_object<R>() == 0) << "\n"; // true;
}