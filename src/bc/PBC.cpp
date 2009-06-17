// this macro removes all LOG statements with level < WARN at source level
#define LOG4ESPP_LEVEL_INFO

#include "PBC.hpp"
#include <cmath>
#include <python.hpp>
#include <iostream>

using namespace espresso;
using namespace espresso::bc;

/* ---------------------------------------------------------------------- */

LOG4ESPP_LOGGER(PBC::theLogger, "bc.PBC");

/* ---------------------------------------------------------------------- */

PBC::PBC() {}

PBC::PBC(Real3D _length) {
  LOG4ESPP_DEBUG(theLogger, "constructor, length = " << length);
  set(_length);
}

PBC::~PBC() {}

void PBC::set(Real3D _length) {
  length = _length;
  lengthInverse[0] = 1.0 / length[0];
  lengthInverse[1] = 1.0 / length[1];
  lengthInverse[2] = 1.0 / length[2];
  LOG4ESPP_INFO(theLogger, "set length = " << length);
}

Real3D PBC::getLength(void) const { return length; }
  
void PBC::foldThis(Real3D &pos) const {
  pos[0] -= floor(pos[0] * lengthInverse[0]) * length[0];
  pos[1] -= floor(pos[1] * lengthInverse[1]) * length[1];
  pos[2] -= floor(pos[2] * lengthInverse[2]) * length[2];
}

Real3D PBC::fold(const Real3D& pos) const {
  Real3D res(pos);
  foldThis(res);
  return res;
}

Real3D PBC::getDist(const Real3D& pos1, const Real3D& pos2) const {

  // res = pos2 - pos1
  Real3D res(pos1);
  res -= pos2;

  // compute distance
  res[0] = remainder(res[0], length[0]);
  res[1] = remainder(res[1], length[1]);
  res[2] = remainder(res[2], length[2]);

  return res;
}

Real3D PBC::randomPos(void) {
  // TODO: RNG!
  Real3D res(length);
  res[0] *= drand48();
  res[1] *= drand48();
  res[2] *= drand48();
  return res;
}


//////////////////////////////////////////////////
// REGISTRATION WITH PYTHON
//////////////////////////////////////////////////
void
PBC::registerPython() {
  using namespace boost::python;

  class_<PBC, boost::shared_ptr<PBC>, bases<BC> >("bc_PBC", init<>())
    .def("set", &PBC::set)
    .def("getLength", &PBC::getLength)
    .def("randomPos", &PBC::randomPos)
    .def("getDist", &PBC::getDist)
    .def("fold", &PBC::fold)
    .def("foldThis", &PBC::foldThis)
    ;
}
