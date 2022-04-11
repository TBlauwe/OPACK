#include <opack/core/simulation.hpp>
#include <iostream>

std::ostream& operator<<(std::ostream& os, const opack::Percept& p)
{
	os << "[" << p.sense.path() << "] - " << p.subject.path() << " has " << p.predicat.path();
	if (p.type == opack::Percept::Type::Relation)
		os << " with " << p.object.path();
	return os;
}

opack::Simulation::Simulation(int argc, char* argv[]) 
	: world{ argc, argv }
{
	world.entity("::opack").add(flecs::Module);

	rule_components_perception = world.rule_builder()
		.expr("$Sense($Observer, $Subject), $Predicat($Subject)")
		.term<Sense>().subj().var("Sense").obj().var("Predicat")
		.build();

	rule_relations_perception = world.rule_builder()
		.expr("$Sense($Observer, $Subject), $Predicat($Subject, $Object)")
		.term<Sense>().subj().var("Sense").obj().var("Predicat")
		.build();
}

opack::Simulation::~Simulation()
{
	rule_components_perception.destruct();
	stop();
}

bool opack::Simulation::step(float elapsed_time) {
	bool should_continue = world.progress(elapsed_time);

	//executor.wait_for_all();
	//size_t size = commands_queue.size();    // Since size can be updated between for loop (async),
	//                                        // we must check only once, not at every loop !
	//for (int i = 0; i<size; i++)
	//{
	//    auto command = commands_queue.pop();
	//    if(command && command.value()) // BUG : Somehow some commands are empty
	//        command.value()(_world);
	//}

	return should_continue;
}

void opack::Simulation::step_n(size_t n, float elapsed_time) {
	bool should_continue = true;
	for (size_t i = 0; i < n && should_continue; i++) {
		should_continue = step(elapsed_time);
	}
}

void opack::Simulation::stop()
{
	world.quit();
	executor.wait_for_all();
}

float opack::Simulation::target_fps()
{
	return world.get_target_fps();
}

void opack::Simulation::target_fps(float value)
{
	world.set_target_fps(value);
}

float opack::Simulation::time_scale()
{
	return world.get_time_scale();
}

void opack::Simulation::time_scale(float value)
{
	world.set_time_scale(value);
}

size_t opack::Simulation::tick() const
{
	return static_cast<size_t>(world.tick());
}

float opack::Simulation::delta_time() const
{
	return world.delta_time();
}

float opack::Simulation::time() const
{
	return world.time();
}

flecs::world& opack::Simulation::flecs_world()
{
	return world; 
};
