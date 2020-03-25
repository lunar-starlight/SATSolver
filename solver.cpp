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
        // for (auto [literal, mode] : units) {
        for (auto [literal, mode] : term) {
            auto p = units.find(literal);
            if (p == units.end()) {
                t[literal] = mode;
            } else if ((*p).second == mode) {
                return std::nullopt;
            }
        }
        term = t;
        return *this;
    }

    std::optional<clause> unit_propagate(const Literal& lit) const
    {
        auto [literal, mode] = lit;
        auto p = term.find(literal);
        if (p == term.end()) {
            return *this;
        } else if ((*p).second == mode) {
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

    std::map<int, clause_data> unit_clauses()
    {
        std::vector<clause> f; f.reserve(formula.size());
        std::map<int, clause_data> units;
        for (auto&& e : formula) {
            if (auto p = e.unit()) {
                units.insert(*p);
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
        // for (auto&& el : pure_literals()) {
        //     solution.insert(el);
        //     unit_propagate(el);
        // }
        auto pures = pure_literals();
        solution.insert(pures.begin(), pures.end());
        unit_propagate(pures);
        // for (auto&& el : formula) {
        //     el.unit_propagate(pures);
        // }
        return _DPLL();
    }

    bool _DPLL()
    {
        // print();
        // print_solution();
        // std::cout << "pre_empty" << std::endl;
        if (formula.empty()) {
            return true;
        }
        // std::cout << "post_empty" << std::endl;
        // std::cout << "pre_empty_cl" << std::endl;
        if (contains_empty_clause()) {
            return false;
        }
        // std::cout << "post_empty_cl" << std::endl;
        // std::cout << "pre_unit" << std::endl;
        auto units = unit_clauses();
        solution.insert(units.begin(), units.end());
        // std::cout << "post_unit" << std::endl;
        // std::cout << units.size() << std::endl;
        unit_propagate(units);
        // for (auto&& el : formula) {
        //     // solution.insert(el);
        //     el.unit_propagate(units);
        // }
        // std::cout << "pre_choose" << std::endl;
        if (auto p = choose_literal()) {
            auto [l_, l] = *p;
            // std::cout << "post_choose" << std::endl;
            // std::cout << "pre_clone" << std::endl;
            Formula ff(*this);
            // std::cout << "post_clone" << std::endl;
            // std::cout << "pre_prop" << std::endl;
            solution.insert(l);
            unit_propagate(l);
            // std::cout << "post_prop" << std::endl;
            if (_DPLL()) {
                return true;
            } else {
                // std::cout << "branch" << std::endl;
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

    // formula.print();
    std::cout << formula.DPLL() << '\n';
    // formula.print_solution();

    return 0;
}
