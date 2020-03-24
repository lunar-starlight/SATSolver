#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <functional>
#include <optional>

int LENGTH;

enum class clause_data : int8_t { normal, negated, unspec };

typedef std::pair<int, clause_data> Literal;

struct clause {
    std::vector<clause_data> term;

    clause(const int& length /*, stdin*/) : term(length, clause_data::unspec)
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

    clause(const Literal& lit, const int& length) : term(length, clause_data::unspec)
    {
        auto [literal, mode] = lit;
        term[literal - 1] = mode;
    }

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

void print(const Formula& formula)
{
    for (auto&& cl : formula) {
        std::cout << "(|";
        for (size_t i = 0; i < cl.term.size(); i++) {
            switch (cl.term[i]) {
            case clause_data::normal:
                std::cout << i << '|';
                break;
            case clause_data::negated:
                std::cout << '!' << i << '|';
                break;
            default:
                break;
            }
        }
        std::cout << ')';
    }
    std::cout << std::endl;
}

Formula parse(/*stdin*/)
{
    char end_of_comment;
    while ((std::cin >> end_of_comment) && end_of_comment == 'c') {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // ignore hardcoded "cnf"
    std::cin >> end_of_comment;
    std::cin >> end_of_comment;
    std::cin >> end_of_comment;

    int number_of_variables, number_of_clauses;
    std::cin >> number_of_variables >> number_of_clauses;
    LENGTH = number_of_variables;
    // std::cout << number_of_variables << ' ' << number_of_clauses << '\n';

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
            if (el.has_term(lit)) {
                b = true;
            }
        }
        return b;
    };
    for (size_t i = 0; i < LENGTH; i++) {
        Literal lit1 = std::make_pair(i + 1, clause_data::normal);
        Literal lit2 = std::make_pair(i + 1, clause_data::negated);
        bool b1 = is_present_in_formula(lit1);
        bool b2 = is_present_in_formula(lit2);
        if (!b1 and b2) {
            pure.push_back(lit2);
        }
        if (b1 and !b2) {
            pure.push_back(lit1);
        }
    }
    return pure;
}

bool contains_empty_clause(const Formula& formula)
{
    for (auto&& cl : formula) {
        bool is_empty = true;
        for (auto&& el : cl.term) {
            if (el != clause_data::unspec) {
                is_empty = false;
            }
        }
        if (is_empty) {
            return true;
        }
    }
    return false;
}

std::pair<Literal, Literal> choose_literal(const Formula& formula)
{
    for (auto&& cl : formula) {
        for (size_t i = 0; i < cl.term.size(); i++) {
            if (cl.term[i] != clause_data::unspec) {
                return std::make_pair(std::make_pair(i + 1, clause_data::normal),
                                      std::make_pair(i + 1, clause_data::negated));
            }
        }
    }
    print(formula);
    __builtin_unreachable();
}

bool DPLL(Formula formula)
{
    for (auto&& el : unit_clauses(formula)) {
        formula = unit_propagate(el, formula);
    }
    for (auto&& el : pure_literals(formula)) {
        formula = unit_propagate(el, formula);
    }
    if (formula.empty()) {
        return true;
    }
    if (contains_empty_clause(formula)) {
        return false;
    }
    auto [l, l_] = choose_literal(formula);
    formula.push_back(clause(l, LENGTH));
    if (DPLL(formula)) {
        return true;
    } else {
        formula.pop_back();
        formula.push_back(clause(l_, LENGTH));
        return DPLL(formula);
    }
}

int main()
{
    auto formula = parse(/*stdin*/);

    // print(formula);

    std::cout << DPLL(formula) << '\n';

    return 0;
}
