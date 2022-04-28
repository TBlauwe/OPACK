/*********************************************************************
 * \file   type_map.hpp
 * \brief  Defines an associative container of type / value.  
 * 
 * \author Tristan
 * \date   April 2022
 *********************************************************************/
#pragma once

#include <any>
#include <typeindex>
#include <unordered_map>
#include <optional>


/**
@class TypeMap

@brief Defines an associative container of type / value.

Usage :
@code{.cpp}
TypeMap map;
T elt = map.add<T>(); // Add and return an element by type.
T elt = map.get<T>(); // Get element by type.
@endcode
*/
class TypeMap
{

public:
    /**
    @brief Return const element of type @c T.
    */
    template<class T>
    const T& get() const
    {
        return std::any_cast<const T&>(container.at(typeid(T)));
    }

    /**
    @brief Return mutable element of type @c T.
    */
    template<class T>
    T& get_mut()
    {
        return std::any_cast<T&>(container.at(typeid(T)));
    }

    /**
    @brief Add a new element of type @c T. If already present, returns associated element.

    @tparam Must be @c DefaultConstructible.
    */
    template<class T>
    T& add()
    {
        if (container.contains(typeid(T)))
            return std::any_cast<T&>(container[typeid(T)]);
        else
            return std::any_cast<T&>(container[typeid(T)] = T());
    }

    /**
    @brief Return number of elements.
    */
    size_t size() const
    {
        return container.size();
    }

private:
    std::unordered_map<std::type_index, std::any> container;
};
