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
    ecs_assert(entity.has<TileStatus>(), ECS_INVALID_OPERATION, "");
    ecs_assert(entity.get<TileStatus>() != nullptr, ECS_INVALID_OPERATION, "Assert here");
}