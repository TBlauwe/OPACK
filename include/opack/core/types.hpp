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

	template<typename TStrat, typename TInputs, typename TOutputs>
	struct O;                     
 
	template
	<
		typename TStrat,
		template<typename...> typename TInputs, typename... TInput, 
		template<typename...> typename TOutputs, typename... TOutput
	>
	struct O<TStrat, TInputs<TInput...>, TOutputs<TOutput...>> 
	{
		using operation_inputs_t = std::tuple<TInput...>;
		using operation_outputs_t = std::tuple<TOutput...>;
		using operation_inputs = std::tuple<TInput&...>;
		using operation_outputs = std::tuple<TOutput...>;
		using strategy = TStrat;
		using impact_inputs = typename strategy::impact_inputs;
		using impact_outputs = typename strategy::impact_outputs;

	};

	template<typename TOper, typename T>
	struct df 
	{
		T value;
	};

	template<typename TOper>
	struct Impact
	{
		std::function<typename TOper::impact_outputs(flecs::entity, typename TOper::operation_inputs&, typename TOper::impact_inputs&)> func;
	};

	template<typename TInputs, typename TOutputs>
	struct Strategy;                     

	template
	<
		template<typename...> typename TInputs, typename... TInput,
		template<typename...> typename TOutputs, typename... TOutput
	>
	struct Strategy<TInputs<TInput...>, TOutputs<TOutput...>>
	{
		using impact_inputs = std::tuple<TInput&...> ;
		using impact_outputs = std::tuple<TOutput...> ;

		template<typename TOper>
		struct Algorithm
		{
			using impact_t = Impact<TOper>;
			using impacts_t = std::vector<const impact_t*>;

			Algorithm(flecs::entity _agent) : agent{ _agent }
			{
				agent.each<Active>(
					[&](flecs::entity object)
					{
						auto impact = object.get_w_object<TOper, impact_t>();
						if (impact)
							impacts.push_back(impact);
					}
				);
			}

			flecs::entity	agent{};
			impacts_t		impacts{};
			impact_inputs	inputs{};
		};
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
	using Actions_t = std::vector<flecs::entity>;
	using Action_t = flecs::entity;
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
