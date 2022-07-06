#include <flecs.h>
#include <iostream>

enum TileStatus {
    Free,
    Occupied
};

int main(int, char* [])
{
    flecs::world ecs;

    ecs.component<TileStatus>().constant("Free", TileStatus::Free); //1
    auto tile = ecs.entity().add(TileStatus::Free); // 2 Also assert;
    std::cout << "Tile : " << tile.has<TileStatus>() << "\n";
    std::cout << "Tile : " << tile.has(TileStatus::Free) << "\n";
}