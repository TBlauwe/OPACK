#include <flecs.h>
#include <fmt/core.h>

struct R { int count; };

int main()
{
	flecs::world ecs;
	ecs.component<R>()
		.member<int>("count");
	auto e = ecs.entity();
	e.add<R>(e);

	fmt::print("{}", e);
	fmt::print("{}", e.target<R>());

	ecs.app().enable_rest().run();
}