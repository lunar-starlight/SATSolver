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

struct Formula {
    std::vector<clause> formula;
    std::vector<Literal> solution;

    void print() const
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
    void print_solution() const
    {
        for (auto&& [literal, mode] : solution) {
            switch (mode) {
            case clause_data::normal:
                std::cout << literal + 1 << "=1, ";
                break;
            case clause_data::negated:
                std::cout << literal + 1 << "=0, ";
                break;
            }
        }
        std::cout << std::endl;
    }

    void unit_propagate(const Literal& literal)
    {
        std::vector<clause> f; f.reserve(formula.size());
        for (auto&& e : formula) {
            if (auto cl = e.unit_propagate(literal)) {
                f.push_back(*cl);
            }
        }
        formula = f;
    }

    std::vector<Literal> unit_clauses() const
    {
        std::vector<Literal> units;
        for (auto&& e : formula) {
            if (auto p = e.unit()) {
                units.push_back(*p);
            }
        }
        return units;
    }

    std::vector<Literal> pure_literals() const
    {
        std::vector<Literal> pure; pure.reserve(LENGTH);
        for (size_t i = 0; i < LENGTH; i++) {
            Literal lit1 = std::make_pair(i, clause_data::normal);
            Literal lit2 = std::make_pair(i, clause_data::negated);
            bool b1 = false;
            bool b2 = false;
            for (auto&& el : formula) {
                if (el.has_term(lit1)) {
                    b1 = true;
                }
                if (el.has_term(lit2)) {
                    b2 = true;
                }
            }
            if (!b1 and b2) {
                pure.push_back(lit2);
            }
            if (b1 and !b2) {
                pure.push_back(lit1);
            }
        }
        return pure;
    }

    bool contains_empty_clause() const
    {
        for (auto&& cl : formula) {
            if (cl.term.empty()) {
                return true;
            }
        }
        return false;
    }

    std::optional<std::pair<Literal, Literal>> choose_literal() const
    {
        for (auto&& cl : formula) {
            if (!cl.term.empty()) {
                return std::make_pair(*cl.term.begin(), neg(*cl.term.begin()));
            }
        }
        return std::nullopt;
    }

    bool DPLL()
    {
        if (formula.empty()) {
            return true;
        }
        if (contains_empty_clause()) {
            return false;
        }
        for (auto&& el : unit_clauses()) {
            solution.push_back(el);
            unit_propagate(el);
        }
        for (auto&& el : pure_literals()) {
            solution.push_back(el);
            unit_propagate(el);
        }
        if (auto p = choose_literal()) {
            auto [l, l_] = *p;
            Formula ff(*this);
            formula.push_back(clause(l, LENGTH));
            if (DPLL()) {
                return true;
            } else {
                ff.formula.push_back(clause(l_, LENGTH));
                return ff.DPLL();
            }
        } else {
            return DPLL();
        }
    }
};

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

    Formula f;
    f.formula.reserve(number_of_clauses);

    for (int i = 0; i < number_of_clauses; ++i) {
        f.formula.emplace_back(number_of_variables /*, stdin*/);
    }

    return f;
}

int main()
{
    auto formula = parse(/*stdin*/);

    std::cout << formula.DPLL() << '\n';
    // formula.print_solution();

    return 0;
}
