#include <flecs.h>

template<typename T>
struct R {};

struct A {};
struct B {};
struct C 
{
    int v {0};
    const char * w {"Test"};
};

int main(int, char* [])
{
    flecs::world ecs;
    auto e = ecs.entity()
        .set<R<A>, int>({1})
        .set<R<B>, int>({2})
        .set<R<A>, float>({3.0})
        .add<R<B>, C>();

    ecs.component<C>().member<int>("v").member<const char*>("w");

    ecs.system<flecs::pair<R<A>, int>>()
        .iter(
            [](flecs::iter& it, int* value)
            {
                for (auto i : it)
                {
                }
            }
        );
    ecs.app().enable_rest().run();
}