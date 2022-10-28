#include <flecs.h>

struct Head {};
struct Turret {};

int main(int, char* [])
{
    flecs::world ecs;

    ecs.prefab<Head>();
    ecs.prefab<Turret>();

	ecs.observer()
	.event(flecs::OnAdd)
	.term(flecs::IsA).second<Turret>()
	.each(
		[](flecs::entity e)
		{
			auto child = e.world().entity().is_a<Head>();
			child.child_of(e); // Issue here
		}
	);

    ecs.entity().is_a<Turret>(); 
}