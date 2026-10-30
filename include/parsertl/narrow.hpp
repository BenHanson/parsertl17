// narrow.hpp
// Copyright (c) 2014-2023 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PARSERTL_NARROW_HPP
#define PARSERTL_NARROW_HPP

#include <sstream>
#include <string>
#include <string_view>

namespace parsertl
{
    template<typename char_type>
    void narrow(const std::basic_string_view<char_type>& vw_, std::ostringstream& ss_)
    {
        for (const char_type c_ : vw_)
        {
            // Safe to simply cast to char when string only contains ASCII.
            ss_ << static_cast<char>(c_);
        }
    }

    template<typename char_type>
    void narrow(const std::basic_string<char_type>& str_, std::ostringstream& ss_)
    {
        narrow(std::basic_string_view<char_type>(str_), ss_);
    }
}

#endif
