/*********************************************************************
 * \file   types.hpp
 * \brief  Basic types used as a foundation.
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <functional>
#include <unordered_map>

#include <flecs.h>

#include <opack/utils/type_map.hpp>

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
		struct Flows {};
		struct Operations {};
		struct Behaviours {};
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
	struct Active {};
	struct Behaviour {};

	template<typename T>
	struct Input {};

	template<typename... T>
	using Inputs = std::tuple<T...>;

	template<typename... T>
	using Outputs = std::tuple<T...>;

	template<std::derived_from<Operation> TOper, typename T>
	struct Dataflow 
	{
		T value;
	};

	template<typename TOutput = void, typename ... TInputs>
	using Impact_t = std::function<TOutput(flecs::entity, TInputs& ...)>;

	template<typename TOutput = void, typename ... TInputs>
	struct Impact 
	{
		Impact_t<TOutput, TInputs...> func;
	};

	template<typename TOutput = void, typename ... TInputs>
	using Impacts = std::vector<const Impact_t<TOutput, TInputs...>*>;

	template<typename TOutput, typename... TInputs>
	struct Strategy 
	{
		virtual TOutput operator()(flecs::entity agent, const opack::Impacts<TOutput, TInputs ...>& impacts, TInputs& ... args) const = 0;
	};

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
