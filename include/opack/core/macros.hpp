/*****************************************************************//**
 * \file   macros.hpp
 * \brief  Defines some macros used internally.
 * 
 * \author Tristan
 * \date   September 2022
 *********************************************************************/
#pragma once

/**
@brief Helper macro to define additional structs to organize entities in explorer.
@param name Folder's name.
*/
#define OPACK_FOLDERS_STRUCT(name) namespace world { struct name { struct prefabs {}; };  }

/**
@brief Helper macro to define a fundamental type in order to reduce boilerplate code.
@param name of fundamental type.
@param plural of fundamental type.
 */
#define OPACK_FUNDAMENTAL_TYPE(name, plural)	\
	struct name ## Handle;						\
	struct name ## HandleView;					\
	OPACK_FOLDERS_STRUCT(plural);				\
	struct name :								\
		opack::internal::root<name>,					\
		opack::internal::handle<name ## Handle, name ## HandleView>, \
		opack::internal::entities_folder<world::plural>

namespace opack::internal
{
	template<typename T>
	struct root
	{
		using root_t = T;
	};

	template<typename Th, typename Tv>
	struct handle
	{
		using handle_t = Th;
		using handle_view_t = Tv;
	};

	template<typename T>
	struct entities_folder
	{
		using entities_folder_t = T;
	};
}
