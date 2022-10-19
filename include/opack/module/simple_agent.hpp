/*****************************************************************//**
 * \file   simple_agent.hpp
 * \brief  Module to import a simple agent with one actuator, one sense and one flow.
 * 
 * \author Tristan
 * \date   September 2022
 *********************************************************************/
#pragma once

#include <opack/core.hpp>

/**
 * \brief  Module to import a simple agent with one actuator, one sense and one flow.
 * Flow is activated every cycle.
 *
 */
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

	static opack::ActuatorHandle get_actuator(opack::EntityView& entity)
	{
		return opack::ActuatorHandle(entity.world(), entity.target<Actuator>());
	}

	static opack::SenseHandle get_sense(opack::World& world)
	{
		return opack::SenseHandle(world, opack::entity<Sense>(world));
	}

	static opack::ActuatorHandle get_sense(opack::EntityView& entity)
	{
		return opack::ActuatorHandle(entity.world(), entity.target<Sense>());
	}

	static opack::FlowHandle get_flow(opack::World& world)
	{
		return opack::FlowHandle(world, opack::entity<Flow>(world));
	}
};
