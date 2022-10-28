/*****************************************************************//**
 * \file   perception.hpp
 * \brief  API for perception capabilities.
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <flecs.h>
#include <opack/core/api_types.hpp>
#include <opack/core/world.hpp>
#include <opack/core/entity.hpp>
#include <opack/utils/flecs_helper.hpp>

/**
@brief Shorthand for OPACK_SUB_PREFAB(name, opack::Sense)
*/
#define OPACK_SENSE(name) OPACK_SUB_PREFAB(name, opack::Sense)

/**
@brief Identical to OPACK_SUB_PREFAB(name, base)
*/
#define OPACK_SUB_SENSE(name, base) OPACK_SUB_PREFAB(name, base)

namespace opack
{
	struct SenseHandleView : HandleView
	{
		using HandleView::HandleView;
	};

	struct SenseHandle : Handle
	{
		using Handle::Handle;
	};

	namespace queries::perception
	{
		/**
		 * Query :
		 * @code
			(IsA, opack.Tangible), $This($Sensor), IsA($Observer, opack.Tangible), $Sens($Observer, $Sensor)
		 * @endcode.
		 */
		struct Entity : internal::Rule
		{
			int32_t observer_var;
			int32_t sense_var;
			Entity(World& world);
		};

		/**
		 * Query :
		 * @code
			$Sense($Observer, $Subject), $Predicate($Subject), opack.Sense($Sense, $Predicate)
		 * @endcode.
		 */
		struct Component : internal::Rule
		{
			int32_t observer_var;
			int32_t sense_var;
			int32_t subject_var;
			int32_t predicate_var;
			Component(World& world);
		};

		/**
		 * Query :
		 * @code
			$Sense($Observer, $Subject), $Predicate($Subject, $Object), opack.Sense($Sense, $Predicate)
		 * @endcode.
		 */
		struct Relation : internal::Rule
		{
			int32_t observer_var;
			int32_t sense_var;
			int32_t subject_var;
			int32_t predicate_var;
			int32_t object_var;
			Relation(World& world);
		};
	}

	/**
	 *@brief Add sense @c T to entity @c prefab
	 *Usage:
	 *@code{.cpp}
	 OPACK_SUB_PREFAB(MyAgent, opack::Agent)
	 OPACK_SUB_PREFAB(MySense, opack::Sense)
     //...
     opack::add_sense<MySense, MyAgent>(world); 
	 *@endcode 
	 */
	template<SensePrefab TSense, std::derived_from<Agent> TAgent>
	void add_sense(World& world)
	{
		// Waiting for fix : https://github.com/SanderMertens/flecs/issues/791
		// NOTE : also need to template sense !
		//opack::prefab<TSense>(world)
		//	.child_of<TAgent>()
	    //  .slot();
		world.observer(fmt::format(fmt::runtime("Observer_AddSense_{}_to_{}"), friendly_type_name<TSense>().c_str(), friendly_type_name<TAgent>().c_str()).c_str())
			.event(flecs::OnAdd)
			.term(flecs::IsA).template second<TAgent>()
			.each(
				[](flecs::entity e)
				{
					auto child = e.world().entity().is_a<TSense>().child_of(e);
					child.set_name(friendly_type_name<TSense>().c_str());
					e.add<TSense>(child);
				}
		).template child_of<world::dynamics>();
	}

	/**
	 * @brief Retrieve instanced sense @c T for current entity.
	 *
	 * WARNING : If you want to retrieve the sense prefab, identified by
	 * @c T, use @ref entity<T>.
	 */
	template<SensePrefab T>
	Entity sense(EntityView entity)
	{
		opack_assert(entity.target<T>(), "No sense {0} for entity {1}. Did you called : `opack::add_sense<{0}, {2}>(world) ?", type_name_cstr<T>(), entity.path().c_str(), entity.has(flecs::IsA, flecs::Wildcard) ? entity.target(flecs::IsA).path().c_str() : "YourAgentType");
		return entity.target<T>();
	}

	/**
	@brief @c T sense is now able to perceive @c U component.
	@return entity of @c U component;

    Usage:
    @code{.cpp}
    opack::perceive<MySense, MyData, MyOtherData, ...>(world);
    @endcode
	*/
	template<std::derived_from<Sense> T = Sense, typename... Us>
	Entity perceive(World& world)
	{
		return (opack::entity<T>(world).template add<Sense, Us>(), ...);
	}

	/**
	@brief @c observer is now able to perceive @c subject through @c T sense.

    Usage:
    @code{.cpp}
    opack::perceive<MySense>(observer, subject);
    @endcode
	*/
	template<SensePrefab ... T>
	void perceive(EntityView observer, EntityView subject)
	{
		(opack::sense<T>(observer).add(subject), ...);
	}

	/**
	@brief @c source is now not able to perceive @c target through @c T sense.
	*/
	template<SensePrefab ...T>
	void conceal(EntityView observer, EntityView subject)
	{
		(opack::sense<T>(observer).remove(subject), ...);
	}

	/**
	 *@brief Struct to query perceptive abilities for an entity
	 *
	 Usage :
	 @code{.cpp}
	 auto p = opack::perception(observer);
	 p.perceive<MySense>(subject); 
	 p.perceive<MySense, MyValue>(subject);
	 p.value<MySense, MyValue>(subject);
	 //etc.
	 @endcode
	 */
	struct perception 
	{
		perception(EntityView _observer) : observer{_observer}{}

		/**
		 *@brief Check if @c observer is perceiving @c subject with sense @c T.
		 *@return True, if @c subject is perceived, false otherwise.

        If no sense type is specified, it defaults to @c opack::Sense.
        Which means it asks, whether the observer perceive @c subject, with any sense.
        In order to do so, a more complex call is required, which is more expensive.
        When possible, you should specify a sense.
		 */
		template<std::derived_from<Sense> T = opack::Sense>
	    bool perceive(EntityView subject) const
        {
			if constexpr (!std::same_as<T, opack::Sense>)
				return opack::sense<T>(observer).has(subject);
			else
				return false;
		}

		/**
		 *@brief Check if @c observer is perceiving component @c C from @c subject with sense @c T.
		 *@return True, if component @c C and @c subject is perceived with sense @c T, false otherwise.

        If no sense type is specified, it defaults to @c opack::Sense.
        Which means it asks, whether the observer perceive @c subject, with any sense.
        In order to do so, a more complex call is required, which is more expensive.
        When possible, you should specify a sense.
		 */
		template<std::derived_from<Sense> T = opack::Sense, typename C>
	    bool perceive(EntityView subject) const
        {
			if constexpr (!std::same_as<T, opack::Sense>)
			{
                if (!opack::sense<T>(observer).template has<Sense, C>())
                    return false;
				return perceive<T>(subject) && subject.has<C>();
			}
			else
			{
				//TODO
			}
		    return false;
		}

		/**
		 *@brief Check if @c observer is perceiving component @c object from @c subject with sense @c T.
		 *@return True, if component @c object and @c subject is perceived with sense @c T, false otherwise.

        If no sense type is specified, it defaults to @c opack::Sense.
        Which means it asks, whether the observer perceive @c subject, with any sense.
        In order to do so, a more complex call is required, which is more expensive.
        When possible, you should specify a sense.
		 */
		template<std::derived_from<Sense> T = opack::Sense>
	    bool perceive(EntityView subject, EntityView object) const
        {
			if constexpr (!std::same_as<T, opack::Sense>)
			{
                if (!opack::sense<T>(observer).template has<Sense>(object))
                    return false;
				return perceive<T>(subject) && subject.has(object);
			}
			else
			{
			    //TODO 	    
			}
		    return false;
        }

		/**
		 *@brief Check if @c observer is perceiving a relation @c R with @c object from @c subject with sense @c T.
		 *@return True, if @c observer perceive @c subject and the relation @c R with @c object, through sense @c T, false otherwise.
		 *If @c object is an agent or an artefact, @c object must also be perceived by @c observer.

        If no sense type is specified, it defaults to @c opack::Sense.
        Which means it asks, whether the observer perceive @c subject, with any sense.
        In order to do so, a more complex call is required, which is more expensive.
        When possible, you should specify a sense.
		 */
		template<std::derived_from<Sense> T = opack::Sense, typename R>
	    bool perceive(EntityView subject, EntityView object) const
        {
			if constexpr (!std::same_as<T, opack::Sense>)
			{
                if (!opack::sense<T>(observer).template has<Sense, R>())
                    return false;
			    if (opack::is_a<Artefact>(object) || opack::is_a<Agent>(object))
					return perceive<T>(subject) && perceive<T>(object) && subject.has<R>(object);
			    return perceive<T>(subject) && subject.has<R>(object);
			}
			else
			{
		        //TODO 	    
			}
		    return false;
		}

		/**
		 *@brief Get value of component @c C from @c subject, only if it is perceivable trough sense @c T.
		 *@return A const pointer to component @c C if it is perceivable, @c nullptr otherwise.

        If no sense type is specified, it defaults to @c opack::Sense.
        Which means it asks, whether the observer perceive @c subject, with any sense.
        In order to do so, a more complex call is required, which is more expensive.
        When possible, you should specify a sense.
		 */
		template<std::derived_from<Sense> T = opack::Sense, typename C>
	    const C* value(EntityView subject) const
        {
			if constexpr (!std::same_as<T, opack::Sense>)
			{
			    if(perceive<T, C>(subject))
				    return subject.get<C>();
			}
			else
			{
		        //TODO 	    
			}
		    return nullptr;
		}

		//TODO Handle at compile time if T is a prefab or tag
		template<typename T>
		void each(std::function<void(Entity, const T*)> func)
		{
		     auto query = observer.world().get<opack::queries::perception::Entity>();
             query->rule.iter()
                .set_var(query->observer_var, observer)
            .each(
                [&func](flecs::iter& it, size_t index)
                {
                    auto subject = it.entity(index);
					if (opack::is_a<T>(subject))
						func(subject, nullptr);
					else if (subject.has<T>())
						func(subject, subject.get<T>());
                }
			 );
		}

		template<typename T>
		void each(std::function<void(Entity)> func)
		{
		     auto query = observer.world().get<opack::queries::perception::Entity>();
             query->rule.iter()
                .set_var(query->observer_var, observer)
            .each(
                [&func](flecs::iter& it, size_t index)
                {
                    auto subject = it.entity(index);
					if (opack::is_a<T>(subject))
						func(subject);
					else if (subject.has<T>())
						func(subject);
                }
			 );
		}

		EntityView observer;
	};

    //template<typename R = void, std::derived_from<Sense> T = opack::Sense>
	//bool does_perceive(Entity observer, Entity subject, Entity object)
	//{
	//	auto world = observer.world();
	//	if (!does_perceive<void, T>(observer, object))
	//		return false;

	//	{
	//		auto query = world.get<queries::perception::Relation>();
	//		auto rule = query->rule.rule.iter()
	//			.set_var(query->observer_var, observer)
	//			.set_var(query->subject_var, subject)
	//			.set_var(query->object_var, object)
	//			;
	//		if constexpr (!std::is_same<T, opack::Sense>::value)
	//		{
	//			rule.set_var(query->sense_var, world.id<T>());
	//		}
	//		if constexpr (!std::is_same<R, void>::value)
	//		{
	//			rule.set_var(query->predicate_var, world.id<R>());
	//		}
	//		return rule.is_true();
	//	}
	//}


	///**
	// Iterate all currently perceived entities with sense @c T, or any if unspecified, with component @c U.
	// @param observer From which perserpective this should be checked.
	// @param func if @c U is not a tag void(Entity subject, const U& value). Otherwise:  void(Entity);

	// TODO For some reasons, it doesn't work with components from a prefab.
	// */
	//template<typename U, std::derived_from<Sense> T = opack::Sense, typename TFunc>
	//void each_perceived(Entity observer, TFunc&& func)
	//{
	//	auto world = observer.world();
	//	auto query = world.get<queries::perception::Component>();
	//	auto rule = query->rule.rule.iter()
	//		.set_var(query->observer_var, observer)
	//		.set_var(query->predicate_var, world.id<U>());
	//	;
	//	if constexpr (!std::is_same<T, opack::Sense>::value)
	//	{
	//		rule.set_var(query->sense_var, world.id<T>());
	//	}

	//	std::unordered_set<Entity_t> hide{};
	//	rule.iter(
	//		[&](flecs::iter& it)
	//		{
	//			auto subject = it.get_var(query->subject_var);
	//			if (!hide.contains(subject))
	//			{
	//				if constexpr (std::is_empty_v<U>)
	//					func(subject);
	//				else
	//					func(subject, *subject.get<U>());
	//				hide.emplace(subject);
	//			}
	//		}
	//	);
	//}

	///**
	// Iterate all currently perceived entities with sense @c T, or any if unspecified, with relation @c R.
	// @param observer From which perserpective this should be checked.
	// @param func Signature is : void(Entity subject, Entity object)
	// */
	//template<typename R, std::derived_from<Sense> T = opack::Sense>
	//void each_perceived_relation(Entity observer, std::function<void(Entity, Entity)> func)
	//{
	//	auto world = observer.world();
	//	auto query = world.get<queries::perception::Relation>();
	//	auto rule = query->rule.rule.iter()
	//		.set_var(query->observer_var, observer)
	//		.set_var(query->predicate_var, world.id<R>());
	//	;
	//	if constexpr (!std::is_same<T, opack::Sense>::value)
	//	{
	//		rule.set_var(query->sense_var, world.id<T>());
	//	}

	//	std::unordered_set<Entity_t> hide{};
	//	rule.iter(
	//		[&](flecs::iter& it)
	//		{
	//			auto subject = it.get_var(query->subject_var);
	//			if (!hide.contains(subject))
	//			{
	//				func(subject, it.get_var(query->object_var));
	//				hide.emplace(subject);
	//			}
	//		}
	//	);
	//}

	///**
	// * Return true if @c observer is currently perceiving @c subject trough sense @c T, or any sense, if @c T is not specified.
	// @param observer From which perserpective this should be checked.
	// @param
	// */
	//template<typename U = void, std::derived_from<Sense> T = opack::Sense>
	//bool does_perceive(World& world, Entity observer, Entity subject)
	//{
	//	{
	//		auto query = world.get<queries::perception::Component>();
	//		auto rule = query->rule.rule.iter()
	//			.set_var(query->observer_var, observer)
	//			.set_var(query->subject_var, subject)
	//			;
	//		if constexpr (!std::is_same<T, opack::Sense>::value)
	//		{
	//			rule.set_var(query->sense_var, world.id<T>());
	//		}
	//		if constexpr (!std::is_same<U, void>::value)
	//		{
	//			rule.set_var(query->predicate_var, world.id<U>());
	//		}

	//		if (rule.is_true())
	//			return true;
	//	}
	//	{
	//		auto query = world.get<queries::perception::Relation>();
	//		auto rule = query->rule.rule.iter()
	//			.set_var(query->observer_var, observer)
	//			.set_var(query->subject_var, subject)
	//			;
	//		if constexpr (!std::is_same<T, opack::Sense>::value)
	//		{
	//			rule.set_var(query->sense_var, world.id<T>());
	//		}
	//		if constexpr (!std::is_same<U, void>::value)
	//		{
	//			rule.set_var(query->predicate_var, world.id<U>());
	//		}
	//		return rule.is_true();
	//	}
	//}

	//template<typename U = void, std::derived_from<Sense> T = opack::Sense>
	//bool does_perceive(Entity observer, Entity subject)
	//{
	//	auto world = observer.world();
	//	return does_perceive<U, T>(world, observer, subject);
	//}

	///**
	// * Return true if @c observer is currently perceiving relation @c R of @c subject with @c object trough sense @c T.
	// * Return false if @c object is not perceived by @c observer with identical sense @c T.
	// */
	//template<typename R = void, std::derived_from<Sense> T = opack::Sense>
	//bool does_perceive(Entity observer, Entity subject, Entity object)
	//{
	//	auto world = observer.world();
	//	if (!does_perceive<void, T>(observer, object))
	//		return false;

	//	{
	//		auto query = world.get<queries::perception::Relation>();
	//		auto rule = query->rule.rule.iter()
	//			.set_var(query->observer_var, observer)
	//			.set_var(query->subject_var, subject)
	//			.set_var(query->object_var, object)
	//			;
	//		if constexpr (!std::is_same<T, opack::Sense>::value)
	//		{
	//			rule.set_var(query->sense_var, world.id<T>());
	//		}
	//		if constexpr (!std::is_same<R, void>::value)
	//		{
	//			rule.set_var(query->predicate_var, world.id<R>());
	//		}
	//		return rule.is_true();
	//	}
	//}

	///**
	//Query all percepts for a specific agent.

	//TODO With default type @c opack::Sense no perception are retrieved since there is no percepts retrievable with this Sense.
	//Maybe specialize function to return all percepts ?

	//@return A vector of all percepts of this type, perceived by the agent.
	//*/
	//template<std::derived_from<Sense> T = opack::Sense>
	//Percepts query_percepts(Entity observer)
	//{
	//	auto world = observer.world();
	//	auto sense = world.id<T>();
	//	Percepts percepts{};
	//	{
	//		auto query = world.get<queries::perception::Component>();
	//		auto rule = query->rule.rule;
	//		{
	//			rule.iter()
	//				.set_var(query->observer_var, observer)
	//				.set_var(query->sense_var, sense)
	//				.iter(
	//					[&](flecs::iter& it)
	//					{
	//						percepts.push_back(Percept{ it.get_var(query->sense_var), it.get_var(query->subject_var), it.get_var(query->predicate_var) });
	//					}
	//				)
	//				;
	//		}
	//	}
	//	{
	//		auto query = world.get<queries::perception::Relation>();
	//		auto rule = query->rule.rule;
	//		rule.iter()
	//			.set_var(query->observer_var, observer)
	//			.set_var(query->sense_var, sense)
	//			.iter(
	//				[&](flecs::iter& it)
	//				{
	//					percepts.push_back(Percept{ it.get_var(query->sense_var), it.get_var(query->subject_var), it.get_var(query->predicate_var), it.get_var(query->object_var) });
	//				}
	//			)
	//			;
	//	}
	//	return percepts;
	//}
}
