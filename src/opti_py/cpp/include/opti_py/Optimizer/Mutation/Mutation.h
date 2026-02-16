#ifndef MUTATION_H
#define MUTATION_H

#include <vector>

#include "SolutionBuilder.h"


class Mutation {
public:
    virtual ~Mutation() = default;

    virtual std::vector<double> mutate(
        const std::vector<std::vector<double>>& population,
        int targetIndex,
        double F,
        const std::vector<double>& bestVector,
        SolutionBuilder& builder
    ) = 0;
};

#endif