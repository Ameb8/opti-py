#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Optimizer/DifferentialEvolution.h"

namespace py = pybind11;

PYBIND11_MODULE(_opti_py, m) {

    py::class_<DifferentialEvolution>(m, "DifferentialEvolution")

        .def(py::init<
                int,
                int,
                int,
                int,
                double,
                double,
                std::string,
                std::string
            >(),
            py::arg("problem_id"),
            py::arg("dimension"),
            py::arg("max_iterations"),
            py::arg("population_size"),
            py::arg("scale"),
            py::arg("crossover_rate"),
            py::arg("mutation") = "rand1",
            py::arg("crossover") = "bin"
        )

        .def("optimize",
             &DifferentialEvolution::optimize,
             py::call_guard<py::gil_scoped_release>())

        .def("get_best_fitness",
             &DifferentialEvolution::getBestFitness)

        .def("get_best_solution",
             &DifferentialEvolution::getBestSolution);
}
