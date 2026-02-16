#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

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
          [](DifferentialEvolution &de) {
             std::vector<double> result;

             // release the GIL while running cpp code
             {
               py::gil_scoped_release release;
               result = de.optimize();
             }

          // GIL automatically reacquired
          return py::array_t<double>(result.size(), result.data());
          })   

        .def("get_best_fitness",
             &DifferentialEvolution::getBestFitness)

        .def("get_best_solution",
             &DifferentialEvolution::getBestSolution);
}
