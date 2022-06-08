/*********************************************************************
 * \file   types.hpp
 * \brief  Basic types used as a foundation.
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <functional>
#include <unordered_map>
#include <map>
#include <typeindex>

#include <flecs.h>

#include <opack/utils/type_map.hpp>

namespace opack
{
	namespace internal
	{
		template<typename T>
		struct PrefabDict
		{
			std::map<std::type_index, flecs::entity> container;
		};

		template<typename Category, typename Key>
		flecs::entity add_prefab(flecs::world& world)
		{
			auto dict = world.get_mut<opack::internal::PrefabDict<Category>>();
			auto entity = world.prefab().add<Key>();
			dict->container.emplace(typeid(Key), entity );
			return entity;
		}

		template<typename Category, typename Key>
		bool has_prefab(flecs::world& world)
		{
			auto dict = world.get<opack::internal::PrefabDict<Category>>();
			return dict->container.contains(typeid(Key));
		}
	}

	/**
	 * Returns entity (prefab) associated with @c Key in the category @c Category.
	 */
	template<typename Category, typename Key = Category>
	flecs::entity prefab(flecs::world& world)
	{
		auto dict = world.get<opack::internal::PrefabDict<Category>>();
		return dict->container.at(typeid(Key));
	}

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
	using DefaultBehaviour = flecs::pair<Active, Behaviour>;

	template<typename T>
	struct Input {};

	template<typename... T>
	using Inputs = std::tuple<T...>;

	template<typename... T>
	using Outputs = std::tuple<T...>;

	template<typename TInputs, typename TOutputs>
	struct O;                     
 
	template
	<
		template<typename...> typename TInputs, typename... TInput, 
		template<typename...> typename TOutputs, typename... TOutput
	>
	struct O<TInputs<TInput...>, TOutputs<TOutput...>> 
	{
		using inputs = std::tuple<TInput...>;
		using outputs = std::tuple<TOutput...>;
	};

	template<typename TOper, typename T>
	struct df 
	{
		T value;
	};

	template<typename TInputs, typename TOutputs, typename TOtherInputs>
	struct Impact;                     

	template
	<
		template<typename...> typename TInputs, typename... TInput, 
		template<typename...> typename TOutputs, typename... TOutput,
		template<typename...> typename TOtherInputs, typename... TOtherInput
	>
	struct Impact<TInputs<TInput...>, TOutputs<TOutput...>, TOtherInputs<TOtherInput...>>
	{
		std::function<std::tuple<TOutput...>(flecs::entity, TInput&..., TOtherInput&...)> func;
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
