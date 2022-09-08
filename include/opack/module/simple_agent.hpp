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

	simple(opack::World& _world)
	{
		opack::init<Actuator>(_world);
		opack::init<Sense>(_world);
		opack::init<Agent>(_world);
		opack::add_actuator<Actuator, Agent>(_world);
		opack::add_sense<Sense, Agent>(_world);
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
