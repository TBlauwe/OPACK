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
		flecs::entity add_prefab(flecs::world& world)
		{
			auto entity = world.prefab().override<T>();
			world.entity<T>().template set<flecs::entity>({ entity });
			return entity;
		}

		template<typename T>
		bool has_prefab(flecs::world& world)
		{
			return world.entity<T>().template has<flecs::entity>();
		}

		template<typename T>
		void organize(flecs::entity& entity)
		{
#ifndef OPACK_OPTIMIZATION
			entity.child_of<T>();
#endif
		}

		template<typename T>
		void doc_name(flecs::entity& entity, const char * name)
		{
#ifndef OPACK_OPTIMIZATION
			entity.set_doc_name(name);
#endif
		}

		template<typename T>
		void doc_brief(flecs::entity& entity, const char * brief)
		{
#ifndef OPACK_OPTIMIZATION
			entity.set_doc_brief(brief);
#endif
		}
	}

	/**
	 * Get prefab of type @c T.
	 */
	template<typename T>
	flecs::entity prefab(flecs::world& world)
	{
		return *world.entity<T>().template get_mut<flecs::entity>();
	}

	// Used only to structure hierarchy
	namespace world 
	{
		struct Agents {};
		struct Artefacts {};
		struct Actuators {};	
		struct Messages {};	
		struct Senses {};
		struct Actions {};
		struct Flows {};
		struct Operations {};
		struct Behaviours {};
		struct Dynamics {};
		namespace prefab
		{
			struct Actions {};
			struct Agents {};
			struct Artefacts {};
			struct Messages {};	
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

	template<typename TOper, typename T>
	struct df 
	{
		using operation = TOper;
		using type = T;
		T value;
	};

	template<typename... T>
	using Inputs = std::tuple<T...>;

	template<typename... T>
	using Outputs = std::tuple<T...>;

	template<typename TOper>
	struct Impact
	{
		flecs::entity_view behaviour;
		std::function<typename TOper::outputs(flecs::entity, typename TOper::inputs&)> func;
	};

	template<typename TInputs, typename TOutputs, typename UInputs, typename UOutputs>
	struct O;                     
 
	template
	<
		template<typename...> typename TInputs, typename... TInput, 
		template<typename...> typename TOutputs, typename... TOutput,
		template<typename...> typename UInputs, typename... UInput, 
		template<typename...> typename UOutputs, typename... UOutput
	>
	struct O<TInputs<TInput...>, TOutputs<TOutput...>, UInputs<UInput...>, UOutputs<UOutput...>> 
	{
		using operation_t = O<TInputs<TInput...>, TOutputs<TOutput...>, UInputs<UInput...>, UOutputs<UOutput...>>;
		using operation_inputs_t = std::tuple<TInput...>;
		using operation_outputs_t = std::tuple<TOutput...>;
		using operation_inputs = std::tuple<TInput&...>;
		using operation_outputs = std::tuple<TOutput...>;
		using impact_inputs = std::tuple<UInput...>;
		using impact_outputs = std::tuple<UOutput...>;
		using inputs = std::tuple<TInput&..., UInput...>;
		using outputs = std::tuple<UOutput...>;
		static constexpr size_t operation_inputs_size = sizeof...(TInput);
		static constexpr size_t operation_outputs_size = sizeof...(TOutput);
		static constexpr size_t impact_inputs_size = sizeof...(UInput);
		static constexpr size_t impact_outputs_size = sizeof...(UOutput);

		template<typename TOper>
		struct Strategy
		{
			using impact_t = Impact<TOper>;
			using impacts_t = std::vector<const impact_t*>;

			Strategy(flecs::entity _agent) : agent{ _agent }
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
	struct Message {};
	struct Timestamp
	{
		float value {0.0f};
	};

	struct Begin {};
	struct End {};
}
