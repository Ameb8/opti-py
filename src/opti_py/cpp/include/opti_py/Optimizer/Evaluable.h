// Evaluable.h
#pragma once
#include <vector>

template<typename T> 
concept Evaluable = requires(T t, std::vector<std::vector<double>>& pop, const std::vector<double>& sol) {
    { t.getInitialSolutions(pop) } -> std::same_as<void>;
    { t.getLowerBounds() } -> std::convertible_to<double>;
    { t.getUpperBounds() } -> std::convertible_to<double>;
    { t.evaluateSolution(sol) } -> std::convertible_to<double>;
};