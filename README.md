parsertl17: The Modular Parser Generator
========

parsertl is a header-only library for constructing parsers at runtime.

As well as normal parsing, it is possible to use the library as a more powerful regex.

### Classic Calculator Example

```cpp
#include <lexertl/generator.hpp>
#include <lexertl/iterator.hpp>
#include <lexertl/rules.hpp>
#include <lexertl/state_machine.hpp>
#include <parsertl/enums.hpp>
#include <parsertl/generator.hpp>
#include <parsertl/iterator.hpp>
#include <parsertl/rules.hpp>
#include <parsertl/state_machine.hpp>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <stack>
#include <string>

struct data_t
{
    lexertl::state_machine _lsm;
    parsertl::state_machine _gsm;
    std::map<uint16_t, void(*)(std::stack<int>&, const parsertl::citerator&)>
        _actions;
};

static void build_calc_parser(data_t& data)
{
    lexertl::rules lrules;
    parsertl::rules grules;

    grules.token("INTEGER");
    grules.left("'+' '-'");
    grules.left("'*' '/'");
    grules.precedence("UMINUS");

    grules.push("start", "exp");

    data._actions[grules.push("exp", "exp '+' exp")] =
        [](std::stack<int>& stack, const parsertl::citerator&)
        {
            const int rhs_ = stack.top();

            stack.pop();
            stack.top() = stack.top() + rhs_;
        };
    data._actions[grules.push("exp", "exp '-' exp")] =
        [](std::stack<int>& stack, const parsertl::citerator&)
        {
            const int rhs_ = stack.top();

            stack.pop();
            stack.top() = stack.top() - rhs_;
        };
    data._actions[grules.push("exp", "exp '*' exp")] =
        [](std::stack<int>& stack, const parsertl::citerator&)
        {
            const int rhs_ = stack.top();

            stack.pop();
            stack.top() = stack.top() * rhs_;
        };
    data._actions[grules.push("exp", "exp '/' exp")] =
        [](std::stack<int>& stack, const parsertl::citerator&)
        {
            const int rhs_ = stack.top();

            stack.pop();
            stack.top() = stack.top() / rhs_;
        };

    grules.push("exp", "'(' exp ')'");

    data._actions[grules.push("exp", "'-' exp %prec UMINUS")] =
        [](std::stack<int>& stack, const parsertl::citerator&)
        {
            stack.top() *= -1;
        };
    data._actions[grules.push("exp", "INTEGER")] =
        [](std::stack<int>& stack, const parsertl::citerator& iter)
        {
            stack.push(atoi(iter.dollar(0).first));
        };

    parsertl::generator::build(grules, data._gsm);

    lrules.push(R"(\+)", grules.token_id("'+'"));
    lrules.push("-", grules.token_id("'-'"));
    lrules.push(R"(\*)", grules.token_id("'*'"));
    lrules.push(R"(\/)", grules.token_id("'/'"));
    lrules.push(R"(\d+)", grules.token_id("INTEGER"));
    lrules.push(R"(\()", grules.token_id("'('"));
    lrules.push(R"(\))", grules.token_id("')'"));
    lrules.push(R"(\s+)", lexertl::rules::skip());
    lexertl::generator::build(lrules, data._lsm);
}

int main()
{
    try
    {
        data_t data;

        build_calc_parser(data);

        std::string expr("1 + 2 * -3");
        lexertl::citerator liter(expr.c_str(), expr.c_str() + expr.size(), data._lsm);
        parsertl::citerator giter(liter, data._gsm);
        std::stack<int> stack;

        for (; giter->entry.action != parsertl::action::error &&
            giter->entry.action != parsertl::action::accept;
            ++giter)
        {
            auto iter = data._actions.find(giter->entry.param);

            if (iter != data._actions.end())
                iter->second(stack, giter);
        }

        if (giter->entry.action == parsertl::action::accept)
            std::cout << expr << " = " << stack.top() << '\n';
        else
            std::cout << "Parse error.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}
```

### Match a string with captures

```cpp
#include <lexertl/iterator.hpp>
#include <parsertl/generator.hpp>
#include <parsertl/match.hpp>

#include <iostream>

int main()
{
    try
    {
        parsertl::rules grules(+parsertl::rule_flags::enable_captures);
        parsertl::state_machine gsm;
        lexertl::rules lrules;
        lexertl::state_machine lsm;

        grules.token("Int Name");
        grules.push("list", "(item) "
            "| list ',' (item)");
        grules.push("item", "Int | Name");
        parsertl::generator::build(grules, gsm);

        lrules.push(R"([A-Z_a-z]\w*)", grules.token_id("Name"));
        lrules.push(R"(\d+)", grules.token_id("Int"));
        lrules.push(",", grules.token_id("','"));
        lrules.push(R"(\s+)", lexertl::rules::skip());
        lexertl::generator::build(lrules, lsm);

        std::string input = "One, 2, Three, Four";
        lexertl::citerator iter(input.c_str(), input.c_str() + input.size(), lsm);
        using capture_vector = std::vector<std::pair<const char *, const char *>>;
        std::vector<capture_vector> captures;

        if (parsertl::match(iter, gsm, captures))
        {
            auto cvi = captures.cbegin();
            auto cve = captures.cend();

            for (; cvi != cve ; ++cvi)
            {
                auto vi = cvi->cbegin();
                auto ve = cvi->cend();

                for (; vi != ve; ++vi)
                {
                    std::cout << std::string(vi->first, vi->second) << '\n';
                }
            }
        }
        else
        {
            std::cout << "No match\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
    }

    return 0;
}
```

This outputs:

```
One, 2, Three, Four
One
2
Three
Four
```

To search a string with captures, switch `match()` for `search()` above:

```cpp
parsertl::search(iter, gsm, captures)
```

The `captures` argument can be omitted if it is not required in both cases.

You can use an iterator instead of calling search:

```cpp
    lexertl::citerator liter(input.c_str(), input.c_str() + input.size(), lsm);
    parsertl::csearch_iterator iter(liter, gsm);
    parsertl::csearch_iterator end;

    for (; iter != end; ++iter)
    {
        for (const auto &vec : *iter)
        {
            for (const auto &pair : vec)
            {
                std::cout << std::string(pair.first, pair.second) << '\n';
            }
        }

        std::cout << '\n';
    }
```

## More examples and documentation

For documentation see http://www.benhanson.net/parsertl.html
