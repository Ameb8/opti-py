#include "ProblemFactory.h"
#include "Problem/Schwefel.h"
#include "Problem/DeJongOne.h"
#include "Problem/Rosenbrock.h"
#include "Problem/Rastrigin.h"
#include "Problem/Griewangk.h"
#include "Problem/SineEnvelope.h"
#include "Problem/StretchedV.h"
#include "Problem/AckleyOne.h"
#include "Problem/AckleyTwo.h"
#include "Problem/EggHolder.h"

namespace {

// Type alias for a factory function returning shared_ptr<Problem>
using Creator = std::shared_ptr<Problem>(*)();

// Factory functions for each problem
std::shared_ptr<Problem> createSchwefel() {
    return std::make_shared<Schwefel>();
}

std::shared_ptr<Problem> createDeJongOne() {
    return std::make_shared<DeJongOne>();
}

std::shared_ptr<Problem> createRosenbrock() {
    return std::make_shared<Rosenbrock>();
}

std::shared_ptr<Problem> createRastrigin() {
    return std::make_shared<Rastrigin>();
}

std::shared_ptr<Problem> createGriewangk() {
    return std::make_shared<Griewangk>();
}

std::shared_ptr<Problem> createSineEnvelope() {
    return std::make_shared<SineEnvelope>();
}

std::shared_ptr<Problem> createStretchedV() {
    return std::make_shared<StretchedV>();
}

std::shared_ptr<Problem> createAckleyOne() {
    return std::make_shared<AckleyOne>();
}

std::shared_ptr<Problem> createAckleyTwo() {
    return std::make_shared<AckleyTwo>();
}

std::shared_ptr<Problem> createEggHolder() {
    return std::make_shared<EggHolder>();
}



// Array mapping integer IDs -> factory functions
constexpr std::array<Creator, 10> creators = {
    &createSchwefel,
    &createDeJongOne,
    &createRosenbrock,
    &createRastrigin,
    &createGriewangk,
    &createSineEnvelope,
    &createStretchedV,
    &createAckleyOne,
    &createAckleyTwo,
    &createEggHolder
};

} // anonymous namespace

std::shared_ptr<Problem> ProblemFactory::create(int id) {
    int index = id - 1; // Decrement index to array index

    // Get Creator function
    if (index < 0 || static_cast<std::size_t>(index) >= creators.size())
        throw std::out_of_range("Invalid problem ID");

    return creators[index](); // Call the factory function
}