//enum class TileStatus {
//    Free,
//    Occupied
//};
//int main(int, char* [])
//{
//    //flecs::world ecs;
//    ////ecs.component<TileStatus>();
//}
#include <iostream>

template<typename T>
struct Input
{
	using type = T;
};

template<typename T>
struct Output
{
	using type = T;
};

template <typename T, typename... TDependencies, typename ...TArgs>
void foo (TArgs...args)
 {
   std::cout << "T:" << typeid(T).name() << std::endl;

   std::cout << "TDependecies list:" << std::endl;

   ((std::cout << "- " << typeid(TDependencies).name() << std::endl), ...);

   std::cout << "TArgs list:" << std::endl;

   ((std::cout << "- " << typeid(TArgs).name() << std::endl), ...);
 }

int main()
 {
   foo<short, int, long, long long>(0, 1l, 2ll);   
 }
