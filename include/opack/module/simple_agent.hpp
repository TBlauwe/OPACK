/*****************************************************************//**
 * \file   agents.hpp
 * \brief  Defines some ready to use agent model.
 * 
 * \author Tristan
 * \date   September 2022
 *********************************************************************/

#include <opack/core.hpp>

struct simple
{
	OPACK_AGENT(Agent);
	OPACK_ACTUATOR(Actuator);
	OPACK_SENSE(Sense);
	OPACK_FLOW(Flow);

	simple(opack::World& world)
	{
		opack::init<Actuator>(world);
		opack::init<Sense>(world);
		opack::init<Agent>(world);
		opack::add_actuator<Actuator, Agent>(world);
		opack::add_sense<Sense, Agent>(world);
		opack::FlowBuilder<Flow>(world).build();
	}

	static opack::ActuatorHandle get_actuator(opack::World& world)
	{
		return opack::ActuatorHandle(world, opack::entity<Actuator>(world));
	}

	static opack::SenseHandle get_sense(opack::World& world)
	{
		return opack::SenseHandle(world, opack::entity<Sense>(world));
	}
};
