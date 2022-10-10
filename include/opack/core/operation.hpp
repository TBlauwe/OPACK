/*****************************************************************//**
 * \file   operation.hpp
 * \brief  Operations API
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <tuple>
#include <concepts>

#include <flecs.h>

#include <opack/core/api_types.hpp>
#include <opack/utils/type_name.hpp>

/**
@brief Shorthand for OPACK_SUB_PREFAB(name, opack::Flow)
*/
#define OPACK_FLOW(name) OPACK_SUB_PREFAB(name, opack::Flow)

/**
@brief Identical to OPACK_SUB_PREFAB(name, base)
*/
#define OPACK_SUB_FLOW(name, base) OPACK_SUB_PREFAB(name, base)

/**
@brief Shorthand for OPACK_SUB_PREFAB(name, opack::Behaviour)
*/
#define OPACK_BEHAVIOUR(name) OPACK_SUB_PREFAB(name, opack::Behaviour)

namespace opack
{
	struct BehaviourHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct BehaviourHandle : Handle
	{
		using Handle::Handle;
	};

	struct OperationHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct OperationHandle : Handle
	{
		using Handle::Handle;
	};

	struct FlowHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct FlowHandle : Handle
	{
		using Handle::Handle;
	};

	/**
	 @brief Create a new behaviour @c T.
	 @param func Activation function with following signature : bool(flecs::entity, (Tinputs&,...))

	 A behaviour is an entity with an associated system that runs every frame for every agent that has this behaviour, 
	 before the update. Each agent will have the component <HasBehaviour, T> if the activation function returns @c true.

	 TODO update only behaviour that are needed, e.g when a flow will be launched.
	 */
	template<std::derived_from<Behaviour> TBeh, typename ... TInputs, typename TFunc>
	flecs::entity behaviour(World& world, TFunc&& func)
	{
		//TODO should probably check if behaviour is not already instantiated
		auto behaviour = opack::init<TBeh>(world);

		world.system<TInputs ...>()
			.term(flecs::IsA).template second<const opack::Agent>()
			.template term<HasBehaviour, TBeh>().write()
			.template kind<opack::Reason::PreUpdate>()
			.each(
				[f = std::forward<TFunc>(func)](flecs::entity agent, TInputs& ... args)
				{
						if(f(agent, args...))
							agent.add<HasBehaviour, TBeh>();
						else
							agent.remove<HasBehaviour, TBeh>();
				}
		)
#ifndef OPACK_ORGANIZE
		.set_doc_name(fmt::format("BehaviourActivation : {}", type_name_cstr<TBeh>()).c_str())
		.template child_of<opack::world::dynamics>()
#endif
	    ;
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
	void impact(World& world, TFunc&& func)
    {
        auto behaviour = opack::entity<T>(world);
		if constexpr (!std::same_as<T,Behaviour>)
		{
			opack_assert(opack::is_a<Behaviour>(behaviour), "Behaviour [{0}] was not instantiated ! Did you called : \"opack::behaviour<{0}>(..)\", to initialize it ?", type_name_cstr<T>());
		}
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
	void default_impact(World& world, TFunc&& func)
    {
		impact<TOper, opack::Behaviour>(world, std::forward<TFunc>(func));
    };


	/**
	@brief Create a flow named @c T that represents part of the agent model.
	*/
	template<std::derived_from<Flow> T>
	class FlowBuilder
	{
	public:
		FlowBuilder(World& world) : 
			world{ world },
			flow_system { world.system() }
		{
			world.component<T>().template child_of<world::flows>();
			flow_system.term<const T>();
			flow_system.template term<T, Begin>().write();
			flow_system.template kind<opack::Reason::PreUpdate>();

		    world.system()
				.term<const T>()
				.template term<T, Begin>().write()
				.template kind<opack::Reason::PostUpdate>()
				.each([](flecs::entity agent)
					{
						agent.remove<T, Begin>();
					}
				)
#ifndef OPACK_ORGANIZE
		        .set_doc_name(fmt::format("CleaningFlow : {}", type_name_cstr<T>()).c_str())
		        .template child_of<opack::world::dynamics>()
#endif
						;
		}

		template<typename Condition>
		FlowBuilder<T>& Not()
		{
			flow_system.template term<Condition>().not_();
			return *this;
		}

		template<typename Condition>
		FlowBuilder<T>& has()
		{
			flow_system.template term<Condition>().inout(flecs::InOutNone);
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
			flow_system.each(
				[](flecs::entity agent)
				{
					agent.add<T, Begin>();
				}
			)
#ifndef OPACK_ORGANIZE
			.set_doc_name(fmt::format("LaunchingFlow : {}", type_name_cstr<T>()).c_str())
		    .template child_of<opack::world::dynamics>()
#endif
				;
		}

	private:
		World& world;
		flecs::system_builder<> flow_system;
	};

	template<typename TFlow>
	void flow(World& world)
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

		OperationBuilder(World& world) : 
			world{ world },
			operation {world.entity<TOper>()},
			system_builder {world.system<TInput...>(type_name_cstr<TOper>())}
		{
			//(world.template component<df<TOper, TInput>>().template member<TInput>("value") , ...); // BUG Doesn't work with templated class ?
#ifndef OPACK_ORGANIZE
			operation.child_of<world::operations>();
#endif
			system_builder.template kind<opack::Reason::Update>();
			(system_builder.template term<df<TOper,TOutput>>().write(),...);
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
			(system_builder.template term<Args>().write(), ...);
			return *this;
		}

		template<typename T>
		flecs::entity build(T&& func)
		{
			system_builder.each(func).template child_of<opack::world::dynamics>();
			return operation;
		}

		flecs::entity strategy()
		{
			system_builder.each(
				[](flecs::entity agent, TInput& ... args)
				{
						auto result = typename TOper::template Strategy<TOper>(agent).compute(args...);
						(agent.set<df<TOper, TOutput>>({std::get<TOutput>(result)}), ...); 
				}
			)
#ifndef OPACK_ORGANIZE
				.set_doc_name(fmt::format("Operation : {}", type_name_cstr<TOper>()).c_str())
					.template child_of<opack::world::dynamics>()
#endif
					;

			return operation;
		}

	private:
		World& world;
		flecs::entity operation;
		flecs::system_builder<TInput...> system_builder;
	};

	template<typename TFlow, typename... TOper>
	void operation(World& world)
	{
		(OperationBuilder<TOper, typename TOper::operation_inputs_t, typename TOper::operation_outputs_t, typename TOper::inputs, typename TOper::outputs>(world)
			.template flow<TFlow>().strategy(), ...);
	};

	inline std::tuple<> make_outputs()
	{
		return std::tuple<>();
	}

	template<typename... Args>
	std::tuple<Args...> make_outputs(Args&&... args)
	{
		return {args...};
	}

	template<typename T, typename... Args>
	typename T::outputs make_outputs(Args&&... args)
	{
		return typename T::outputs{args...};
	}

	template<typename T, typename... Args>
	typename T::inputs make_inputs(Args&&... args)
	{
		return typename T::inputs{args...};
	}

	// Shorthand to directly get the const ref of a dataflow.
	template<typename TOper, typename T>
	const T& dataflow(flecs::entity e)
	{
		return e.get<df<TOper, T>>()->value;
	}

	template<typename T, typename... Args>
	T& input(std::tuple<Args...>& tuple)
	{
		return std::get<T&>(tuple);
	}
}

