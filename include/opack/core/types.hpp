/*********************************************************************
 * \file   types.hpp
 * \brief  Basic types used as a foundation.
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

namespace opack
{
	// Used only to structure hierarchy
	namespace world 
	{
		struct Agents {};
		struct Artefacts {};
		struct Actuators {};	
		struct Senses {};
		struct Actions {};
		namespace prefab
		{
			struct Actions {};
			struct Agents {};
			struct Artefacts {};
		};
	};

	// MAS - Multi-agent systems
	//--------------------------
	struct Agent {};
	struct Artefact {};

	// O - Operation
	//--------------
	struct Flow {};
	struct Operation {};
	struct Impact {};

	// P - Perception
	//--------------
	struct Sense {};
	// Percept are expressed as relation between two entities
	// e.g : agent --Sense--> artefact

	// A - Action
	//-----------
	struct Actuator {};	
	struct Action {};
	enum ActionType {Ponctual, Continuous};
	// An action is performed by someones
	struct By {};
	// An action is performed on something
	struct On {};	
	struct Arity { size_t min{ 1 }; size_t max{ 1 };};
	struct Delay { float value{ 1 }; };

	// C - Caracteristics
	//-------------------
	// C <==> Components

	// K - Knowledge
	//--------------
	struct Knowledge {};
	// TODO

	// MISC
	//-----
	struct Timestamp
	{
		float value {0.0f};
	};

	struct Begin 
	{
	};
	struct End 
	{
	};
}
