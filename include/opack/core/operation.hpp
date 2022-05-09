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
	/**
	@brief Used to form a pair with a flow to indicate when a flow is starting.
	*/
	struct Begin {};

	/**
	@brief @c T sense is now able to perceive @c U component.
	@param agent Which agent perceives this
	@return entity of @c U component;
	*/
	template<std::derived_from<Flow> T>
	void flow(flecs::world& world)
	{
		auto flow = world.component<T>();

		auto cleaner = world.system<T>()
			.kind(flecs::PostUpdate)
			.arg(1).obj(flecs::Wildcard)
			.iter(
				[](flecs::iter& it)
				{
					for (auto i : it)
					{
						it.entity(i).remove<T>(it.id(1).object());
					}
				}
		);
		cleaner.set_doc_name("System_CleanFlowLeftOver");

		//return world.system()
		//	.term<const opack::Agent>()
		//	.term<const T>()
		//	.template term<T>().template obj<Begin>().inout(flecs::Out).set(flecs::Nothing)
		//	.iter(
		//		[](flecs::iter& it)
		//		{
		//			for (auto i : it)
		//			{
		//				it.entity(i).add<T, Begin>();
		//			}
		//		}
		//);
	}

	template<std::derived_from<Flow> TFlow, std::derived_from<Operation> TOper>
	class OperationBuilder
	{
		OperationBuilder(flecs::world& world) : world{ world }
		{
			operation = world.component<TOper>();
			system_builder = world.system<const Agent, const TOper>();
		}

		template<typename ... Args>
		OperationBuilder& input()
		{
			(system_builder.term<Args>(), ...);
		}

		template<typename ... Args>
		OperationBuilder& output()
		{
			(system_builder.term<Args>()., ...);
		}

		template<typename T>
		flecs::entity build(T&& func)
		{
			system_builder.each(func);
			return operation;
		}

	private:
		flecs::entity operation;
		flecs::system_builder<> system_builder;
		flecs::world& world;
	};
}