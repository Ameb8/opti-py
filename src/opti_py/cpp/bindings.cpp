#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "FlowShop/FlowShop.h"
#include "Optimizer/DifferentialEvolution.h"
#include "Optimizer/ParticleSwarm.h"
#include "ExperimentConfig.h"

namespace py = pybind11;


PYBIND11_MODULE(_opti_py, m) {

    py::class_<FlowShop>(m, "FlowShop")

        // default
        .def(py::init<>())

        // construct from dimensions
        .def(py::init<size_t, size_t>(),
            py::arg("num_jobs"),
            py::arg("num_machines"))

        // construct from numpy array
        .def(py::init([](
                py::array_t<uint64_t,
                            py::array::c_style | py::array::forcecast> arr)
        {
            auto buf = arr.request();

            if (buf.ndim != 2)
                throw std::runtime_error("jobs must be 2D");

            size_t num_jobs = buf.shape[0];
            size_t num_machines = buf.shape[1];

            std::vector<uint64_t> data(num_jobs * num_machines);

            std::memcpy(data.data(),
                        buf.ptr,
                        data.size() * sizeof(uint64_t));

            return FlowShop(num_jobs, num_machines, std::move(data));
        }),
        py::arg("jobs"))

        // zero-copy NumPy view
        .def_property("jobs",

            [](FlowShop& self) {
                return py::array_t<uint64_t>(
                    { self.num_jobs(), self.num_machines() },
                    { self.num_machines() * sizeof(uint64_t),
                    sizeof(uint64_t) },
                    self.data(),
                    py::cast(&self)
                );
            },

            [](FlowShop& self,
            py::array_t<uint64_t,
                        py::array::c_style | py::array::forcecast> arr)
            {
                auto buf = arr.request();

                if (buf.ndim != 2)
                    throw std::runtime_error("jobs must be 2D");

                size_t num_jobs = buf.shape[0];
                size_t num_machines = buf.shape[1];

                std::vector<uint64_t> new_data(num_jobs * num_machines);

                std::memcpy(new_data.data(),
                            buf.ptr,
                            new_data.size() * sizeof(uint64_t));

                self.set_jobs(num_jobs,
                            num_machines,
                            std::move(new_data));
            }
        )
        .def_property("due_dates",

            [](FlowShop& self) {
                return py::array_t<uint64_t>(
                    { self.num_jobs() },
                    { sizeof(uint64_t) },
                    self.due_dates_data(),
                    py::cast(&self)
                );
            },

            [](FlowShop& self,
            py::array_t<uint64_t,
                        py::array::c_style | py::array::forcecast> arr)
            {
                auto buf = arr.request();

                if (buf.ndim != 1)
                    throw std::runtime_error("due_dates must be 1D");

                if ((size_t)buf.shape[0] != self.num_jobs())
                    throw std::runtime_error("due_dates length must equal num_jobs");

                std::vector<uint64_t> new_data(self.num_jobs());

                std::memcpy(new_data.data(),
                            buf.ptr,
                            new_data.size() * sizeof(uint64_t));

                self.set_due_dates(std::move(new_data));
            }
        )
        .def_property_readonly("num_jobs",
            &FlowShop::num_jobs)
        .def_property_readonly("num_machines",
            &FlowShop::num_machines)

        .def("to_dict", [](const FlowShop &fs) {
            py::dict d;
            d["num_jobs"] = fs.num_jobs();
            d["num_machines"] = fs.num_machines();

            // create numpy view of job matrix
            d["jobs"] = py::array_t<uint64_t>(
                { fs.num_jobs(), fs.num_machines() },
                { fs.num_machines() * sizeof(uint64_t),
                sizeof(uint64_t) },
                fs.data(),
                py::cast(&fs)
            );

            // Create numpy view of due_dates array
            d["due_dates"] = py::array_t<uint64_t>(
                { fs.num_jobs() },
                { sizeof(uint64_t) },
                fs.due_dates_data(),
                py::cast(&fs)
            );

            return d;
        })

        .def("run_neh",
            [](FlowShop &fs, bool blocking, bool tardiness) {
                // Lambda to invoke cpp method
                FlowShopResult result;
                { // Relese GIL to enable OpenMP parallelization
                    py::gil_scoped_release release;
                    result = fs.runNEH(blocking, tardiness);
                } // Reacquire GIL
                return result;
            },

            // Assign Default paramters
            py::arg("blocking") = false,
            py::arg("tardiness") = false,
            "Run the NEH heuristic and return a FlowShopResult"
        )


        .def("run_de",
            [](
                FlowShop &fs,
                bool blocking,
                bool tardiness,
                size_t max_gens,
                size_t pop_size,
                double f,
                double cr,
                unsigned long seed
            ) {
                // Lambda to invoke cpp method
                FlowShopResult result;
                { // Release GIL to enable OpenMP parallelization
                    py::gil_scoped_release release;
                    result = fs.runDE(
                        blocking,
                        tardiness,
                        pop_size,
                        f,
                        cr,
                        max_gens,
                        seed
                    );
                } // Reacquire GIL
                return result;
            },
            
            // Assign default parameters
            py::arg("blocking") = false,
            py::arg("tardiness") = false,
            py::arg("max_gens") = 100,
            py::arg("pop_size") = 200,
            py::arg("f") = 0.5,
            py::arg("cr") = 0.9,    
            py::arg("seed") = 108664UL
        );


    py::class_<FlowShopResult>(m, "FlowShopResult")
        .def_readwrite("sequence", &FlowShopResult::sequence)
        .def_readwrite("makespan", &FlowShopResult::makespan)
        .def_readwrite("completionTimes", &FlowShopResult::completionTimes)
        .def_readwrite("tardiness", &FlowShopResult::tardiness)
        .def("to_dict", [](const FlowShopResult &r) {
            py::dict d;
            d["sequence"] = r.sequence;
            d["makespan"] = r.makespan;
            d["completionTimes"] = r.completionTimes;
            d["tardiness"] = r.tardiness;
            return d;
        });



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
