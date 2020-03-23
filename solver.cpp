#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <limits>

enum class clause_data : int8_t { normal, negated, unspec };

struct clause {
    std::vector<int8_t> term;

    clause(int length /*, stdin*/) : term(length, clause_data::unspec) {
        int reader;
        while ((std::cin >> reader) && reader) {
            if (reader > 0) {
                term[reader - 1] = clause_data::normal;
            } else {
                term[-reader - 1] = clause_data::negated;
            }
        }
    }
}

std::vector<clause> parse(/*stdin*/) {
    char end_of_comment;
    while ((std::cin >> end_of_comment) && end_of_comment == 'c') {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // ignore hardcoded "cnf"
    std::cin.ignore();

    int number_of_variables, number_of_clauses;
    std::cin >> number_of_variables >> number_of_clauses;

    std::vector<clause> formula(number_of_clauses);

    for (int i = 0; i < number_of_clauses; ++i) {
        formula[i] = clause(number_of_variables);
    }

    return formula;
}

int main() {

    auto formula = parse(/*stdin*/);

    return 0;
}
