/*****************************************************************//**
 * \file   components.hpp
 * \brief  Some components defined for OPACK.
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <unordered_set>
#include <functional>

#include <flecs.h>
#include <opack/utils/ring_buffer.hpp>
#include <opack/core/api_types.hpp>

namespace opack
{
    /** Do not clean entities with this component. */
    struct DoNotClean {};

    struct Token {};

	/** Relation between an action and an entity.
	 * Actuator "X" is doing action "Y".
	 */
	struct Doing {};

	/** Relation between an action and an entity.
	 * Action "X" is done "By" entity "Y".
	 */
	struct By {};

	/** Relation between an action and an entity.
	 * Action "X" is being done "On" entity "Y".
	 */
	struct On {};	

	/** Indicate which actuator is required for the action. */
	struct RequiredActuator
	{
		flecs::entity_view value;
	};

	struct LastActionPrefabs
	{
		LastActionPrefabs(const std::size_t ring_buffer_size = 1) : previous_prefabs_done{ ring_buffer_size }{}
		ring_buffer<EntityView> previous_prefabs_done;

        /**
         * Return last @c n th action_prefab done. @c 0 is the most recent value pushed, whereas @c size()-1 is the oldest value.
         * Assert if @c n is superior or equal to @c size().
         */
		EntityView peek(std::size_t n = 0) const { return previous_prefabs_done.peek(n); }

		bool has_done(EntityView action_prefab) const { return previous_prefabs_done.contains(action_prefab); }
	};

	/** Indicates the minimum and maximum of entities needed by an action. */
	struct Arity { std::size_t min{ 1 }; std::size_t max{ 1 };};

	/** Indicates how much time is left, before action is started. */
	struct Delay { float value{ 1 }; };

	struct Duration { float value{ 0.0 }; };

	/** Removed once value reaches zero. */
	struct Timer { float value{ 1.0 }; };

	/** Holds simulation time. */
	struct Timestamp
	{
		float value {0.0f};
	};

	/** Destroy entity when timeout is equal to zero. */
	template<typename T>
	struct Timeout 
	{
		T value;
	};
	using TickTimeout = Timeout<size_t>;
	using TimeTimeout = Timeout<float>;

	struct Begin {};
	struct End {};

    /** Relation used to indicate active behaviors. */
	struct HasBehaviour {};

	using DefaultBehaviour = flecs::pair<HasBehaviour, Behaviour>;

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
			using impacts_t = std::unordered_set<const impact_t*>;

			Strategy(flecs::entity _agent) : agent{ _agent }
			{
				agent.each<HasBehaviour>(
					[&](flecs::entity object)
					{
						auto impact = object.get_second<TOper, impact_t>();
						if (impact)
							impacts.emplace(impact);
					}
				);
			}

			flecs::entity	agent{};
			impacts_t		impacts{};
		};
	};
}
