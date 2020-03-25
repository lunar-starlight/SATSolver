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

    std::optional<clause> unit_propagate(const std::map<int, clause_data>& units)
    {
        std::map<int, clause_data> t;
        for (auto [literal, mode] : term) {
            if (units.find(literal) == units.end()) {
                t[literal] = mode;
            } else if (units.at(literal) == mode) {
                return std::nullopt;
            }
        }
        term = t;
        return *this;
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
    std::map<int, clause_data> solution;

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

    template<typename T>
    void unit_propagate(const T& units)
    {
        std::vector<clause> f; f.reserve(formula.size());
        for (auto&& e : formula) {
            if (auto cl = e.unit_propagate(units)) {
                f.push_back(*cl);
            }
        }
        formula = f;
    }

    std::optional<std::map<int, clause_data>> unit_clauses()
    {
        std::vector<clause> f; f.reserve(formula.size());
        std::map<int, clause_data> units;
        for (auto&& e : formula) {
            if (auto p = e.unit()) {
                auto [literal, mode] = *p;
                if (units.find(literal) == units.end()) {
                    units.insert(*p);
                } else if (units.at(literal) != mode) {
                    return std::nullopt;
                }
            } else {
                f.push_back(e);
            }
        }
        formula = f;
        return units;
    }

    std::map<int, clause_data> pure_literals() const
    {
        std::map<int, clause_data> pure;
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
                pure.insert(lit2);
            }
            if (b1 and !b2) {
                pure.insert(lit1);
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
        auto pures = pure_literals();
        solution.insert(pures.begin(), pures.end());
        unit_propagate(pures);
        return _DPLL();
    }

    bool _DPLL()
    {
        if (formula.empty()) {
            return true;
        }
        if (contains_empty_clause()) {
            return false;
        }
        for (auto units = unit_clauses(); units.has_value() && !(*units).empty(); units = unit_clauses()) {
            if (!units.has_value()) {
                return false; // contradiction
            }
            solution.insert((*units).begin(), (*units).end());
            unit_propagate((*units));
        }

        if (auto p = choose_literal()) {
            Formula ff(*this); // copy for branch
            auto [l_, l] = *p;
            solution.insert(l);
            unit_propagate(l);

            if (_DPLL()) {
                return true;
            } else {
                ff.solution.insert(l_);
                ff.unit_propagate(l_);
                return ff._DPLL();
            }
        } else {
            return _DPLL();
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

    return 0;
}
