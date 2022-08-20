#include <flecs.h>
#include <iostream>
#include <concepts>

using entity = flecs::entity;
using entity_view = flecs::entity_view;
using agent = entity;
using agent_view = entity_view;
using artefact = entity;
using artefact_view = entity_view;
using environment = flecs::world;
using delta_time = float;

struct Agent
{
    struct Operations {};
    struct Perceptions {};
    struct Actions {};
    struct Knowledge {};

    static void init(environment& env)
    {
        env.prefab<Agent>();
        env.prefab<Agent::Operations>().child_of<Agent>().slot_of<Agent>();
        env.prefab<Agent::Perceptions>().child_of<Agent>().slot_of<Agent>();
        env.prefab<Agent::Actions>().child_of<Agent>().slot_of<Agent>();
        env.prefab<Agent::Knowledge>().child_of<Agent>().slot_of<Agent>();
    }
};


/// <summary>
/// A caracteristic is either a parameter or a variable to describe a concept, such as stress, emotion, etc.
/// 
/// By inheriting this struct, we provide few functions to manipulate the caracteristic.
/// </summary>
/// <typeparam name="T">Type inheriting this struct, e.g : 
///     @code{cpp} struct my_type : Caracteristic<my_type> {}; @endcode
/// </typeparam>
struct Caracteristic
{
};

template<typename T>
struct NumericalCaracteristic
{
    static constexpr T min {};
    static constexpr T max {};
    static constexpr T def {};
    T value {};
};

template<typename T>
void update(NumericalCaracteristic<T>& c, entity_view ev, delta_time dt)
{
}

template<typename T>
concept tag = sizeof(T) == 1;

template<typename T>
concept relation = sizeof(T) == 1;

template<typename T>
concept component = sizeof(T) > 1;

template<typename T>
concept state = std::is_enum_v<T>;

/// <summary>
/// Retrieve immutable component @c T for given entity @c e.
/// </summary>
template<typename T> 
requires component<T> || state<T>
inline static const T& get(entity_view e) { return e.get<T>(); };

template<relation T>
inline static entity_view get(entity_view e) { return e.target<T>(); };


/// <summary>
/// Retrieve mutable component @c T for given entity @c e.
/// </summary>
template<typename T>
inline static T& get_mut(entity e) { return *e.get_mut<T>(); };

/// <summary>
/// Set a component @c T for given entity @c e with value @c value.
/// </summary>
template<typename T>
inline static void set(entity e, T&& value) { e.set<T>(value); };

/// <summary>
/// Set a component @c T for given entity @c e with value @c value.
/// </summary>
template<typename T>
inline static void add(entity e) { e.add<T>(); };

/// <summary>
/// Optional call to register this caracteristic in the world. 
/// It returns the associated entity, if you need to attach additionnal information/behaviour.
/// </summary>
template<typename T>
inline static flecs::entity init(environment& env){ return env.component<T>(); };

struct MyAgent {};

int main()
{
    environment env;
    env.entity<Agent>();
    env.prefab<Agent>();
    env.prefab<Agent::Perceptions>().child_of<Agent>().slot_of<Agent>();
    env.prefab<MyAgent>().is_a<Agent>();
    env.prefab<Agent::Knowledge>().child_of<MyAgent>().slot_of<MyAgent>();

    env.entity().is_a<Agent>();
    env.entity().is_a<Agent>();
    env.entity().is_a<Agent>();
    
    {
        auto e = env.entity().is_a<MyAgent>();
        std::cout << e.target<Agent::Perceptions>() << "\n";
        std::cout << e.target<Agent::Knowledge>() << "\n";
    }
    //ecs_assert(inst.has<Agent>(), ECS_INTERNAL_ERROR, "Not good");
    //ecs_assert(env.count<Agent>() == 1, ECS_INTERNAL_ERROR, "Not good");
    env.system().term(flecs::IsA, env.entity<Agent>()).each([](flecs::entity e) {std::cout << e.path() << "\n"; });

    {
        auto rule = env.rule_builder().term(flecs::IsA, env.entity<Agent>()).term(flecs::Prefab).not_().build();
        std::cout << rule.count() << "\n";
        rule.destruct();
    }
    {
        auto rule = env.rule_builder().term(flecs::IsA, env.entity<MyAgent>()).term(flecs::Prefab).not_().build();
        std::cout << rule.count() << "\n";
        rule.destruct();
    }

//    auto agent = env.entity();
//    auto e = env.entity();
//    A::add(e);
//    A::add(agent);
//    A::init(env);
    env.app().enable_rest().run();;
}