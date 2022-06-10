#include <flecs.h>
#include <tuple>

template<typename... T>
using Inputs = std::tuple<T...>;

template<typename TOper>
struct dataflow {};

template<typename TOper, typename T>
using df = flecs::pair<dataflow<TOper>, T>;

template<typename T>
struct R {};

struct A {};
struct B {};
struct C 
{
    int v {0};
    const char * w {"Test"};
};

template<typename TInputs>
struct Test;

template<template <typename...> class TInputs, typename... TInput>
struct Test<TInputs<TInput...>>
{
    using tuple = std::tuple<TInput...>;
};

template<typename T>
struct input_type
{};

int main(int, char* [])
{
    static_assert(std::is_same_v<int, df<A, int>::type>, "works");

    flecs::world ecs;
    auto e = ecs.entity()
        .set<df<A, int>>({1})
        .set<df<B, int>>({2})
        .set<df<A, float>>({3.0})
        .add<df<B, C>>();

    ecs.component<C>().member<int>("v").member<const char*>("w");

    ecs.system<df<A, int>>()
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