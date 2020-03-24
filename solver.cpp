#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <functional>
#include <optional>

enum class clause_data : int8_t { normal, negated, unspec };

typedef std::pair<int, clause_data> Literal;

struct clause {
    std::vector<clause_data> term;

    clause(int length /*, stdin*/) : term(length, clause_data::unspec)
    {
        int reader;
        while ((std::cin >> reader) && reader) {
            if (reader > 0) {
                term[reader - 1] = clause_data::normal;
            } else {
                term[-reader - 1] = clause_data::negated;
            }
        }
    }
    clause(const clause& cl) : term(cl.term) {}

    std::optional<clause> unit_propagate(const Literal& lit) const
    {
        auto [literal, mode] = lit;
        if (term[literal - 1] == mode) {
            return std::nullopt;
        } else if (term[literal - 1] == clause_data::unspec) {
            return *this;
        } else {
            clause cl(*this);
            cl.term[literal - 1] = clause_data::unspec;
            return cl;
        }
    }

    std::optional<Literal> unit() const
    {
        int n = 0;
        Literal lit;
        for (size_t i = 0; i < term.size(); i++) {
            if (term[i] != clause_data::unspec) {
                ++n;
                lit = std::make_pair(i + 1, term[i]);
            }
        }
        if (n == 1) {
            return lit;
        } else {
            return std::nullopt;
        }
    }

    bool has_term(const Literal& lit) const
    {
        auto [literal, mode] = lit;
        return term[literal - 1] == mode;
    }
};

typedef std::vector<clause> Formula;

Formula parse(/*stdin*/)
{
    char end_of_comment;
    while ((std::cin >> end_of_comment) && end_of_comment == 'c') {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // ignore hardcoded "cnf"
    std::cin.ignore();

    int number_of_variables, number_of_clauses;
    std::cin >> number_of_variables >> number_of_clauses;

    Formula formula;
    formula.reserve(number_of_clauses);

    for (int i = 0; i < number_of_clauses; ++i) {
        formula.emplace_back(number_of_variables /*, stdin*/);
    }

    return formula;
}

Formula unit_propagate(const Literal& literal, const Formula& formula)
{
    Formula f; f.reserve(formula.size());
    for (auto&& e : formula) {
        if (auto cl = e.unit_propagate(literal)) {
            f.push_back(*cl);
        }
    }
    return f;
}

std::vector<Literal> unit_clauses(const Formula& formula)
{
    std::vector<Literal> units;
    for (auto&& e : formula) {
        if (auto p = e.unit()) {
            units.push_back(*p);
        }
    }
    return units;
}

std::vector<Literal> pure_literals(const Formula& formula)
{
    std::vector<Literal> pure;
    // true if literal is a term of any of the clauses in the formula
    auto is_present_in_formula = [ = ](Literal lit) {
        bool b = false;
        for (auto&& el : formula) {
            if (!el.has_term(lit)) {
                b = true;
            }
        }
        return b;
    };
    for (size_t i = 0; i <  formula.front().term.size(); i++) {
        Literal lit;
        if (!is_present_in_formula(lit = std::make_pair(i + 1, clause_data::normal)) ||
            !is_present_in_formula(lit = std::make_pair(i + 1, clause_data::negated))) {
            pure.push_back(lit);  // if it is not present negated or normal, then it is pure
        }
    }
    return pure;
}


int main()
{
    auto formula = parse(/*stdin*/);

    return 0;
}
