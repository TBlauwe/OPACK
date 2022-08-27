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

namespace opack
{
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
		world.observer()
			.event(flecs::OnAdd)
			.term(flecs::IsA).second<TAgent>()
			.each(
				[](flecs::entity e)
				{
					auto child = e.world().entity().is_a<TSense>().child_of(e);
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
	template<SubPrefab T>
	Entity sense(const Entity& entity)
	{
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
	template<std::derived_from<Sense> ... T>
	void perceive(Entity observer, Entity subject)
	{
		(opack::sense<T>(observer).add(subject), ...);
	}

	/**
	@brief @c source is now not able to perceive @c target through @c T sense.
	*/
	template<std::derived_from<Sense> ...T>
	void conceal(Entity observer, Entity subject)
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
		perception(Entity observer) : observer{observer}{}

		/**
		 *@brief Check if @c observer is perceiving @c subject with sense @c T.
		 *@return True, if @c subject is perceived, false otherwise.

        If no sense type is specified, it defaults to @c opack::Sense.
        Which means it asks, whether the observer perceive @c subject, with any sense.
        In order to do so, a more complex call is required, which is more expensive.
        When possible, you should specify a sense.
		 */
		template<std::derived_from<Sense> T = opack::Sense>
	    bool perceive(const Entity subject) const
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
	    bool perceive(const Entity subject) const
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
	    bool perceive(const Entity subject, const Entity object) const
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
	    bool perceive(const Entity subject, const Entity object) const
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
	    const C* value(const Entity subject) const
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
		    return {};
		}

		Entity observer;
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
	//@brief A percept represents a perceivable entity with a component or relation.
	//*/
	//struct Percept
	//{
	//	enum class Type { Component, Relation };
	//	Type type{ Type::Component };

	//	EntityView	sense;
	//	EntityView	subject;
	//	EntityView	predicate;
	//	EntityView	object; // Only set if @c type is equal to @c Type::Component.

	//	Percept(EntityView _sense, EntityView _subject, EntityView _predicate) :
	//		type{ Type::Component }, sense{ _sense }, subject{ _subject }, predicate{ _predicate }, object{}
	//	{}

	//	Percept(EntityView _sense, EntityView _subject, EntityView _predicate, EntityView _object) :
	//		type{ Type::Relation }, sense{ _sense }, subject{ _subject }, predicate{ _predicate }, object{ _object }
	//	{}

	//	/**
	//	@brief Return true if percept is using given sense @c T.
	//	*/
	//	template<std::derived_from<Sense> T = Sense>
	//	bool with_sense() { return sense == sense.world().id<T>(); }

	//	/**
	//	@brief Return true if subject is equal to @c _subject.
	//	*/
	//	bool subject_is(const Entity _subject) const { return subject == _subject; }

	//	/**
	//	@brief Return true if predicate is of type @c T .
	//	*/
	//	template<typename T>
	//	bool predicate_is() { return predicate == predicate.world().id<T>(); }

	//	/**
	//	@brief Return true if object is equal to @c _object.
	//	*/
	//	bool object_is(Entity _object) { return object == _object; }

	//	/**
	//	@brief Return true if percept is a relation of type @c R with object @c object.
	//	*/
	//	template<typename R>
	//	bool is_relation(Entity object)
	//	{
	//		auto world = subject.world();
	//		return world.pair<R>(object) == world.pair(predicate, object);
	//	}

	//	/**
	//	@brief Retrieve the current value from the perceived entity.
	//	Pointer stability is not guaranteed. Copy value if you need to keep it.
	//	Does not check if @c T is indeed accessible from this sense and if @c target do have it.
	//	Use @c is() if you wish to check this beforehand.
	//	*/
	//	template<typename T>
	//	const T* value() { return subject.get<T>(); }
	//};
	//using Percepts = std::vector<Percept>;

	namespace queries::perception
	{
		/**
		 * Query :
		 * @code
			$Sense($Observer, $Subject), $Predicate($Subject), opack.Sense($Sense, $Predicate)
		 * @endcode.
		 */
		struct Component
		{
			internal::Rule rule;
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
		struct Relation
		{
			internal::Rule rule;
			int32_t observer_var;
			int32_t sense_var;
			int32_t subject_var;
			int32_t predicate_var;
			int32_t object_var;
			Relation(World& world);
		};
	}

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
