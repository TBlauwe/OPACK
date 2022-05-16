/*********************************************************************
 * \file   perception.hpp
 * \brief  File containing related class/utilities for perception.
 *
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <vector>
#include <unordered_set>
#include <functional>

#include <flecs.h>
#include <opack/core/types.hpp>
#include <opack/utils/flecs_helper.hpp>

namespace opack
{
	/**
	@brief A percept represents a perceivable entity with a component or relation.
	*/
	struct Percept
	{
		enum class Type { Component, Relation };
		Type type{ Type::Component };

		flecs::entity_view	sense;
		flecs::entity_view	subject;
		flecs::entity_view	predicate;
		flecs::entity_view	object; // Only set if @c type is equal to @c Type::Component.

		Percept(flecs::entity_view _sense, flecs::entity_view _subject, flecs::entity_view _predicate) :
			type{ Type::Component }, sense{ _sense }, subject{ _subject }, predicate{ _predicate }, object{}
		{}

		Percept(flecs::entity_view _sense, flecs::entity_view _subject, flecs::entity_view _predicate, flecs::entity_view _object) :
			type{ Type::Relation }, sense{ _sense }, subject{ _subject }, predicate{ _predicate }, object{ _object }
		{}

		/**
		@brief Return true if percept is using given sense @c T.
		*/
		template<std::derived_from<Sense> T = Sense>
		inline bool with_sense() { return sense == sense.world().id<T>(); }

		/**
		@brief Return true if subject is equal to @c _subject.
		*/
		inline bool subject_is(flecs::entity _subject) { return subject == _subject; }

		/**
		@brief Return true if predicate is of type @c T .
		*/
		template<typename T>
		inline bool predicate_is() { return predicate == predicate.world().id<T>(); }

		/**
		@brief Return true if object is equal to @c _object.
		*/
		inline bool object_is(flecs::entity _object) { return object == _object; }

		/**
		@brief Return true if percept is a relation of type @c R with object @c object.
		*/
		template<typename R>
		inline bool is_relation(flecs::entity object)
		{
			auto world = subject.world();
			return world.pair<R>(object) == world.pair(predicat, object);
		}

		/**
		@brief Retrieve the current value from the perceived entity.
		Pointer stability is not guaranteed. Copy value if you need to keep it.
		Does not check if @c T is indeed accessible from this sense and if @c target do have it.
		Use @c is() if you wish to check this beforehand.
		*/
		template<typename T>
		inline const T* value() { return subject.get<T>(); }
	};
	using Percepts = std::vector<Percept>;

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
			Component(flecs::world& world);
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
			Relation(flecs::world& world);
		};
	}

	/**
	@brief @c T sense is now able to perceive @c U component.
	@param agent Which agent perceives this
	@return entity of @c U component;
	*/
	template<std::derived_from<Sense> T = Sense, typename U>
	inline flecs::entity perceive(flecs::world& world)
	{
		return world.component<T>().template add<Sense, U>();
	}

	/**
	@brief @c observer is now able to perceive @c subject through @c T sense.
	*/
	template<std::derived_from<Sense> ... T>
	inline void perceive(flecs::entity observer, flecs::entity subject)
	{
		(observer.add<T>(subject), ...);
	}

	/**
	@brief @c source is now not able to perceive @c target through @c T sense.
	*/
	template<std::derived_from<Sense> ...T>
	inline void conceal(flecs::entity source, flecs::entity target)
	{
		(source.remove<T>(target), ...);
	}

	/**
	 Iterate all currently perceived entities with sense @c T, or any if unspecified, with component @c U.
	 @param observer From which perserpective this should be checked.
	 @param func Signature is : void(flecs::entity subject, flecs::entity value)

	 TODO For some reasons, it doesn't work with components from a prefab.
	 */
	template<std::derived_from<Sense> T = opack::Sense, typename U>
	void each_perceived(flecs::entity observer, std::function<void(flecs::entity, const U&)> func)
	{
		auto world = observer.world();
		auto query = world.get<queries::perception::Component>();
		auto rule = query->rule.rule.iter()
			.set_var(query->observer_var, observer)
			.set_var(query->predicate_var, world.id<U>());
		;
		if constexpr (!std::is_same<T, opack::Sense>::value)
		{
			rule.set_var(query->sense_var, world.id<T>());
		}

		std::unordered_set<flecs::entity_t> hide{};
		rule.iter(
			[&](flecs::iter& it)
			{
				auto subject = it.get_var(query->subject_var);
				if (!hide.contains(subject))
				{
					func(subject, *subject.get<U>());
					hide.emplace(subject);
				}
			}
		);
	}

	/**
	 Iterate all currently perceived entities with sense @c T, or any if unspecified, with relation @c R.
	 @param observer From which perserpective this should be checked.
	 @param func Signature is : void(flecs::entity subject, flecs::entity object)
	 */
	template<std::derived_from<Sense> T = opack::Sense, typename R>
	void each_perceived_relation(flecs::entity observer, std::function<void(flecs::entity, flecs::entity)> func)
	{
		auto world = observer.world();
		auto query = world.get<queries::perception::Relation>();
		auto rule = query->rule.rule.iter()
			.set_var(query->observer_var, observer)
			.set_var(query->predicate_var, world.id<R>());
		;
		if constexpr (!std::is_same<T, opack::Sense>::value)
		{
			rule.set_var(query->sense_var, world.id<T>());
		}

		std::unordered_set<flecs::entity_t> hide{};
		rule.iter(
			[&](flecs::iter& it)
			{
				auto subject = it.get_var(query->subject_var);
				if (!hide.contains(subject))
				{
					func(subject, it.get_var(query->object_var));
					hide.emplace(subject);
				}
			}
		);
	}

	template<std::derived_from<Sense> T = opack::Sense, typename U = void>
	bool does_perceive(flecs::entity observer, flecs::entity subject)
	{
		auto world = observer.world();
		return does_perceive<T, U>(world, observer, subject);
	}

	/**
	 * Return true if @c observer is currently perceiving @c subject trough sense @c T, or any sense, if @c T is not specified.
	 @param observer From which perserpective this should be checked.
	 @param
	 */
	template<std::derived_from<Sense> T = opack::Sense, typename U = void>
	bool does_perceive(flecs::world& world, flecs::entity observer, flecs::entity subject)
	{
		{
			auto query = world.get<queries::perception::Component>();
			auto rule = query->rule.rule.iter()
				.set_var(query->observer_var, observer)
				.set_var(query->subject_var, subject)
				;
			if constexpr (!std::is_same<T, opack::Sense>::value)
			{
				rule.set_var(query->sense_var, world.id<T>());
			}
			if constexpr (!std::is_same<U, void>::value)
			{
				rule.set_var(query->predicate_var, world.id<U>());
			}

			if (rule.is_true())
				return true;
		}
		{
			auto query = world.get<queries::perception::Relation>();
			auto rule = query->rule.rule.iter()
				.set_var(query->observer_var, observer)
				.set_var(query->subject_var, subject)
				;
			if constexpr (!std::is_same<T, opack::Sense>::value)
			{
				rule.set_var(query->sense_var, world.id<T>());
			}
			if constexpr (!std::is_same<U, void>::value)
			{
				rule.set_var(query->predicate_var, world.id<U>());
			}
			return rule.is_true();
		}
	}

	/**
	 * Return true if @c observer is currently perceiving relation @c R of @c subject with @c object trough sense @c T.
	 * Return false if @c object is not perceived by @c observer with identical sense @c T.
	 */
	template<std::derived_from<Sense> T = opack::Sense, typename R = void>
	bool does_perceive(flecs::entity observer, flecs::entity subject, flecs::entity object)
	{
		auto world = observer.world();
		if (!does_perceive<T>(observer, object))
			return false;

		{
			auto query = world.get<queries::perception::Relation>();
			auto rule = query->rule.rule.iter()
				.set_var(query->observer_var, observer)
				.set_var(query->subject_var, subject)
				.set_var(query->object_var, object)
				;
			if constexpr (!std::is_same<T, opack::Sense>::value)
			{
				rule.set_var(query->sense_var, world.id<T>());
			}
			if constexpr (!std::is_same<R, void>::value)
			{
				rule.set_var(query->predicate_var, world.id<R>());
			}
			return rule.is_true();
		}
	}

	/**
	Query all percepts for a specific agent.

	TODO With default type @c opack::Sense no perception are retrieved since there is no percepts retrievable with this Sense.
	Maybe specialize function to return all percepts ?

	@return A vector of all percepts of this type, perceived by the agent.
	*/
	template<std::derived_from<Sense> T = opack::Sense>
	Percepts query_percepts(flecs::entity observer)
	{
		auto world = observer.world();
		auto sense = world.id<T>();
		Percepts percepts{};
		{
			auto query = world.get<queries::perception::Component>();
			auto rule = query->rule.rule;
			{
				rule.iter()
					.set_var(query->observer_var, observer)
					.set_var(query->sense_var, sense)
					.iter(
						[&](flecs::iter& it)
						{
							percepts.push_back(Percept{ it.get_var(query->sense_var), it.get_var(query->subject_var), it.get_var(query->predicate_var) });
						}
					)
					;
			}
		}
		{
			auto query = world.get<queries::perception::Relation>();
			auto rule = query->rule.rule;
			rule.iter()
				.set_var(query->observer_var, observer)
				.set_var(query->sense_var, sense)
				.iter(
					[&](flecs::iter& it)
					{
						percepts.push_back(Percept{ it.get_var(query->sense_var), it.get_var(query->subject_var), it.get_var(query->predicate_var), it.get_var(query->object_var) });
					}
				)
				;
		}
		return percepts;
	}
}
