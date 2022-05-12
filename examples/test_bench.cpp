#include <flecs.h>

enum class TileStatus {
    Free,
    Occupied
};

int main(int, char* [])
{
    flecs::world ecs;
    ecs.component<TileStatus>();
    //ecs.entity().add(TileStatus::Free);
}