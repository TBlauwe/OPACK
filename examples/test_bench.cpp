#include <flecs.h>

template<typename T>
struct R {};

struct A {};
struct B {};
struct C {};

int main(int, char* [])
{
    flecs::world ecs;
    auto e = ecs.entity()
        .set<R<A>, int>({1})
        .set<R<B>, int>({2})
        .set<R<A>, float>({3.0});
    ecs.system<flecs::pair<R<A>, int>>()
        .iter(
            [](flecs::iter& it)
            {
                for (auto i : it)
                {

                }
            }
        );
    ecs.app().enable_rest().run();
}