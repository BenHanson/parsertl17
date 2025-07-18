// iterator.hpp
// Copyright (c) 2022-2023 Ben Hanson (http://www.benhanson.net/)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef PARSERTL_ITERATOR_HPP
#define PARSERTL_ITERATOR_HPP

#include <lexertl/iterator.hpp>
#include "lookup.hpp"
#include "match_results.hpp"
#include "token.hpp"

namespace parsertl
{
    template<typename lexer_iterator, typename sm_type,
        typename id_type = std::uint16_t>
    class iterator
    {
    public:
        using results = basic_match_results<sm_type>;
        using value_type = results;
        using difference_type = ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        // Qualify token to prevent arg dependant lookup
        using token = parsertl::token<lexer_iterator>;
        using token_vector = typename token::token_vector;

        iterator() = default;

        iterator(const lexer_iterator& iter_, const sm_type& sm_) :
            _iter(iter_),
            _results(_iter->id, sm_),
            _sm(&sm_)
        {
            // The first action can only ever be reduce
            // if the grammar treats no input as valid.
            if (_results.entry.action != action::reduce)
                lookup();
        }

        iterator(const lexer_iterator& iter_, const sm_type& sm_,
            const std::size_t reserved_) :
            _iter(iter_),
            _results(_iter->id, sm_, reserved_),
            _productions(reserved_),
            _sm(&sm_)
        {
            // The first action can only ever be reduce
            // if the grammar treats no input as valid.
            if (_results.entry.action != action::reduce)
                lookup();
        }

        typename token_vector::value_type dollar(const std::size_t index_) const
        {
            return _results.dollar(index_, *_sm, _productions);
        }

        std::size_t production_size(const std::size_t index_) const
        {
            return _results.production_size(*_sm, index_);
        }

        iterator& operator ++()
        {
            lookup();
            return *this;
        }

        iterator operator ++(int)
        {
            iterator iter_ = *this;

            lookup();
            return iter_;
        }

        const value_type& operator *() const
        {
            return _results;
        }

        const value_type* operator ->() const
        {
            return &_results;
        }

        bool operator ==(const iterator& rhs_) const
        {
            return _sm == rhs_._sm &&
                (_sm == nullptr ? true :
                    _results == rhs_._results);
        }

        bool operator !=(const iterator& rhs_) const
        {
            return !(*this == rhs_);
        }

        lexer_iterator lexer_iter() const
        {
            return _iter;
        }

        std::pair<std::size_t, std::size_t> line_column(typename lexer_iterator::
            data_iterator& start) const
        {
            const std::size_t line =
                1 + std::count(start, _iter->first, '\n');
            const typename lexer_iterator::value_type::char_type
                endl[] = { '\n' };
            const std::size_t column = _iter->first - std::find_end(start,
                _iter->first, endl, endl + 1);

            return std::make_pair(line, column);
        }

    private:
        lexer_iterator _iter;
        basic_match_results<sm_type> _results;
        token_vector _productions;
        const sm_type* _sm = nullptr;

        void lookup()
        {
            // do while because we need to move past the current reduce action
            do
            {
                parsertl::lookup(_iter, *_sm, _results, _productions);
            } while (_results.entry.action == action::shift ||
                _results.entry.action == action::go_to);

            switch (_results.entry.action)
            {
            case action::accept:
            case action::error:
                _sm = nullptr;
                break;
            default:
                break;
            }
        }
    };

    using siterator = iterator<lexertl::siterator, state_machine>;
    using citerator = iterator<lexertl::citerator, state_machine>;
    using wsiterator = iterator<lexertl::wsiterator, state_machine>;
    using wciterator = iterator<lexertl::wciterator, state_machine>;
}

#endif
