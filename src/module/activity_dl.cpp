#include <opack/module/activity_dl.hpp>

adl::adl(flecs::world& world)
{
	world.module<adl>();

	world.component<adl::Activity>();

	world.component<adl::ContextualCondition>();
	world.component<adl::FavorableCondition>();
	world.component<adl::NomologicalCondition>();
	world.component<adl::RegulatoryCondition>();
	world.component<adl::SatisfactionCondition>();
}
