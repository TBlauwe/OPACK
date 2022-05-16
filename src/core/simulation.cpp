#include <opack/core/simulation.hpp>
#include <opack/core/perception.hpp>
#include <iostream>

flecs::world opack::internal::world(int argc, char* argv[])
{
	flecs::world world{ argc, argv };
	world.import<opack::concepts>();
	return world;
}

opack::Simulation::Simulation(int argc, char* argv[]) : world{ internal::world(argc, argv) }
{
	world.import<opack::dynamics>();
}

opack::concepts::concepts(flecs::world& world)
{
	world.module<concepts>();
	world.import<flecs::units>();

	// Organisation
	// ------------
	world.entity("::opack::queries").add(flecs::Module);
	world.entity("::opack::modules").add(flecs::Module);
	world.entity("::modules").add(flecs::Module);

	world.entity("::world").add(flecs::Module);
	world.entity("::world::prefab").add(flecs::Module);
	world.entity<world::prefab::Actions>("::world::prefab::Actions").add(flecs::Module);
	world.entity<world::prefab::Artefacts>("::world::prefab::Artefacts").add(flecs::Module);
	world.entity<world::prefab::Agents>("::world::prefab::Agents").add(flecs::Module);
	world.entity<world::Agents>("::world::Agents").add(flecs::Module);
	world.entity<world::Artefacts>("::world::Artefacts").add(flecs::Module);
	world.entity<world::Actions>("::world::Actions").add(flecs::Module);
	world.entity<world::Actuators>("::world::Actuators").add(flecs::Module);
	world.entity<world::Senses>("::world::Senses").add(flecs::Module);

	// MAS
	// ---
	world.prefab<Agent>().add<Agent>();
	world.prefab<Artefact>().add<Artefact>();

	// Action
	// ------
	world.component<Arity>()
		.member<size_t>("min")
		.member<size_t>("max")
		;
	world.component<By>();
	world.component<On>();
	world.component<Delay>()
		.member<float, flecs::units::duration::Seconds>("value")
		;

	auto action = world.prefab<Action>()
		.add<Action>()
		.add<Arity>()
		;

	world.prefab<Actuator>()
		.add<Actuator>()
		.add(flecs::Exclusive)
		.add(flecs::OneOf, action)
		;

	// Perception
	// ----------
	world.prefab<Sense>().add<Sense>();

	world.entity("::opack::queries::perception").add(flecs::Module);
	world.emplace<queries::perception::Component>(world);
	world.emplace<queries::perception::Relation>(world);
}

opack::dynamics::dynamics(flecs::world& world)
{
	world.module<dynamics>();
	world.import<concepts>();

	world.system<Action>("CleanActions")
		.term<By>().obj(flecs::Wildcard).oper(flecs::Not)
		.iter([](flecs::iter& iter)
			{
				for (auto i : iter)
				{
					iter.entity(i).destruct();
				}
			}
	);

	world.system<Delay>("UpdateDelay")
		.iter([](flecs::iter& iter, Delay* delays)
			{
				for (auto i : iter)
				{
					delays[i].value -= iter.delta_system_time();
					if (delays[i].value <= 0)
						iter.entity(i).remove<Delay>();
				}
			}
	);
}

float opack::Simulation::target_fps() const { return world.get_target_fps(); }

void opack::Simulation::target_fps(float value) { world.set_target_fps(value); }

float opack::Simulation::time_scale() const { return world.get_time_scale(); }

void opack::Simulation::time_scale(float value) { return world.set_time_scale(value); }

int32_t opack::Simulation::tick()
{
	return world.tick();
}

float opack::Simulation::delta_time()
{
	return world.delta_time();
}

float opack::Simulation::time()
{
	return world.time();
}

bool opack::Simulation::step(float elapsed_time) { return opack::step(world, elapsed_time); }

void opack::Simulation::step_n(size_t n, float elapsed_time) { opack::step_n(world, n, elapsed_time); }

void opack::Simulation::run_with_webapp() { world.app().enable_rest().run(); }

void opack::Simulation::run() { world.app().run(); }

void opack::name(flecs::entity e, const char* name) { e.set_doc_name(name); }

bool opack::step(flecs::world& world, float delta_time) { return world.progress(delta_time); }

void opack::stop(flecs::world& world) { world.quit(); }

size_t opack::count(flecs::world& world, flecs::entity_t rel, flecs::entity_t obj) { return static_cast<size_t>(world.count(rel, obj)); }

void opack::step_n(flecs::world& world, size_t n, float delta_time)
{
	bool should_continue = true;
	for (size_t i{ 0 }; i < n && should_continue; i++) {
		should_continue = step(world, delta_time);
	}
}
