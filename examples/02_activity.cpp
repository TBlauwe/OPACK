#include <iostream>

#include <opack/core.hpp>
#include <opack/module/simple_agent.hpp>
#include <opack/module/fipa_acl.hpp>
#include <opack/module/adl.hpp>
#include <opack/module/flows.hpp>

OPACK_FLOW(MyFlow);
OPACK_SUB_ACTION(BaseAction, adl::Action);

int main()
{
	auto world = opack::create_world();

	world.import<simple>();
	world.import<adl>();
	world.import<fipa_acl>();

	world.entity<simple::Agent>().add<MyFlow>();
	opack::init<BaseAction>(world).require<simple::Actuator>();

	ActivityFlowBuilder<MyFlow, adl::Activity>(world).interval(1.0).build();

	world.observer()
		.term<const adl::Constructor>()
		.term(flecs::ChildOf, flecs::Wildcard)
		.event(flecs::OnAdd)
		.each([](flecs::entity entity) {fmt::print("Number of child {} : {}\n", entity.path(), opack::internal::children_count(entity.parent())); });

	world.plecs_from_file("plecs/activity.flecs");

	world.lookup("instance").children([](flecs::entity child) { fmt::print("{}", child.path()); });
	opack::run_with_webapp(world);
}