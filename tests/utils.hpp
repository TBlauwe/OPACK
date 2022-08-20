/*****************************************************************//**
 * \file   utils.hpp
 * \brief  Classical utility header.
 * 
 * \author Tristan
 * \date   August 2022
 *********************************************************************/
#pragma once

#include <algorithm>
#include <string>
#include <vector>

#define DOCTEST_VALUE_PARAMETERIZED_DATA(data, data_container)                                  \
    static size_t _doctest_subcase_idx = 0;                                                     \
    std::for_each(data_container.begin(), data_container.end(), [&](const auto& in) {           \
        DOCTEST_SUBCASE((std::string(#data_container "[") +                                     \
                        std::to_string(_doctest_subcase_idx++) + "]").c_str()) { data = in; }  \
    });                                                                                         \
    _doctest_subcase_idx = 0
