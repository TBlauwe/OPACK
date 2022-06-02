/*********************************************************************
 * \file   operation.hpp
 * \brief  API to build operations.
 *
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <concepts>
#include <functional>

#include <flecs.h>

#include <opack/core/types.hpp>
#include <opack/module/core.hpp>

namespace opack
{

	/**
	 @brief Create a new behaviour @c T.
	 @param func Activation function with following signature : bool(flecs::entity, (Tinputs&,...))

	 A behaviour is an entity with an associated system that runs every frame for every agent that has this behaviour, 
	 before the update. Each agent will have the component <Active, T> if the activation function returns @c true.
	 */
	template<std::derived_from<Behaviour> T, typename ... TInputs, typename TFunc>
	flecs::entity behaviour(flecs::world& world, TFunc&& func)
	{
		auto behaviour = world.component<T>();
		behaviour.template child_of<world::Behaviours>();

		auto launcher = world.system<TInputs ...>()
			.template term<const T>()
			.template term<Active, T>().inout(flecs::Out).set(flecs::Nothing)
			.kind(flecs::PreUpdate)
			.iter(
				[f = std::forward<TFunc>(func)](flecs::iter& it, TInputs * ... args)
				{
					for (auto i : it)
					{
						auto e = it.entity(i);
						if(f(e, (args[i], ...)))
							e.add<Active, T>();
						else
							e.remove<Active, T>();
					}
				}
		);
		launcher.set_doc_name("System_CheckBehaviour");
		launcher.template child_of<opack::dynamics>();
		return behaviour;
	}

	/**
	 * .
	 */
	template<std::derived_from<Operation> T, typename TFunc>
	void each_active_behaviours(flecs::entity e, TFunc&& func)
	{
		e.each<Active>(
			[f = std::forward<TFunc>(func)](flecs::entity object)
			{
				auto impact = object.get_object<T>();
				if (impact)
					f(impact);
			}
		);
	}

	/**
	 * Add an impact to a behaviour.
	 */
	template<std::derived_from<Behaviour> T, std::derived_from<Operation> TOper, typename TOutput = void, typename... TInputs>
	void impact(flecs::world& world, Impact_t<TOutput, TInputs...>&& func)
	{
		auto behaviour = world.entity<T>();
		behaviour.template set<TOper, Impact>({func});
	}

	/**
	@brief Create a flow named @c T that represents part of the agent model
	@param @c world 
	@param @c rate how much time per second
	*/
	template<std::derived_from<Flow> T>
	flecs::entity flow(flecs::world& world, float rate = 1.0f)
	{
		auto flow = world.component<T>();
		flow.template child_of<world::Flows>();

		auto launcher = world.system<const T>()
			.template term<T, Begin>().inout(flecs::Out).set(flecs::Nothing)
			.kind(flecs::PreUpdate)
			.interval(rate)
			.iter(
				[](flecs::iter& it)
				{
					for (auto i : it)
					{
						it.entity(i).add<T, Begin>();
					}
				}
		);
		launcher.set_doc_name("System_LaunchFlow");
		launcher.set_doc_brief(type_name_cstr<T>());
		launcher.template child_of<opack::dynamics>();

		auto cleaner = world.system<const T>()
			.template term<T, Begin>()
			.kind(flecs::PostUpdate)
			.iter(
				[](flecs::iter& it)
				{
					for (auto i : it)
					{
						it.entity(i).remove<T, Begin>();
					}
				}
		);
		cleaner.set_doc_name("System_CleanFlowLeftOver");
		cleaner.set_doc_brief(type_name_cstr<T>());
		cleaner.template child_of<opack::dynamics>();

		return flow;
	}

	// Primary template
	template<std::derived_from<Operation> TOper, typename TInputs, typename TOutputs>
	class OperationBuilder;                     
 
	// Partial specialization
	template<std::derived_from<Operation> TOper, template<typename...> typename TInputs, typename... TInput, template<typename ...> typename TOutputs, typename... TOutput>
	class OperationBuilder<TOper, TInputs<TInput...>, TOutputs<TOutput...>> 
	{
	public:
		using inputs = std::tuple<TInput...> ;
		using outputs = std::tuple<TOutput...> ;

		OperationBuilder(flecs::world& world) : 
			world{ world },
			operation {world.entity<TOper>()},
			system_builder {world.system<TInput...>(type_name_cstr<TOper>())}
		{
			//(world.template component<Dataflow<TOper, TInput>>().member<TInput>("value"), ...); // Doesn't work with templated class ?
			operation.child_of<world::Operations>();
			system_builder.kind(flecs::OnUpdate);
			(system_builder.template term<Dataflow<TOper, TInput>>().inout(flecs::Out).set(flecs::Nothing),...);
			system_builder.multi_threaded(true);
		}

		// If using this, then we should add tag (relation) to declare when an operation is finished, 
		// at the cost of creating tables. So let's try to do without.
		template<std::derived_from<Operation> T>
		OperationBuilder& after()
		{
			system_builder.template term<T, End>().inout(flecs::In);
			return *this;
		}

		template<std::derived_from<Flow> T>
		OperationBuilder& flow()
		{
			system_builder.template term<T, Begin>().inout(flecs::In);
			opack::entity<T>(world).template add<Operation, TOper>();
			return *this;
		}

		template<typename ... Args>
		OperationBuilder& output()
		{
			(system_builder.template term<Args>().inout(flecs::Out).set(flecs::Nothing), ...);
			return *this;
		}

		template<typename T>
		flecs::entity build(T&& func)
		{
			system_builder.each(func).template child_of<opack::dynamics>();
			return operation;
		}

		template<template<typename, typename ..., typename ...> typename T, typename TBehaviourOutput = void, typename... TAdditionalInputs>
		//flecs::entity strategy(std::function<void(flecs::entity, TInputs...)> strategy)
		flecs::entity strategy()
		{
			system_builder.iter(
				[](flecs::iter& it, TInput* ... args)
				{
					for (auto i : it)
					{
						auto e = it.entity(i);
						// For each entity, we retrieve every active behaviours and store those whom have an impact for this operation
						// Then we called the passed strategy.
						Impacts<TBehaviourOutput, TInput ...> impacts{};
						e.each<Active>(
							[&](flecs::entity object)
							{
								auto impact = object.get_w_object<TOper, Impact<TBehaviourOutput, TInput...>>();
								if (impact)
									impacts.push_back(&impact->func);
							}
						);
						(e.set<Dataflow<TOper, TInput>>({}), ...);
						T<TOutput, TInput..., TAdditionalInputs...>()(e, impacts, args[i]...);
					}
				}
			).template child_of<opack::dynamics>();
			return operation;
		}

	private:
		flecs::entity operation;
		flecs::system_builder<TInput...> system_builder;
		flecs::world& world;
	};
}