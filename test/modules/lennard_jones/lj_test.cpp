#define RAND_MAX
#define NUMBER_OF_PARTICLES 33000
#define BOX_SIZE 200

#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <sys/times.h>

#include "Particle.hpp"
#include "LennardJonesInteraction.hpp"
#include "FullPairVIterator.hpp"

using namespace std;

#define MAX 4096

template<class Interaction>
void compute (std::vector<Particle> *pc, Interaction lj, PairVIterator* it)

{
  double rsq;
  double en;

  en = 0.0;

  //loop over all pairs assuming non-periodic BCs

  for (it->reset(); !it->done(); it->next()) {
     int jlist[MAX];
     int i;
     int jk;
     it->get(i, jk, MAX, jlist);

//     printf ("it->get, i = %d, jk = %d\n", i, jk);
     Particle* Pi = &(*pc)[i];
     for (int j1 = 0; j1 < jk; j1++) {
        // printf ("calc i = %d, j = %d\n", i, j[j1]);
        Particle* Pj = &(*pc)[jlist[j1]];
        rsq   = pow(Pi->getx() - Pj->getx(), 2);
        rsq  += pow(Pi->gety() - Pj->gety(), 2);
        rsq  += pow(Pi->getz() - Pj->getz(), 2);
        en += lj.computeLennardJonesEnergy(rsq);
     }
  }

  //write out the total LJ energy
  std::cout << "en = " << en << std::endl;

}

void iterate(int size) {

  //variables for storing random numbers
  double rx;
  double ry;
  double rz;

  //create a vector to store the particles

  std::vector<Particle> pc(size);

  //assign random positions to the particles on r[0, BOX_SIZE]
  for(int i = 0; i < pc.size(); i++) {
    rx = BOX_SIZE * double(rand()) / RAND_MAX;
    ry = BOX_SIZE * double(rand()) / RAND_MAX;
    rz = BOX_SIZE * double(rand()) / RAND_MAX;
    pc[i] = Particle(rx, ry, rz);
  }

  //print a particle to standard output as a test
  std::cout << pc[1].toString() << std::endl;

  //create a LJ interaction and set its cutoff
  LennardJonesInteraction lj = LennardJonesInteraction();
  lj.setCutoff(2.5);

  PairVIterator* it = new FullPairVIterator(pc.size());

  compute<LennardJonesInteraction>(&pc, lj, it);

}

int main() {

  struct tms start_t, stop_t;
  const unsigned clocks_per_sec = sysconf(_SC_CLK_TCK);

  // int sizes[] = { 10000, 10000, 33000};

  int sizes[] = {  10000, 10000, 33000 };
  
  int n = sizeof(sizes) / sizeof(int);

  for (int i = 0; i < n; i++) {

     int size = sizes[i];

     times(&start_t);
     iterate (size);
     times(&stop_t);
 
     cout << "LJ interaction for " << size << " particles finished" << endl;
     cout << "user time = "
          << static_cast<double>(stop_t.tms_utime - start_t.tms_utime) / clocks_per_sec
          << "s\tsystem time = "
          << static_cast<double>(stop_t.tms_stime - start_t.tms_stime) / clocks_per_sec
          << "s" << endl;
     }
}

