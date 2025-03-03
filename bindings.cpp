#include "include/helperfunctions.h"
#include "include/flexoffer.h"
#include "include/alignments.h"
#include "include/group.h"
#include <ctime>
#include <pybind11/pybind11.h>

PYBIND11_MODULE(flexoffer_logic, m) {
    m.doc() = "Flexoffer logic module"; // Optional module docstring

    m.def("clusterFoGroups", &clusterFo_Group, "A function that clusters stuff",
          pybind11::arg("groups"), pybind11::arg("est_threshold"), pybind11::arg("lst_threshold"), 
          pybind11::arg("max_group_size"));
    pybind11::class_<Flexoffer>(m, "Flexoffer")
      .def(pybind11::init<int, time_t, time_t, time_t, vector<TimeSlice>&, int>())
      .def("get_offer_id", &Flexoffer::get_offer_id)
      .def("get_est", &Flexoffer::get_est)
      .def("get_lst", &Flexoffer::get_lst)
      .def("get_et", &Flexoffer::get_et)
      .def("get_duration", &Flexoffer::get_duration)
      .def("get_profile", &Flexoffer::get_profile);

    pybind11::class_<Fo_Group>(m, "Fo_Group")
      .def(pybind11::init<int>())
      .def("addFlexoffer", &Fo_Group::addFlexOffer)
      .def("createAggregatedOffer", &Fo_Group::createAggregatedOffer);

}

