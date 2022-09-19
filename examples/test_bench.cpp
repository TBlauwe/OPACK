#include <flecs.h>

int main(int, char* [])
{
    flecs::world ecs;
    ecs.plecs_from_str("test", "e { - A }");
    ecs.app().enable_rest().run();
}