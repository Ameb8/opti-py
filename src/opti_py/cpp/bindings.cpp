#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "Optimizer/DifferentialEvolution.h"
#include "Optimizer/ParticleSwarm.h"
#include "ExperimentConfig.h"

namespace py = pybind11;


PYBIND11_MODULE(_opti_py, m) {

    py::class_<ExperimentConfig>(m, "ExperimentConfig")
        .def(py::init<
                int,
                int,
                double,
                double,
                int,
                int
            >(),
            py::arg("problem_type"),
            py::arg("dimensions"),
            py::arg("lower"),
            py::arg("upper"),
            py::arg("seed") = 42,
            py::arg("max_iterations") = 100
        )
        .def_readwrite("problem_type", &ExperimentConfig::problemType)
        .def_readwrite("dimensions", &ExperimentConfig::dimensions)
        .def_readwrite("lower", &ExperimentConfig::lower)
        .def_readwrite("upper", &ExperimentConfig::upper)
        .def_readwrite("seed", &ExperimentConfig::seed)
        .def_readwrite("max_iterations", &ExperimentConfig::maxIterations);

        /*
                const ExperimentConfig& config,
        double scale,
        double crossoverRate,
        int popSize,
        const std::string& mutationType,
        const std::string& crossoverType)
        */



    py::class_<DifferentialEvolution>(m, "DifferentialEvolution")
        .def(py::init<
                const ExperimentConfig&,
                double,
                double,
                int,
                std::string,
                std::string
            >(),
            py::arg("config"),
            py::arg("scale") = 0.9,
            py::arg("crossover_rate") = 0.6,
            py::arg("pop_size") = 200,
            py::arg("mutation") = "rand1",
            py::arg("crossover") = "bin"
        )

        .def("optimize",
            [](DifferentialEvolution &de) {
                std::vector<double> result;

                // Release the GIL while running C++ code
                {
                    py::gil_scoped_release release;
                    result = de.optimize();
                }

                // GIL automatically reacquired
                return py::array_t<double>(result.size(), result.data());
            }
        )
        .def("get_best_fitness",
            &DifferentialEvolution::getBestFitness
        )
        .def("get_fitnesses",
            &DifferentialEvolution::getBestFitnesses
        )
        .def("get_best_solution",
            &DifferentialEvolution::getBestSolution
        );
    

    py::class_<ParticleSwarm>(m, "ParticleSwarm")
        .def(py::init<
            const ExperimentConfig&,
            double,
            double,
            int
        >(),
        py::arg("config"),
        py::arg("c1") = 0.8,
        py::arg("c2") = 1.2,
        py::arg("pop_size") = 200
        )
        .def("optimize",
            [](ParticleSwarm &pso) {
                std::vector<double> result;
                {
                    py::gil_scoped_release release;
                    result = pso.optimize();
                }
                return py::array_t<double>(result.size(), result.data());
            }
        )
        .def("get_best_fitness", &ParticleSwarm::getBestFitness)
        .def("get_fitnesses", &ParticleSwarm::getBestFitnesses)
        .def("get_best_solution", &ParticleSwarm::getBestSolution);
}
