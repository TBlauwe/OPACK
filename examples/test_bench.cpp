#include <functional>
#include <iostream>

#include <flecs.h>

//enum class TileStatus {
//    Free,
//    Occupied
//};
//int main(int, char* [])
//{
//    //flecs::world ecs;
//    ////ecs.component<TileStatus>();
//}

struct R {};

template<typename TOutput = void, typename ... TInputs>
struct Impact
{
    std::function<TOutput(flecs::entity, TInputs...)> func;
};

template<typename TOper, typename TOutput, typename ... TInputs, typename TFunc>
//void impact(flecs::world& world, std::function<TOutput(flecs::entity, TInputs...)> func)
void impact(flecs::world& world, TFunc&& func)
{
	auto behaviour = world.entity<TOper>();
	behaviour.template set<TOper, Impact<TOutput, TInputs ...>>({ std::forward<TFunc>(func) });
}

int main(int, char* [])
{
    flecs::world world;
	impact<R, int, const char*>(world, [](flecs::entity, const char*) {return 0; });
}