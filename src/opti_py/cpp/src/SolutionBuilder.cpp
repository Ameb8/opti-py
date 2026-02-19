#include "SolutionBuilder.h"

#include <algorithm>


std::vector<double> SolutionBuilder::getRand() {
    std::vector<double> solution(dimensions);

    // Generate random solutions
    for(int i = 0; i < dimensions; i++) {{
            solution[i] = lower + (upper - lower) * mt.genrand_real1(); 
        }
    }
    
    return solution;
}

double SolutionBuilder::checkBounds(double value) {
    if(value > upper)
        value = upper;
    else if(value < lower)
        value = lower;

    return value;
}

std::vector<std::vector<double>> SolutionBuilder::getNeighbors(
    const std::vector<double>& center,
    int numNeighbors,
    double maxDelta
) {
    std::vector<std::vector<double>> solutions(numNeighbors, std::vector<double>(dimensions));

    // Randomly generate neighbors within maxDelta for each dimension
    for(int i = 0; i < numNeighbors; i++) {
        for(int j = 0; j < dimensions; j++) {
            // Increment randomly within maxDelta range
            double delta = (2.0 * mt.genrand_real1() - 1) * maxDelta;
            solutions[i][j] = checkBounds(center[j] + delta);
        }
    }

    
    return solutions;
}


std::vector<int> SolutionBuilder::getSubset(
    int populationSize,
    int subsetSize,
    int source
) {
    std::vector<int> indices(populationSize - 1);
    int idx = 0;

    // Prepare vector of all valid indices except source
    for (int i = 0; i < populationSize; ++i) {
        if (i != source) {
            indices[idx] = i;
            ++idx;
        }
    }

    // Partial Fisher-Yates shuffle to select subsetSize random indices
    for (int i = 0; i < subsetSize; ++i) {
        int j = i + (mt.genrand_int32() % (indices.size() - i));
        std::swap(indices[i], indices[j]);
    }

    // Return only the first 'subsetSize' indices
    indices.resize(subsetSize);
    return indices;
}



std::vector<double> SolutionBuilder::getDifference(
    const std::vector<double> vec1,
    const std::vector<double> vec2
) {
    std::vector<double> diff(vec1.size());

    // Calculate element-wise difference
    for(int i = 0; i < vec1.size(); i++)
        diff[i] = vec1[i] - vec2[i];

    return diff;
}


std::vector<double> SolutionBuilder::getInitialVelocity() {
    std::vector<double> velocity(dimensions);

    double range = upper - lower;
    double vmax = 0.5 * range;

    for(int i = 0; i < dimensions; i++) {
        velocity[i] = (2.0 * mt.genrand_real1() - 1.0) * vmax;
    }

    return velocity;
}

double SolutionBuilder::clampVelocity(double v) {
    double vmax = 0.5 * (upper - lower);
    return std::max(-vmax, std::min(vmax, v));
}

