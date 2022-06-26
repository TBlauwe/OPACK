/*********************************************************************
 * \file   operation.hpp
 * \brief  API to build operations.
 *
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <tuple>
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

	 TODO update only behaviour that are needed, e.g when a flow will be launched.
	 */
	template<std::derived_from<Behaviour> TBeh, typename ... TInputs, typename TFunc>
	flecs::entity behaviour(flecs::world& world, TFunc&& func)
	{
		auto behaviour = world.entity<TBeh>();
		behaviour.template child_of<world::Behaviours>();

		auto launcher = world.system<TInputs ...>()
			.template term<const opack::Agent>()
			.template term<Active, TBeh>().inout(flecs::Out).set(flecs::Nothing)
			.kind(flecs::PreUpdate)
			.iter(
				[f = std::forward<TFunc>(func)](flecs::iter& it, TInputs * ... args)
				{
					for (auto i : it)
					{
						auto e = it.entity(i);
						if(f(e, args[i]...))
							e.add<Active, TBeh>();
						else
							e.remove<Active, TBeh>();
					}
				}
		);
		launcher.set_doc_name("System_CheckBehaviour");
		launcher.template child_of<opack::dynamics>();
		return behaviour;
	}

	/**
	 * Add an impact to a behaviour.
	 */
	template
		<
		typename TOper,
		std::derived_from<Behaviour> T = Behaviour,
		typename TFunc
		>
	void impact(flecs::world& world, TFunc&& func)
    {
        auto behaviour = world.entity<T>();
        behaviour.template set<TOper, Impact<TOper>> ({ behaviour, func });
    };

	/**
	 * Add an impact to the default behaviour.
	 */
	template
		<
		typename TOper,
		typename TFunc
		>
	void default_impact(flecs::world& world, TFunc&& func)
    {
		impact<TOper, opack::Behaviour>(world, std::forward<TFunc>(func));
    };


	/**
	@brief Create a flow named @c T that represents part of the agent model.
	@param @c world 
	@param @c rate how much time per second
	*/
	template<std::derived_from<Flow> T>
	class FlowBuilder
	{
	public:
		FlowBuilder(flecs::world& world) : 
			world{ world },
			flow_system { world.system<const T>() }
		{
			world.component<T>().template child_of<world::Flows>();

			flow_system.template term<T, Begin>().inout(flecs::Out).set(flecs::Nothing);
			flow_system.kind(flecs::PreUpdate);

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
		}

		template<typename Condition>
		FlowBuilder<T>& Not()
		{
			flow_system.template term<Condition>().oper(flecs::Not);
			return *this;
		}

		template<typename Condition>
		FlowBuilder<T>& has()
		{
			flow_system.template term<Condition>().inout(flecs::InOutFilter);
			return *this;
		}

		flecs::system_builder<const T>& conditions()
		{
			return flow_system;
		}

		FlowBuilder<T>& interval(float rate = 1.0f)
		{
			flow_system.interval(rate);
			return *this;
		}

		void build()
		{
			auto launcher = flow_system
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
		}

	private:
		flecs::system_builder<const T> flow_system;
		flecs::world& world;
	};

	template<typename TFlow>
	void flow(flecs::world& world)
	{
		FlowBuilder<TFlow>(world).interval().build();
	};


	// Primary template
	template<typename TOper, typename TInputs, typename TOutputs, typename UInputs, typename UOutputs>
	class OperationBuilder;                     
 
	// Partial specialization
	template<
		typename TOper, 
		template<typename...> typename TInputs, typename... TInput, 
		template<typename ...> typename TOutputs, typename... TOutput,
		template<typename...> typename UInputs, typename... UInput, 
		template<typename ...> typename UOutputs, typename... UOutput
	>
	class OperationBuilder<TOper, TInputs<TInput...>, TOutputs<TOutput...>, UInputs<UInput...>, UOutputs<UOutput...>>
	{
	public:
		using inputs = std::tuple<TInput&...> ;
		using outputs = std::tuple<TOutput...> ;

		OperationBuilder(flecs::world& world) : 
			world{ world },
			operation {world.entity<TOper>()},
			system_builder {world.system<TInput...>(type_name_cstr<TOper>())}
		{
			//(world.template component<df<TOper, TInput>>().template member<TInput>("value") , ...); // BUG Doesn't work with templated class ?
			operation.child_of<world::Operations>();
			system_builder.kind(flecs::OnUpdate);
			(system_builder.template term<df<TOper,TOutput>>().inout(flecs::Out).set(flecs::Nothing),...);
			//system_builder.multi_threaded(true); // BUG doesn't seem to work with monitor
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

		flecs::entity strategy()
		{
			system_builder.iter(
				[](flecs::iter& it, TInput* ... args)
				{
					for (auto i : it)
					{
						auto e = it.entity(i);
						auto result = typename TOper::template Strategy<TOper>(e).compute(args[i]...);
						(e.set<df<TOper, TOutput>>({std::get<TOutput>(result)}), ...); 
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

	template<typename TFlow, typename... TOper>
	void operation(flecs::world& world)
	{
		(OperationBuilder<TOper, typename TOper::operation_inputs_t, typename TOper::operation_outputs_t, typename TOper::inputs, typename TOper::outputs>(world)
			.template flow<TFlow>().strategy(), ...);
	};

	template<typename T, typename... Args>
	inline typename T::outputs make_outputs(Args&&... args)
	{
		return typename T::outputs{args...};
	}

	template<typename T, typename... Args>
	inline typename T::inputs make_inputs(Args&&... args)
	{
		return typename T::inputs{args...};
	}

	// Shorthand to directly get the const ref of a dataflow.
	template<typename TOper, typename T>
	const T& dataflow(flecs::entity e)
	{
		return e.template get<df<TOper, T>>()->value;
	}

	template<typename T, typename... Args>
	inline T& input(std::tuple<Args...>& tuple)
	{
		return std::get<T&>(tuple);
	}
}

