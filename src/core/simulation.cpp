#include <opack/core/simulation.hpp>
#include <opack/core.hpp>

float opack::target_fps(const World& world) { return world.get_target_fps(); }

void opack::target_fps(World& world, float value) { world.set_target_fps(value); }

float opack::time_scale(const World& world) { return world.get_time_scale(); }

void opack::time_scale(World& world, float value) { return world.set_time_scale(value); }

int64_t opack::tick(const World& world)
{
	return world.tick();
}

float opack::delta_time(const World& world)
{
	return world.delta_time();
}

float opack::time(const World& world)
{
	return world.time();
}

void opack::run_with_webapp(World& world)
{
	fmt::print(fmt::fg(fmt::color::dim_gray) | fmt::emphasis::italic,
		"See explorer here : ");
	fmt::print(fmt::fg(fmt::color::steel_blue) | fmt::emphasis::italic | fmt::emphasis::bold,
		"https://www.flecs.dev/explorer/?remote=true\n");
	world.app().enable_rest().enable_monitor().run();
}

void opack::run(World& world) { world.app().run(); }

bool opack::step(World& world, float delta_time) { return world.progress(delta_time); }

void opack::stop(World& world) { world.quit(); }

void opack::step_n(World& world, size_t n, float delta_time)
{
	bool should_continue = true;
	for (size_t i{ 0 }; i < n && should_continue; i++) {
		should_continue = step(world, delta_time);
	}
}
