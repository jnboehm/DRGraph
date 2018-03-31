#ifndef VISUALIZE_H
#define VISUALIZE_H

#include "data.h"
#include <gsl/gsl_rng.h>
#include "multilevel.h"
#include <pthread.h>


class visualizemod {
private:
    data *graphdata = NULL;
    int out_dim, n_threads, n_negatives;
    long long n_samples, per_samples, edge_count_actual;
    float initial_alpha, gamma;
    static const gsl_rng_type * gsl_T;
    static gsl_rng * gsl_r;
    int *neg_table;
    int neg_size;
    int *alias;
    float *prob, *vis;

    Multilevel *ml;
    bool ml_method = false;

    bool isout = false;
    char outfolder[1000];

    void visualize_thread(int id, int iter);
    static void *visualize_thread_caller(void *arg);
    void init_alias_table();
    void init_neg_table();
    int sample_an_edge(float rand_value1, float rand_value2);

public:
    visualizemod();

    ~visualizemod();

    visualizemod(const visualizemod &v);

    visualizemod &operator=(visualizemod &v);

    void clean();
    void run(data *data1, int out_d, int n_thre, int n_samp, float alph, int n_nega,
             float gamm, vector<Multilevel> mls);

    void save(char *outfile);

    void random_vis(Multilevel *ml);

    void visualize(Multilevel *m, long long _samples, float _alpha, float _gamma, bool method = false);
};


#endif //VISUALIZE_H
