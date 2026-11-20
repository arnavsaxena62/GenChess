#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "bot.h"

namespace py = pybind11;

PYBIND11_MODULE(chessbots, m) {
    m.doc() = "C++ chess bot bindings";

    m.def(
        "batch_matches",
        &batchMatches,
        py::arg("bots"),
        "Run pairwise matches between all supplied bots."
    );

    m.def(
        "batch_matches_defined",
        &batchMatchesDefined,
        py::arg("bots"),
        py::arg("matches"),
        "run matches between supplied indexes"
    );

    m.def(
        "won",
        &won,
        py::arg("bot1"),
        py::arg("bot2"),
        "Play one match: 0 = bot1 wins, 1 = bot2 wins, 2 = draw."
    );
}