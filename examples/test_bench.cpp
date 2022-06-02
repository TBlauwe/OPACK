#include <iostream>
#include <flecs.h>

enum class TileStatus {
    Free,
    Occupied
};

struct some_type {};

template<typename T, typename U>
struct some_struct
{
    U value;
};

int main(int, char* [])
{
    flecs::world ecs;
    //ecs.component<some_struct<some_type, int>().member<int>("value");
}
