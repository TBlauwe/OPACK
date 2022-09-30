/*****************************************************************//**
 * \file   api_types.hpp
 * \brief  API types.
 * 
 * \author Tristan 
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <unordered_map>
#include <concepts>

#include <flecs.h>
#include <fmt/core.h>

#include <opack/utils/debug.hpp>			// So it's available everywhere
#include <opack/utils/flecs_helper.hpp>
#include <opack/core/macros.hpp>

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
	using EntityView = flecs::entity_view;

	class HandleView : public flecs::entity_view
	{
		using entity_view::entity_view;
	};

	struct Handle : flecs::entity
	{
		using entity::entity;
	};

	struct MessageHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct MessageHandle : Handle
	{
		using Handle::Handle;
	};

	/** @}*/ //End of group

	/**
	 *@brief Namespace used to organize entities in explorer. If @c OPACK_ORGANIZE is defined,
	 * then it will not be used.
	 */
	namespace world 
	{
		/** @brief Scope to regroup dynamic entities (systems, observer, trigger, etc.).*/
		struct dynamics {};

		/** @brief Scope to regroup rules/query entities.*/
		struct rules {};
	};

	//--------------------------
	// Foundational types
	//--------------------------
	struct Tangible {};

	OPACK_FUNDAMENTAL_TYPE(Agent, agents), Tangible
	{};

	OPACK_FUNDAMENTAL_TYPE(Artefact, artefacts), Tangible
	{};

    OPACK_FUNDAMENTAL_TYPE(Action, actions)
    {};

    OPACK_FUNDAMENTAL_TYPE(Actuator, actuators)
	{};

    OPACK_FUNDAMENTAL_TYPE(Message, messages)
	{};

    OPACK_FUNDAMENTAL_TYPE(Sense, senses)
	{};

    OPACK_FUNDAMENTAL_TYPE(Flow, flows)
	{};

    OPACK_FUNDAMENTAL_TYPE(Operation, operations)
	{};

    OPACK_FUNDAMENTAL_TYPE(Behaviour, behaviours)
	{};

	// Phases
	//--------
	/** @addtogroup Phases

	Phases are a mean provided by flecs to organize systems call order. Here, we define three
	groups of 3 phases : "Perceive", "Reason" and "Act". Each are also divided by 3 other phases :
	"PreUpdate", "Update", "PostUpdate".

    During "Perceive" phases, all operations/systems related to this, otherwise, those that manipulate P,
    must be finished.

    Afterwards, during "Reason" phases, this is where all reasoning is happening, even action selection.

    Phases in "Act" are reserved to handle actions update. Three phases are necessary in order to begin action,
    tick them and ending them in one cycle.

	@{
	*/
	namespace Cycle
	{
		struct Begin{};
	}
	namespace Perceive
	{
	    struct PreUpdate{};
	    struct Update{};
	    struct PostUpdate{};
	};

	namespace Reason
	{
	    struct PreUpdate{};
	    struct Update{};
	    struct PostUpdate{};
	};

	namespace Act
	{
	    struct PreUpdate{};
	    struct Update{};
	    struct PostUpdate{};
	};

	namespace Cycle
	{
		struct End{};
	}

	/** @}*/ //End of group

	// A - Action
	//-----------
	using Actions_t = std::vector<flecs::entity>;
	using Action_t = flecs::entity;

	enum class ActionStatus
	{
		waiting,
		starting,
		running,
		suspended,
		resumed,
		aborted,//TODO should add unaboratble and unresumable action logic + abortable and resumable logic
		finished	
	};

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
    };

	template<typename T>
	concept HasRoot = requires
	{
	    typename T::root_t;
	};

	template<typename T>
	concept HasHandle = requires
	{
	    typename T::handle_t;
	    typename T::handle_view_t;
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

	namespace internal
	{
		template<typename T>
		void organize_entity(flecs::entity&) {}

		template<typename T>
			requires (HasRoot<T>&& HasFolder<typename T::root_t>)
		void organize_entity(Entity& entity)
		{
#ifndef OPACK_ORGANIZE
			if (!entity.name())
				opack::internal::name_entity_after_type<T>(entity);
			entity.child_of<typename T::root_t::entities_folder_t>();
#endif
		}

		template<HasFolder T>
		void create_module_entity(World& world)
		{
#ifndef OPACK_ORGANIZE
			world.entity<typename T::entities_folder_t>().add(flecs::Module);
#endif
		}
	}
}

