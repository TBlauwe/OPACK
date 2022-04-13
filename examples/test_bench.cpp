#include <flecs.h>

struct A
{
    struct X {};
};

struct B {};


int main(int argc, char* argv[])
{
    flecs::world world;
    world.prefab<A>();
    world.component<A::X>();    
    world.prefab<B>().is_a<A>();
}