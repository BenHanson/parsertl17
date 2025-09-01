# Part of CMake configuration for parsertl library
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
include_directories(.)

set(target parsertl_include_tests)
add_executable(${target}
    bison_lookup.cpp
    debug.cpp
    dfa.cpp
    ebnf_tables.cpp
    enums.cpp
    generator.cpp
    include_test.cpp
    include_test.sln
    include_test.vcxproj
    include_test.vcxproj.filters
    include_test.vcxproj.user
    iterator.cpp
    lookup.cpp
    match.cpp
    match_results.cpp
    narrow.cpp
    nt_info.cpp
    parse.cpp
    read_bison.cpp
    rules.cpp
    runtime_error.cpp
    search.cpp
    search_iterator.cpp
    serialise.cpp
    state_machine.cpp
    token.cpp)
include_directories(../../../lexertl17/include)
target_link_libraries(${target} PRIVATE parsertl)
