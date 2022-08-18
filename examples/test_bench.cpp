//#include <flecs.h>
//#include <iostream>
//
//template<typename T>
//class Entity : public flecs::entity
//{
//public:
//    using flecs::entity::entity;
//    inline operator flecs::entity() { return *this; };
//
//// Little hack to ensure we derived from the correct class
//// see : https://www.fluentcpp.com/2017/05/12/curiously-recurring-template-pattern/
//private:
//    Entity(){};
//    friend T;
//};
//
//struct Template : public base_entity 
//{
//    using base_entity::base_entity;
//};
//struct Agent : public base_entity
//{
//    using base_entity::base_entity;
//};
//struct Artefact : public base_entity
//{
//    using base_entity::base_entity;
//};
//struct Environment : public flecs::world 
//{
//    using flecs::world::world;
//    inline operator flecs::world&() { return *this; };
//};
//
//template<typename Struct>
//
//struct Caracteristic
//{
//    inline static const Struct& get(Agent e) { return e.get<Struct>(); };
//    inline static Struct& get_mut(Agent e) { return *e.get_mut<Struct>(); };
//    inline static void set(Agent e, Struct&& value) { e.set<Struct>(value); };
//    inline static void add(Agent e) { e.add<Struct>(); };
//    inline static flecs::entity init(Environment& world){ return world.component<Struct>(); };
//};
//
//struct A : public Caracteristic<A> 
//{
//    inline static flecs::entity init(flecs::world& world) { std::cout << "Hello\n"; return world.entity(); };
//};
//
int main()
{
    //flecs::world ecs;
    //auto e = Agent(ecs.entity());
    //A::add(e);
    //A::init(ecs);
    ////ecs.app().enable_rest().run();;
}