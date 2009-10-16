#ifndef _UNITTEST_UT_HPP
#define _UNITTEST_UT_HPP

#include <acconfig.hpp>

#ifdef PARALLEL_TEST_MODULE
#define BOOST_TEST_MODULE PARALLEL_TEST_MODULE
#endif

#include <boost/test/unit_test.hpp>

#ifdef PARALLEL_TEST_MODULE

#include "mpi.hpp"
#include "../espresso_common.hpp"

struct MPIFixture {  
  MPIFixture() { 
    initMPI(); 
  }
  ~MPIFixture() { 
    finalizeMPI(); 
  }
};

BOOST_GLOBAL_FIXTURE(MPIFixture);

#endif

#endif
