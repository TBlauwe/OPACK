#include <flecs.h>
#include <iostream>

enum class TileStatus {
    Free,
    Occupied
};

int main(int, char* [])
{
    flecs::world ecs;

    ecs.component<TileStatus>(); //1
    auto tile = ecs.entity().add(TileStatus::Free); // 2 Also assert;
    std::cout << "Tile : " << tile.has<TileStatus>() << "\n";
}