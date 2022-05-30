/*********************************************************************
 * \file   operation.hpp
 * \brief  API to build operations.
 *
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <concepts>

#include <flecs.h>

#include <opack/core/types.hpp>

namespace opack
{

	struct OperationBegin
	{
		flecs::entity_view entity;
	};
	struct OperationEnd
	{
		flecs::entity_view entity;
	};

	template<std::derived_from<Operation> T>
	flecs::entity manipulation(flecs::world& world)
	{
		auto operation = world.entity<T>();
		operation.set<OperationBegin>({ world.entity() });
		operation.set<OperationEnd>({ world.entity() });
		return operation;
	}

	/**
	@brief @c T sense is now able to perceive @c U component.
	@param agent Which agent perceives this
	@return entity of @c U component;
	*/
	template<std::derived_from<Flow> T>
	flecs::entity flow(flecs::world& world, float rate = 1.0f)
	{
		auto flow = world.component<T>();
		flow.template child_of<world::Flows>();

		auto launcher = world.system<const T>()
			.template term<T, Begin>().inout(flecs::Out).set(flecs::Nothing)
			.kind(flecs::OnUpdate)
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
		cleaner.template child_of<opack::dynamics>();

		return flow;
	}

	template<std::derived_from<Operation> TOper, typename ... TInputs>
	class OperationBuilder
	{
	public:
		OperationBuilder(flecs::world& world) : 
			world{ world },
			operation {world.entity<TOper>()},
			system_builder {world.system<TInputs ...>(type_name_cstr<TOper>())}
		{
			operation.child_of<world::Operations>();
			system_builder.kind(flecs::OnUpdate);
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

	private:
		flecs::entity operation;
		flecs::system_builder<TInputs ...> system_builder;
		flecs::world& world;
	};
}