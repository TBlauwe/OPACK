#include <flecs.h>
#include <fmt/core.h>

enum class Status { Free, Taken };

int main()
{
	flecs::world ecs;
	auto e = ecs.entity().add(Status::Free);
	auto f = ecs.entity().add(Status::Free);
	fmt::print("{}\n", e.has(*f.get<Status>()));
	ecs.app().enable_rest().run();
}