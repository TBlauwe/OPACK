#include <flecs.h>

enum class TileStatus {
    Free,
    Occupied
};

int main(int, char* [])
{
    flecs::world ecs;

    auto tile = ecs.entity().add(TileStatus::Free);
}