#ifndef GENRANDOM
#define GENRANDOM

#include "genrandom.h"

genrandom::genrandom() {}

gsl_rng * genrandom::init_gsl (){
    const gsl_rng_type * T;
    real u;
    gsl_rng_env_setup();
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, time (NULL) * getpid());
}

real genrandom::gslRandom() {
    real u = gsl_rng_uniform (r);
    return u;
}

#endif
