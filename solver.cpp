#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <map>

int LENGTH;

enum class clause_data : int8_t { normal, negated };

typedef std::pair<int, clause_data> Literal;

struct clause {
    std::map<int, clause_data> term;

    clause(const int& length /*, stdin*/)
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

    clause(const Literal& lit, const int& length)
    {
        auto [literal, mode] = lit;
        term[literal] = mode;
    }

    std::optional<clause> unit_propagate(const Literal& lit) const
    {
        auto [literal, mode] = lit;
        if (term.find(literal) == term.end()) {
            return *this;
        } else if (term.at(literal) == mode) {
            return std::nullopt;
        } else {
            clause cl(*this);
            cl.term.erase(literal);
            return cl;
        }
    }

    std::optional<Literal> unit() const
    {
        if (term.size() == 1) {
            return *term.begin();
        } else {
            return std::nullopt;
        }
    }

    bool has_term(const Literal& lit) const
    {
        return term.find(lit.first) != term.end();
    }

};

typedef std::vector<clause> Formula;

void print(const Formula& formula)
{
    for (auto&& cl : formula) {
        std::cout << "(|";
        for (auto&& [literal, mode] : cl.term) {
            switch (mode) {
            case clause_data::normal:
                std::cout << literal + 1 << '|';
                break;
            case clause_data::negated:
                std::cout << '!' << literal + 1 << '|';
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
        Literal lit1 = std::make_pair(i, clause_data::normal);
        Literal lit2 = std::make_pair(i, clause_data::negated);
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
        if (cl.term.empty()) {
            return true;
        }
    }
    return false;
}

constexpr Literal neg(const Literal& lit)
{
    auto [literal, mode] = lit;
    switch (mode) {
    case clause_data::normal:
        return std::make_pair(literal, clause_data::negated);
    case clause_data::negated:
        return std::make_pair(literal, clause_data::normal);
    }
}

std::optional<std::pair<Literal, Literal>> choose_literal(const Formula& formula)
{
    for (auto&& cl : formula) {
        if (!cl.term.empty()) {
            return std::make_pair(*cl.term.begin(), neg(*cl.term.begin()));
        }
    }
    return std::nullopt;
}

bool DPLL(Formula formula)
{
    if (formula.empty()) {
        return true;
    }
    if (contains_empty_clause(formula)) {
        return false;
    }
    for (auto&& el : unit_clauses(formula)) {
        formula = unit_propagate(el, formula);
    }
    for (auto&& el : pure_literals(formula)) {
        formula = unit_propagate(el, formula);
    }
    if (auto p = choose_literal(formula)) {
        auto [l, l_] = *p;
        formula.push_back(clause(l, LENGTH));
        if (DPLL(formula)) {
            return true;
        } else {
            formula.pop_back();
            formula.push_back(clause(l_, LENGTH));
            return DPLL(formula);
        }
    } else {
        return DPLL(formula);
    }
}

int main()
{
    auto formula = parse(/*stdin*/);

    // print(formula);

    for (int i = 0; i < 10; i++) {
        std::cout << DPLL(formula) << '\n';
    }

    return 0;
}
