// Copyright 2016 United States Government as represented by the Administrator of the National Aeronautics and Space
// Administration. No copyright is claimed in the United States under Title 17, U.S. Code. All Other Rights Reserved.
//
// The “Parfait: A Toolbox for CFD Software Development [LAR-18839-1]” platform is licensed under the
// Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.
//
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and limitations under the License.
#pragma once

#include <stdexcept>
#include <sstream>
#include <iostream>
#include "Exception.h"

#define PARFAIT_THROW(message)                                                                                  \
    {                                                                                                           \
        throw Parfait::Exception(std::string(message) + "\n at file: " + std::string(__FILE__) +                \
                                 " function: " + std::string(__func__) + " line: " + std::to_string(__LINE__)); \
    }
#define PARFAIT_EXIT(message)                                                                         \
    {                                                                                                 \
        std::cerr << "\n\u2757ERROR:\n"                                                             \
                  << std::string(message) + "\n at file: " + std::string(__FILE__) +                  \
                         " function: " + std::string(__func__) + " line: " + std::to_string(__LINE__) \
                  << std::endl;                                                                       \
        exit(914);                                                                                    \
    }
#define PARFAIT_WARNING(message)                                                                      \
    {                                                                                                 \
        std::cerr << "\n\u2757WARNING: \n"                                                          \
                  << std::string(message) + "\n at file: " + std::string(__FILE__) +                  \
                         " function: " + std::string(__func__) + " line: " + std::to_string(__LINE__) \
                  << std::endl;                                                                       \
    }
#define PARFAIT_ASSERT(boolean_statement, message)                                                                  \
    {                                                                                                               \
        if (not(boolean_statement)) {                                                                               \
            throw Parfait::Exception(std::string("ASSERT_FAILED: ") + std::string(message) +                        \
                                     " at file: " + std::string(__FILE__) + " function: " + std::string(__func__) + \
                                     " line: " + std::to_string(__LINE__) + std::string("\n"));                     \
        }                                                                                                           \
    }
#define PARFAIT_ASSERT_BOUNDS(container, index, message)                                                            \
    {                                                                                                               \
        if ((index < 0) or (long(index) >= long(container.size()))) {                                               \
            throw Parfait::Exception(std::string("BOUNDS FAILURE: index ") + std::to_string(index) +                \
                                     " out of bounds 0," + std::to_string(container.size()) + "\n" +                \
                                     std::string(message) + " at file: " + std::string(__FILE__) +                  \
                                     " function: " + std::string(__func__) + " line: " + std::to_string(__LINE__) + \
                                     std::string("\n"));                                                            \
        }                                                                                                           \
    }
#define PARFAIT_ASSERT_EQUAL(a, b)                                                                                     \
    {                                                                                                                  \
        if (not(a == b)) {                                                                                             \
            throw Parfait::Exception(std::string("ASSERT_FAILED: ") + std::to_string(a) + " != " + std::to_string(b) + \
                                     " at file: " + std::string(__FILE__) + " function: " + std::string(__func__) +    \
                                     " line: " + std::to_string(__LINE__) + std::string("\n"));                        \
        }                                                                                                              \
    }
#define PARFAIT_ASSERT_KEY(my_map, key) PARFAIT_ASSERT(my_map.count(key) == 1, "Map key not found")
#define PARFAIT_ASSERT_KEY_MSG(my_map, key, message) \
    PARFAIT_ASSERT(my_map.count(key) == 1, std::string("No key found") + message)
