/*****************************************************************//**
 * \file   api_types.hpp
 * \brief  API types.
 * 
 * \author Tristan 
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <functional>
#include <unordered_map>
#include <map>
#include <concepts>

#include <flecs.h>

/**
@brief Helper macro to define additional structs to organize entities in explorer.
@param name Folder's name.
*/
#define OPACK_FOLDERS_STRUCT(name) namespace world { struct name { struct prefabs {}; };  }

/**
@brief Helper macro to define adequate typedefs for organization.
@param name Type's name.
*/
#define OPACK_FOLDERS_TYPEDEF(name) \
    using entities_folder_t = world::name; \
    using prefabs_folder_t = world::name::prefabs

/**
@brief Define a new type named @c name to identify a tag.

@param name Type's name

Usage :
@code{.cpp}
OPACK_TAG(MyTag);
//can be used as a regular type, eg:
opack::has<MyTag>(some_entity); 
@endcode
*/
#define OPACK_TAG(name) struct name {}

/**
@brief Define a new type named @c name to identify a prefab.

@param name Type's name

Usage :
@code{.cpp}
OPACK_PREFAB(A);
//can be then used to identify a prefab anywhere in code, eg:
opack::prefab<A>(world); 
@endcode
*/
#define OPACK_PREFAB(name) struct name {}

/**
@brief Define a new type named @c name to identify a prefab.
@brief Define a new type named @c name to identify a prefab instantiated from a prefab named @c base.

@param name Type's name
@param base Prefab type's name, from which to inherit.

Usage :
@code{.cpp}
OPACK_PREFAB(A);
OPACK_SUB_PREFAB(B, A);
//can be then used to identify a prefab anywhere in code, eg:
opack::prefab<A>(world); 
opack::init<B>(world); 
@endcode
*/
#define OPACK_SUB_PREFAB(name, base) struct name : public base {using base_t = base;}

/**
@brief Library namespace.
*/
namespace opack
{
	namespace _
	{
		template<typename T>
	    struct root
		{
			using root_t = T;
		};
	}
	/** @addtogroup Flecs 

	OPACK is built around flecs. To ensure minimal friction, we do not want to encapsulate its basic types. 
	There is currently no reason to do it. However, this library is build with two users in mind : student and pro-user.
	Pro-user refers to those who already know flecs. Student refers to those who may not know flecs, or even c++.
	For this reason, we try to be as easy to use as possible (given the scope) to newcomers, while not impending pro-users.
	Hence the use of a typedef.

	@{
	*/

	/**
	@brief A world store all information. 
	See https://www.flecs.dev/flecs/#/docs/Quickstart?id=world
	*/
    using World = flecs::world;

	/**
	@brief An entity is an unique id, to which are associated components. 
	See https://www.flecs.dev/flecs/#/docs/Quickstart?id=entity
	*/
	using Entity = flecs::entity;

	/**
	@brief An entity_view is an immutable entity handle. 
	See https://flecs.docsforge.com/master/api-cpp/flecs/entity_view/
	*/
	using Entity_view = flecs::entity_view;

	/** @}*/ //End of group

	/**
	 *@brief Namespace used to organize entities in explorer. If @c OPACK_OPTIMIZE is defined,
	 * then it will not be used.
	 */
	namespace world 
	{
		/** @brief Scope to regroup dynamic entities (systems, observer, trigger, etc.).*/
		struct dynamics {};

		/** @brief Scope to regroup behaviours.*/
		struct behaviours {};

		/** @brief Scope to regroup prefab entities.*/
		struct prefabs {};

		/** @brief Scope to regroup rules/query entities.*/
		struct rules {};

		/** @brief Scope to regroup modules.*/
		struct modules {};
	};

	// MAS - Multi-agent systems
	//--------------------------
    OPACK_FOLDERS_STRUCT(agents);
	struct Agent : public _::root<Agent>
	{
        OPACK_FOLDERS_TYPEDEF(agents);
	};

    OPACK_FOLDERS_STRUCT(artefacts);
	struct Artefact : public _::root<Artefact>
	{
        OPACK_FOLDERS_TYPEDEF(artefacts);
	};

    OPACK_FOLDERS_STRUCT(actions);
	struct Action : public _::root<Action>
	{
        OPACK_FOLDERS_TYPEDEF(actions);
	};

    OPACK_FOLDERS_STRUCT(messages);
	struct Message : public _::root<Message>
	{
        OPACK_FOLDERS_TYPEDEF(messages);
	};

    OPACK_FOLDERS_STRUCT(actuators);
	struct Actuator : public _::root<Actuator>
	{
        OPACK_FOLDERS_TYPEDEF(actuators);
	};

    OPACK_FOLDERS_STRUCT(senses);
	struct Sense : public _::root<Sense>
	{
        OPACK_FOLDERS_TYPEDEF(actuators);
	};

    OPACK_FOLDERS_STRUCT(flows);
	struct Flow : public _::root<Flow>
	{
        OPACK_FOLDERS_TYPEDEF(flows);
	};

    OPACK_FOLDERS_STRUCT(operations);
	struct Operation : public _::root<Operation>
	{
        OPACK_FOLDERS_TYPEDEF(operations);
	};

	// O - Operation
	//--------------
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
						auto impact = object.get_second<TOper, impact_t>();
						if (impact)
							impacts.push_back(impact);
					}
				);
			}

			flecs::entity	agent{};
			impacts_t		impacts{};
		};
	};

	// A - Action
	//-----------
	using Actions_t = std::vector<flecs::entity>;
	using Action_t = flecs::entity;

	// K - Knowledge
	//--------------
	struct Knowledge {};
	// TODO

	// MISC
	//-----

	// Concepts
	//--------------
	template<typename T>
	concept DefaultConstructible = std::is_default_constructible_v<T>;

	template<typename T, typename U>
	concept Composable = requires (T t, U u)
	{
		{ t.template add<U>() };
		{ t.template set<U>({u}) };
		{ t.template get<U>() } -> std::same_as<const U*>;
		{ t.template get_mut<U>() } -> std::same_as<U*>;
	};

	template<typename T>
	concept HasFolder = requires 
    {
        typename T::entities_folder_t;
        typename T::prefabs_folder_t;
    };

	template<typename T>
	concept HasRoot = requires
	{
	    typename T::root_t;
	};

	template<typename T>
	concept SubPrefab = requires { typename T::base_t; };

	template<typename T>
	concept NotSubPrefab = requires { !SubPrefab<T>; };

	template<typename T>
	concept AgentPrefab = SubPrefab<T> && std::derived_from<T, Agent>;

	template<typename T>
	concept ArtefactPrefab = SubPrefab<T> && std::derived_from<T, Artefact>;

	template<typename T>
	concept ActionPrefab = SubPrefab<T> && std::derived_from<T, Action>;

	template<typename T>
	concept ActuatorPrefab = SubPrefab<T> && std::derived_from<T, Actuator>;

	template<typename T>
	concept SensePrefab = SubPrefab<T> && std::derived_from<T, Sense>;
}
