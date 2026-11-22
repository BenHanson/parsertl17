// state_machine.hpp
// Copyright (c) 2014-2026 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PARSERTL_GENERATE_CPP_HPP
#define PARSERTL_GENERATE_CPP_HPP

#include "state_machine.hpp"

#include <iostream>

namespace parsertl
{
    class table_based_cpp
    {
    public:
        template<typename id_type>
        static void dump_tables(const parsertl::basic_state_machine<id_type>& sm_,
            std::ostream& os_)
        {
            output_tabs(1, os_);
            os_ << "gsm._columns = " << sm_._columns << ";\n";
            output_tabs(1, os_);
            os_ << "gsm._rows = " << sm_._rows << ";\n";

            for (const auto& rule_ : sm_._rules)
            {
                std::size_t item_ = 0;

                output_tabs(1, os_);
                os_ << "gsm._rules.push_back({ " << rule_._lhs << ", { ";

                for (const id_type rhs_ : rule_._rhs)
                {
                    if (item_ > 0)
                        os_ << ", ";

                    os_ << rhs_;
                    ++item_;
                }

                os_ << " } });\n";
            }

            if (!sm_._captures.empty())
            {
                for (const auto& pair_ : sm_._captures)
                {
                    output_tabs(1, os_);
                    os_ << "gsm._captures.push_back({" <<
                        pair_.first << ", {} });\n";

                    for (const auto& capture : pair_.second)
                    {
                        output_tabs(1, os_);
                        os_ << "gsm._captures.back().second.emplace_back(" <<
                            capture.first << ", " << capture.second <<
                            ");\n";
                    }
                }
            }

            for (const auto& row : sm_._table)
            {
                output_tabs(1, os_);
                os_ << "gsm._table.emplace_back();\n";

                for (const auto& pair : row)
                {
                    output_tabs(1, os_);
                    os_ << "gsm._table.back().push_back( {" <<
                        pair._id << ", { ";

                    switch (pair._entry.action)
                    {
                    case action::error:
                        os_ << "parsertl::action::error";
                        break;
                    case action::shift:
                        os_ << "parsertl::action::shift";
                        break;
                    case action::reduce:
                        os_ << "parsertl::action::reduce";
                        break;
                    case action::go_to:
                        os_ << "parsertl::action::go_to";
                        break;
                    case action::accept:
                        os_ << "parsertl::action::accept";
                        break;
                    }

                    os_ << ", " << pair._entry.param << " } } );\n";
                }
            }
        }
    protected:
        static void output_tabs(const std::size_t tabs_, std::ostream& os_)
        {
            for (std::size_t i_ = 0; i_ < tabs_; ++i_)
            {
                os_ << "    ";
            }
        }

    };
}

#endif
