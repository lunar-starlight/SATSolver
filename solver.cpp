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

enum class clause_data : int8_t { normal, negated, unspec };

typedef std::pair<int, clause_data> Literal;

constexpr Literal neg(const Literal& lit)
{
    auto [literal, polarity] = lit;
    switch (polarity) {
    case clause_data::normal:
        return std::make_pair(literal, clause_data::negated);
    case clause_data::negated:
        return std::make_pair(literal, clause_data::normal);
    default:
        __builtin_unreachable();
    }
}

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

    clause(const Literal& lit) : term(LENGTH, clause_data::unspec)
    {
        auto [literal, polarity] = lit;
        term.at(literal) = polarity;
    }

    std::optional<clause> unit_propagate(const std::map<int, clause_data>& units)
    {
        for (int literal = 0; literal < LENGTH; literal++) {
            if (units.find(literal) != units.end()) {
                if (units.at(literal) == term.at(literal)) {
                    return std::nullopt;
                } else {
                    term.at(literal) = clause_data::unspec;
                }
            }
        }
        return *this;
    }

    std::optional<clause> unit_propagate(const Literal& lit)
    {
        auto [literal, polarity] = lit;
        if (term.at(literal) == clause_data::unspec) {
            return *this;
        } else if (term.at(literal) == polarity) {
            return std::nullopt;
        } else {
            term.at(literal) = clause_data::unspec;
            return *this;
        }
    }

    std::optional<Literal> unit() const
    {
        bool b = false;
        int k;
        for (int literal = 0; literal < LENGTH; literal++) {
            if (term.at(literal) != clause_data::unspec) {
                if (b) {
                    b = false;
                    break;
                }
                b = true;
                k = literal;
            }
        }
        if (b) {
            return std::make_pair(k, term.at(k));
        } else {
            return std::nullopt;
        }
    }

    bool has_term(const Literal& lit) const
    {
        return term.at(lit.first) != clause_data::unspec;
    }

};

struct Formula {
    std::vector<clause> formula;
    std::map<int, clause_data> solution;

    void print() const
    {
        for (auto&& cl : formula) {
            std::cout << "(|";
            for (int literal = 0; literal < LENGTH; literal++) {
                switch (cl.term.at(literal)) {
                case clause_data::normal:
                    std::cout << literal + 1 << '|';
                    break;
                case clause_data::negated:
                    std::cout << '!' << literal + 1 << '|';
                    break;
                default:;
                }
            }
            std::cout << ')';
        }
        std::cout << std::endl;
    }
    void print_solution() const
    {
        for (auto&& [literal, polarity] : solution) {
            switch (polarity) {
            case clause_data::normal:
                std::cout << literal + 1 << "=1, ";
                break;
            case clause_data::negated:
                std::cout << literal + 1 << "=0, ";
                break;
            default:;
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
                auto [literal, polarity] = *p;
                if (units.find(literal) == units.end()) {
                    units.insert(*p);
                } else if (units.at(literal) != polarity) {
                    return std::nullopt; // contradiction
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
            for (int literal = 0; literal < LENGTH; ++literal) {
                if (cl.term.at(literal) != clause_data::unspec) {
                    return std::make_pair(std::make_pair(literal, clause_data::negated), std::make_pair(literal, clause_data::normal));
                }
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
        std::cerr << "pre" << std::endl;
        auto units = unit_clauses();
        std::cerr << "post" << std::endl;
        for (; !units.has_value() || !(*units).empty(); units = unit_clauses()) {
            // std::cerr << 'h' << std::endl;
            if (!units.has_value()) {
                return false; // contradiction
            }
            solution.insert((*units).begin(), (*units).end());
            unit_propagate((*units));
        }
        // std::cerr << "post" << std::endl;

        if (auto p = choose_literal()) {
            Formula ff(*this); // copy for branch
            auto [l, l_] = *p;
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
    std::cin.ignore(10, 'f');

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

    std::cout << formula.DPLL() << std::endl;

    return 0;
}
