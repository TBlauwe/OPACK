#include <flecs.h>

struct Head {};

int main(int, char* [])
{
    flecs::world ecs;
    auto e = ecs.prefab();
    ecs.prefab<Head>().child_of(e);
    ecs.entity().is_a(e); // Assert here
}