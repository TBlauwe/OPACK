#include <flecs.h>
#include <iostream>

enum class TileStatus {
    Free,
    Occupied
};

int main(int, char* [])
{
    flecs::world ecs;
    auto prefab = ecs.entity().add(TileStatus::Free); 
    auto entity = ecs.entity().is_a(prefab); 
    std::cout << "Entity has tile   : " << entity.has<TileStatus>() << "\n";
    std::cout << "Entity tile value : " << static_cast<int>(*entity.get<TileStatus>()) << "\n";
}