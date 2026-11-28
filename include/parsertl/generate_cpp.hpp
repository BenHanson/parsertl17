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
    template<typename char_type>
    class basic_cpp
    {
    public:
        using rules = basic_rules<char_type>;
        using ostream = std::basic_ostream<char_type>;

        static void dump(const rules& rules_, ostream& stream_)
        {
            std::size_t start_ = rules_.start();
            const production_vector& grammar_ = rules_.grammar();
            const token_info_vector& tokens_info_ = rules_.tokens_info();
            const std::size_t terminals_ = tokens_info_.size();
            string_vector symbols_;
            std::set<std::size_t> seen_;
            token_map map_;

            rules_.symbols(symbols_);

            // Skip EOI token
            for (std::size_t idx_ = 1; idx_ < terminals_; ++idx_)
            {
                const token_info& token_info_ = tokens_info_[idx_];
                token_prec_assoc info_(token_info_._precedence,
                    token_info_._associativity);
                auto map_iter_ = map_.find(info_);
                const auto& symbol_ = symbols_[idx_];

                if (info_.second == rules::associativity::token_assoc &&
                    ((symbol_.front() == static_cast<char_type>('\'') &&
                        symbol_.back() == static_cast<char_type>('\'')) ||
                        (symbol_.front() == static_cast<char_type>('"') &&
                        symbol_.back() == static_cast<char_type>('"'))))
                {
                    continue;
                }

                if (map_iter_ == map_.end())
                {
                    if (symbol_.front() == '"' && symbol_.back() == '"')
                        map_.insert(token_pair(info_, escape_dquotes(symbol_)));
                    else
                        map_.insert(token_pair(info_, symbol_));
                }
                else
                {
                    map_iter_->second += static_cast<char_type>(' ');

                    if (symbol_.front() == '"' && symbol_.back() == '"')
                        map_iter_->second += escape_dquotes(symbol_);
                    else
                        map_iter_->second += symbol_;
                }
            }

            for (const auto& pair_ : map_)
            {
                switch (pair_.first.second)
                {
                case rules::associativity::token_assoc:
                    token(stream_);
                    break;
                case rules::associativity::precedence_assoc:
                    precedence(stream_);
                    break;
                case rules::associativity::non_assoc:
                    nonassoc(stream_);
                    break;
                case rules::associativity::left_assoc:
                    left(stream_);
                    break;
                case rules::associativity::right_assoc:
                    right(stream_);
                    break;
                default:
                    break;
                }

                stream_ << static_cast<char_type>('"') <<
                    pair_.second <<
                    static_cast <char_type>('"') <<
                    static_cast <char_type>(')') <<
                    static_cast <char_type>(';') <<
                    static_cast<char_type>('\n');
            }

            if (start_ != static_cast<std::size_t>(~0))
            {
                auto str_ = symbols_[terminals_ + start_];

                if (str_ == "$accept")
                {
                    start_ = grammar_.back()._rhs._symbols.front()._id;
                }

                if (start_ != 0)
                {
                    stream_ << static_cast<char_type>('\n');
                    start(stream_);
                    stream_ << static_cast<char_type>('"') <<
                        symbols_[terminals_ + start_] <<
                        static_cast<char_type>('"') <<
                        static_cast<char_type>(')') <<
                        static_cast<char_type>(';') <<
                        static_cast<char_type>('\n');
                }
            }

            stream_ << static_cast<char_type>('\n');
            dump_grammar(grammar_, seen_, symbols_, terminals_, stream_);
        }

    private:
        using production = typename rules::production;
        using production_vector = typename rules::production_vector;
        using string = std::basic_string<char_type>;
        using string_vector = typename rules::string_vector;
        using symbol = typename rules::symbol;
        using token_prec_assoc =
            std::pair<std::size_t, typename rules::associativity>;
        using token_info = typename rules::token_info;
        using token_info_vector = typename rules::token_info_vector;
        using token_map = std::map<token_prec_assoc, string>;
        using token_pair = std::pair<token_prec_assoc, string>;

        static void dump_grammar(const production_vector& grammar_,
            std::set<std::size_t>& seen_, const string_vector& symbols_,
            const std::size_t terminals_, ostream& stream_)
        {
            for (auto iter_ = grammar_.cbegin(), end_ = grammar_.cend();
                iter_ != end_; ++iter_)
            {
                if (seen_.find(iter_->_lhs) == seen_.end())
                {
                    auto lhs_iter_ = iter_;
                    std::size_t index_ = lhs_iter_ - grammar_.begin();
                    const auto& lhs_ = symbols_[terminals_ + lhs_iter_->_lhs];

                    if (lhs_ == "$accept")
                        continue;

                    push(stream_);
                    stream_ << static_cast<char_type>('"') << lhs_ <<
                        static_cast<char_type>('"') <<
                        static_cast<char_type>(',') <<
                        static_cast<char_type>(' ') <<
                        static_cast<char_type>('"');

                    while (index_ != static_cast<std::size_t>(~0))
                        dump_production(grammar_, lhs_iter_, symbols_,
                            terminals_, index_, stream_);

                    seen_.insert(iter_->_lhs);
                    stream_ << static_cast<char_type>('"') <<
                        static_cast<char_type>(')') <<
                        static_cast<char_type>(';') <<
                        static_cast<char_type>('\n');
                }
            }
        }

        static void dump_production(const production_vector& grammar_,
            typename production_vector::const_iterator& lhs_iter_,
            const string_vector& symbols_, const std::size_t terminals_,
            std::size_t& index_, ostream& stream_)
        {
            if (lhs_iter_->_rhs._symbols.empty())
            {
                stream_ << static_cast<char_type>(' ');
                empty(stream_);
            }
            else
            {
                auto rhs_iter_ = lhs_iter_->_rhs._symbols.cbegin();
                auto rhs_end_ = lhs_iter_->_rhs._symbols.cend();
                std::size_t idx_ = 0;

                for (; rhs_iter_ != rhs_end_; ++rhs_iter_)
                {
                    const std::size_t id_ =
                        rhs_iter_->_type == symbol::type::TERMINAL ?
                        rhs_iter_->_id :
                        terminals_ + rhs_iter_->_id;

                    // Don't dump '$'
                    if (id_ > 0)
                    {
                        const auto& symbol_ = symbols_[id_];

                        if (idx_)
                            stream_ << static_cast<char_type>(' ');

                        if (symbol_.front() == '"' && symbol_.back() == '"')
                            stream_ << escape_dquotes(symbol_);
                        else
                            stream_ << symbol_;

                        ++idx_;
                    }
                }
            }

            if (!lhs_iter_->_rhs._prec.empty())
            {
                stream_ << static_cast<char_type>(' ');
                prec(stream_);
                stream_ << lhs_iter_->_rhs._prec;
            }

            index_ = lhs_iter_->_next_lhs;

            if (index_ != static_cast<std::size_t>(~0))
            {
                const string& lhs_ =
                    symbols_[terminals_ + lhs_iter_->_lhs];

                stream_ << static_cast<char_type>(' ') <<
                    static_cast<char_type>('"') <<
                    static_cast<char_type>('\n');
                lhs_iter_ = grammar_.cbegin() + index_;
                stream_ << string(lhs_.size(), static_cast<char_type>(' ')) <<
                    static_cast<char_type>('"') <<
                    static_cast<char_type>('|') <<
                    static_cast<char_type>(' ');
            }
        }

        static void dump_rhs(std::size_t& j_, const production& p_,
            const std::size_t terminals_, const cursor& pair_,
            const string_vector& symbols_, ostream& stream_)
        {
            for (; j_ < p_._rhs._symbols.size(); ++j_)
            {
                const symbol& symbol_ = p_._rhs._symbols[j_];
                const std::size_t id_ = symbol_._type ==
                    symbol::type::TERMINAL ? symbol_._id :
                    terminals_ + symbol_._id;

                if (j_ == pair_._index)
                {
                    stream_ << static_cast<char_type>(' ') <<
                        static_cast<char_type>('.');
                }

                stream_ << static_cast<char_type>(' ') <<
                    symbols_[id_];
            }
        }

        static void empty(std::ostream& stream_)
        {
            stream_ << "%empty";
        }

        static void empty(std::wostream& stream_)
        {
            stream_ << L"%empty";
        }

        static void empty(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"%empty";
        }

        static void left(std::ostream& stream_)
        {
            stream_ << "grules.left(";
        }

        static void left(std::wostream& stream_)
        {
            stream_ << L"grules.left(";
        }

        static void left(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"grules.left(";
        }

        static void nonassoc(std::ostream& stream_)
        {
            stream_ << "grules.nonassoc(";
        }

        static void nonassoc(std::wostream& stream_)
        {
            stream_ << L"grules.nonassoc(";
        }

        static void nonassoc(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"grules.nonassoc(";
        }

        static void prec(std::ostream& stream_)
        {
            stream_ << "%prec ";
        }

        static void prec(std::wostream& stream_)
        {
            stream_ << L"%prec ";
        }

        static void prec(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"%prec ";
        }

        static void precedence(std::ostream& stream_)
        {
            stream_ << "grules.precedence(";
        }

        static void precedence(std::wostream& stream_)
        {
            stream_ << L"grules.precedence(";
        }

        static void precedence(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"grules.precedence(";
        }

        static void push(std::ostream& stream_)
        {
            stream_ << "grules.push(";
        }

        static void push(std::wostream& stream_)
        {
            stream_ << L"grules.push(";
        }

        static void push(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"grules.push(";
        }

        static void right(std::ostream& stream_)
        {
            stream_ << "grules.right(";
        }

        static void right(std::wostream& stream_)
        {
            stream_ << L"grules.right(";
        }

        static void right(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"grules.right(";
        }

        static void start(std::ostream& stream_)
        {
            stream_ << "grules.start(";
        }

        static void start(std::wostream& stream_)
        {
            stream_ << L"grules.start(";
        }

        static void start(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"grules.start(";
        }

        static void token(std::ostream& stream_)
        {
            stream_ << "grules.token(";
        }

        static void token(std::wostream& stream_)
        {
            stream_ << L"grules.token(";
        }

        static void token(std::basic_ostream<char32_t>& stream_)
        {
            stream_ << U"grules.token(";
        }

        static std::string escape_dquotes(const std::string& str_)
        {
            std::string output_ = '\\' + str_;

            output_.insert(output_.size() - 1, 1, '\\');
            return output_;
        }
    };

    using cpp = basic_cpp<char>;
    using wcpp = basic_cpp<wchar_t>;
    using u32cpp = basic_cpp<char32_t>;

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
