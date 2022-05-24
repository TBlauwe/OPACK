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

		auto cleaner = world.system<const T>()
			.term(world.pair<T, Begin>())
			.kind(flecs::PostUpdate)
			.iter(
				[](flecs::iter& it)
				{
					for (auto i : it)
					{
						std::cout << "Cleaning flow for agent : " << it.entity(i).doc_name() << "\n";
						it.entity(i).remove<T, Begin>();
					}
				}
		);
		cleaner.set_doc_name("System_CleanFlowLeftOver");

		auto launcher = world.system<const T>()
			.term(world.pair<T, Begin>()).inout(flecs::Out).set(flecs::Nothing)
			.interval(rate)
			.iter(
				[](flecs::iter& it)
				{
					for (auto i : it)
					{
						std::cout << "Launching flow for agent : " << it.entity(i).doc_name() << "\n";
						it.entity(i).add<T, Begin>();
					}
				}
		);
		launcher.set_doc_name("System_LaunchFlow");

		return flow;
	}

	template<std::derived_from<Flow> TFlow, std::derived_from<Operation> TOper>
	class OperationBuilder
	{
	public:
		OperationBuilder(flecs::world& world) : 
			world{ world },
			operation {world.entity<TOper>()},
			system_builder {world.system(type_name_cstr<TOper>())}
		{
		}

		template<std::derived_from<Operation> T>
		OperationBuilder& after()
		{
			system_builder.term<T, End>().inout(flecs::In);
			return *this;
		}

		template<typename ... Args>
		OperationBuilder& input()
		{
			(system_builder.term<Args>().inout(flecs::In), ...);
			return *this;
		}

		template<typename ... Args>
		OperationBuilder& output()
		{
			(system_builder.term<Args>().inout(flecs::Out).set(flecs::Nothing), ...);
			return *this;
		}

		template<typename T>
		flecs::entity build(T&& func)
		{
			if (predecessor_count == 0)
			{
				system_builder.term<TFlow, Begin>().inout(flecs::In);
			}

			system_builder.iter(
				[&func](flecs::iter& it)
				{
					for (auto i : it)
					{
						func(it.entity(i));
					}
				}
			);
			return operation;
		}

	private:
		size_t predecessor_count{ 0 };
		flecs::entity operation;
		flecs::system_builder<> system_builder;
		flecs::world& world;
	};
}