#include <python.hpp>
#include <boost/bind.hpp>

#include "thermostat/Langevin.hpp"
#include "Property.hpp"
#include "error.hpp"
#include "particles/Computer.hpp"
#include "integrator/VelocityVerlet.hpp"


using namespace espresso;
using namespace espresso::particles;
using namespace espresso::storage;
using namespace espresso::thermostat;

/* -- define the Logger for the class  ------------------------------------------- */

LOG4ESPP_LOGGER(Langevin::theLogger, "Langevin");

/***********************************************************************************
*  subclass of Langevin to handle thermostatting after StepA                       *
***********************************************************************************/

class StepThermalA: public Computer {

private:
  PropertyHandle< Real3D > vel;
  real timeStep;
  real gamma;

public:
  StepThermalA(Set::SelfPtr set,
	       Property< Real3D >::SelfPtr velProperty, 
	       real _timeStep, real _gamma)
    : vel(velProperty->getHandle(set)), 
      timeStep(_timeStep), gamma(_gamma) { }
  void prepare(Storage::SelfPtr set) {}
  // m = 1
  virtual bool apply(const ParticleHandle pref) {
    vel[pref] = vel[pref] - 0.5 * gamma * vel[pref] * timeStep;
    return true;
  }
};


/***********************************************************************************
*  subclass of Langevin to handle thermostatting after StepB                       *
***********************************************************************************/

class StepThermalB: public particles::Computer {
  
private:
  PropertyHandle< Real3D > vel;
  real timeStep;
  real gamma;
  real temperature;
  real c;

public:
  StepThermalB(Set::SelfPtr set,
	       Property< Real3D >::SelfPtr velProperty, 
	       real _timeStep, real _gamma, real _temperature)
    : vel(velProperty->getHandle(set)), 
      timeStep(_timeStep), gamma(_gamma), temperature(_temperature) {
    /*
     * The c coefficient represents the strength of the noise in the Langevin thermostat.
     * The formula is usually given as c = sqrt(2*gamma*temp/timeStep) multiplied by
     * a *normally* distributed random number N(0,1). One can approximate this by using
     * a uniformly distributed random number with same first and second moment, i.e.
     * mean 0 and interval [-sqrt(3);sqrt(3)]. This can be reproduced with a distribution
     * between [-0.5;0.5] by using a prefactor of 2*sqrt(3) in front. This now gives
     * c = sqrt(24 * gamma * temp/ timeStep).
     * We finally multiply this by an additional factor of 2 in order to weight correctly
     * the noise term over the two steps of the Velocity Verlet integrator.
     * c = sqrt(96 * gamma * temp/ timeStep).
     */
    
    c = sqrt(96.0 * gamma * temperature / timeStep);
  }
  
  void prepare(Storage::SelfPtr set) {}

  // m = 1
  virtual bool apply(ParticleHandle pref) {
    Real3D rand3(drand48() - 0.5, drand48() - 0.5, drand48() - 0.5);
    vel[pref] = vel[pref] + 0.5 * (c * rand3 - gamma * vel[pref]) * timeStep;
    return true;
  }

};

/***********************************************************************************
*  class Langevin                                                                  *
***********************************************************************************/

Langevin::Langevin(real _temperature, real _gamma)
  : Thermostat(_temperature),
    linearCongruential(15154),
    normalDist(0.,1.),
    gauss(linearCongruential, normalDist)
{
  setGamma(_gamma);         // also checks for a correct argument

  LOG4ESPP_INFO(theLogger, "Langevin, temperature = " << temperature << ", gamma = " << gamma);
}

/**********************************************************************************/

void Langevin::setGamma(real _gamma) 
{ 
  if (_gamma < 0.0) {
     ARGERROR(theLogger, "gamma = " << _gamma << " illegal, must not be negative");
  }
  gamma = _gamma; 
}

/**********************************************************************************/

real Langevin::getGamma() const { return gamma; }

/**********************************************************************************/

void Langevin::thermalizeA(const integrator::VelocityVerlet& integrator) {

  LOG4ESPP_DEBUG(theLogger, "Langevin thermalizeA at integration step = " 
                             << integrator.getIntegrationStep());

  real timeStep = integrator.getTimestep();

  Set::SelfPtr localset = takeFirst(set, integrator.getSet());

  StepThermalA stepThermalA(localset, integrator.getVelProperty(), 
			    timeStep, gamma);

  // apply stepThermalA to my particle set only if available
  localset->foreach(stepThermalA);

}

/**********************************************************************************/



void Langevin::thermalizeB(const integrator::VelocityVerlet& integrator) {

  LOG4ESPP_DEBUG(theLogger, "Langevin thermalizeB at integration step = " 
                             << integrator.getIntegrationStep());

  real timeStep = integrator.getTimestep();
  real temperature = getTemperature();

  Set::SelfPtr localset = takeFirst(set, integrator.getSet());

  StepThermalB stepThermalB(localset, integrator.getVelProperty(), 
			    timeStep, gamma, temperature);

  localset->foreach(stepThermalB);
}

/**********************************************************************************/

void Langevin::connect(integrator::VelocityVerlet::SelfPtr integrator) {
  // check that there is no existing connection

  if (!integrator) {
     ARGERROR(theLogger, "Langevin: connect to NULL integrator");
  }

  LOG4ESPP_INFO(theLogger, "connect to VelocityVerlet integrator");

  integrator->connections.add
    (integrator->updateVelocity1, 
     shared_from_this(), &Langevin::thermalizeA);

  integrator->connections.add
    (integrator->updateVelocity2, 
     shared_from_this(), &Langevin::thermalizeB);
}

/**********************************************************************************/

void Langevin::disconnect(integrator::VelocityVerlet::SelfPtr integrator) 
{
  LOG4ESPP_INFO(theLogger, "Langevin disconnects from integrator");
  integrator->connections.remove(shared_from_this());
}

/**********************************************************************************/

Langevin::~Langevin() {

  LOG4ESPP_INFO(theLogger, "~Langevin");
}

//////////////////////////////////////////////////
// REGISTRATION WITH PYTHON
//////////////////////////////////////////////////

void
Langevin::registerPython() {
  using namespace espresso::python;

  class_< Langevin, bases< Thermostat > >
    ("thermostat_Langevin", init< real, real >())
    .add_property("gamma", &Langevin::getGamma, &Langevin::setGamma)
    .def("connect", &Langevin::connect)    
    .def("disconnect", &Langevin::disconnect)    
    ;
}
