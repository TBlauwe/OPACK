#include <opack/core/simulation.hpp>
#include <iostream>

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

void opack::Simulation::run_with_webapp() {world.app().enable_rest().enable_monitor().run(); }

void opack::Simulation::run() { world.app().run(); }

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
