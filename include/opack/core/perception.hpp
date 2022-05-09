/*****************************************************************//**
 * \file   perception.hpp
 * \brief  File containing related class/utilities for perception.
 * 
 * \author Tristan
 * \date   May 2022
 *********************************************************************/
#pragma once

#include <vector>

#include <flecs.h>

namespace opack
{	
	/**
	@brief A percept is a small class to tell what can be perceived for a given entity and sense.
	*/
	struct Percept
	{
		enum class Type { Component, Relation };
		Type type{ Type::Component };

		flecs::entity_view	sense;
		flecs::entity_view	subject;
		flecs::entity_view	predicat;
		flecs::entity_view	object; // Only set if @c type is equal to @c Type::Component.

		Percept(flecs::entity_view _sense, flecs::entity_view _subject, flecs::entity_view _predicat) :
			type{ Type::Component }, sense{ _sense }, subject{ _subject }, predicat{ _predicat }, object{}
		{}

		Percept(flecs::entity_view _sense, flecs::entity_view _subject, flecs::entity_view _predicat, flecs::entity_view _object) :
			type{ Type::Relation }, sense{ _sense }, subject{ _subject }, predicat{ _predicat }, object{ _object }
		{}

		/**
		@brief Tells if percept is using given sense.
		*/
		template<std::derived_from<Sense> T = Sense>
		inline bool use() { return sense == sense.world().id<T>(); }

		template<typename T>
		inline bool is_pred() { return predicat == predicat.world().id<T>(); }

		template<typename T>
		inline bool is_pred(flecs::entity object)
		{
			auto world = subject.world();
			return world.pair<T>(object) == world.pair(predicat, object);
		}

		/**
		@brief Retrieve the current value from the perceived entity.
		Pointer stability is not guaranteed. Copy value if you need to keep it.
		Does not check if @c T is indeed accessible from this sense and if @c target do have it.
		Use @c is() if you wish to check this beforehand.
		*/
		template<typename T>
		inline const T* fetch() { return subject.get<T>(); }
	};
	using Percepts = std::vector<Percept>;
}
