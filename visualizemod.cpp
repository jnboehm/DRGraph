#include <iostream>
#include "visualizemod.h"

visualizemod::visualizemod() = default;

visualizemod::~visualizemod() {
    delete[] neg_table; neg_table = NULL;
    delete[] alias; alias = NULL;
    delete[] prob; prob = NULL;
    delete[] vis; vis = NULL;
}

visualizemod::visualizemod(const visualizemod &v) {
    graphdata = v.graphdata;
    out_dim = v.out_dim;
    n_threads = v.n_threads;
    n_negatives = v.n_negatives;
    n_samples = v.n_samples;
    per_samples = v.per_samples;
    edge_count_actual = v.edge_count_actual;
    initial_alpha = v.initial_alpha;
    gamma = v.gamma;
    neg_size = v.neg_size;
    neg_table = new int[neg_size];
    for (int i = 0; i < neg_size; i++) {
        neg_table[i] = v.neg_table[i];
    }
    alias = new int[graphdata->n_edge];
    prob = new float[graphdata->n_edge];
    for (int i = 0; i < graphdata->n_edge; i++) {
        alias[i] = v.alias[i];
        prob[i] = v.prob[i];
    }
}

visualizemod &visualizemod::operator=(visualizemod &v) {
    clean();
    graphdata = v.graphdata;
    out_dim = v.out_dim;
    n_threads = v.n_threads;
    n_negatives = v.n_negatives;
    n_samples = v.n_samples;
    per_samples = v.per_samples;
    edge_count_actual = v.edge_count_actual;
    initial_alpha = v.initial_alpha;
    gamma = v.gamma;
    neg_size = v.neg_size;
    neg_table = new int[neg_size];
    for (int i = 0; i < neg_size; i++) {
        neg_table[i] = v.neg_table[i];
    }
    alias = new int[graphdata->n_edge];
    prob = new float[graphdata->n_edge];
    for (int i = 0; i < graphdata->n_edge; i++) {
        alias[i] = v.alias[i];
        prob[i] = v.prob[i];
    }
    return *this;
}


void visualizemod::clean(){
    delete[] neg_table; neg_table = NULL;
    delete[] alias; alias = NULL;
    delete[] prob; prob = NULL;
    delete[] vis; vis = NULL;
}

const gsl_rng_type *visualizemod::gsl_T = nullptr;
gsl_rng *visualizemod::gsl_r = nullptr;


void visualizemod::run(data *data1, int out_d, int n_thre, int n_samp , float alph, int n_nega,
                       float gamm, vector<Multilevel> mls)
{
    gsl_rng_env_setup();
    gsl_T = gsl_rng_rand48;
    gsl_r = gsl_rng_alloc(gsl_T);
    gsl_rng_set(gsl_r, 314159265);

    graphdata = data1;
    if (!graphdata->head)
    {
        printf("Missing training data!\n");
        return;
    }
    out_dim = out_d < 0 ? 2 : out_d;
    initial_alpha = alph < 0 ? 1 : alph;
    n_threads = n_thre < 0 ? 8 : n_thre;
    int samp_ratio = n_samp < 0 ? 500 : n_samp;
    n_negatives = n_nega < 0 ? 10 : n_nega;
    float t_gamma = gamm < 0 ? 1 : gamm;

    init_neg_table();
    init_alias_table();

    float _gamma = 0.01;
    for (int i = mls.size() - 1; i >= 0; i--) {
        if (mls[i].num_cluster < 1000)
            n_samples = mls[i].num_cluster * 1000;
        else n_samples = mls[i].num_cluster * 100;

        mls[i].init(mls, i);
        if (i == mls.size() - 1) {
            random_vis(&mls[i]);
        }
        visualize(&mls[i], n_samples, 1, _gamma, true);
        if (i < mls.size() - 1) _gamma = t_gamma;
    }
    visualize(nullptr, (long long)graphdata->n_vertices * samp_ratio, 1, _gamma);
    printf("\n");
}


void visualizemod::random_vis(Multilevel *ml) {
    int i;
    vis = new float[graphdata->n_vertices * out_dim];
    float temp[ml->num_cluster * out_dim];
    for (i = 0; i< ml->num_cluster * out_dim; i++) temp[i] = (float)((gsl_rng_uniform(gsl_r) - 0.5) / out_dim * 0.0001);
    for (i = 0; i < graphdata->n_vertices; ++i) {
        for (int j = 0; j < out_dim; j++) {
            vis[i*out_dim + j] = temp[ml->membership[i] * out_dim + j];
        }
    }
}

void visualizemod::visualize(Multilevel *m, long long _samples, float _alpha, float _gamma, bool method) {
    ml = m;

    n_samples = _samples;
    gamma = _gamma;
    ml_method = method;

    if (method && ml == NULL){
        cout << "no multilevel layout" << endl;
        exit(1);
    }

    edge_count_actual = 0;

    int n_iter = 1;
    pthread_t *pt;
    per_samples = n_samples / n_iter;

    for (int iter = 0; iter < n_iter; iter++) {
        pt = new pthread_t[n_threads];
        edge_count_actual = iter * per_samples;
        for (int j = 0; j < n_threads; ++j) pthread_create(&pt[j], nullptr, visualizemod::visualize_thread_caller, new arg_struct(this, j, iter));
        for (int j = 0; j < n_threads; ++j) pthread_join(pt[j], nullptr);
        delete[] pt;
    }
    if (ml_method) ml->output(vis);
}

void *visualizemod::visualize_thread_caller(void *arg)
{
    visualizemod *ptr = (visualizemod*)(((arg_struct*)arg)->ptr);
    ptr->visualize_thread(((arg_struct*)arg)->id, ((arg_struct*)arg)->times);
    pthread_exit(NULL);
}

void visualizemod::visualize_thread(int id, int iter)
{
    long long edge_count = 0, last_edge_count = 0;
    int x, y, p, lx, ly, i, j;
    float f, g, gg, cur_alpha = initial_alpha;
    float *cur = new float[out_dim];
    float *err = new float[out_dim];
    float *samp = new float[out_dim];
    float grad_clip = 1.0;
    while (1)
    {
        if (edge_count > per_samples / n_threads + 2) break;
        if (edge_count - last_edge_count > 100)
        {
            edge_count_actual += edge_count - last_edge_count;
            last_edge_count = edge_count;
            cur_alpha = initial_alpha * (1 - edge_count_actual / (n_samples + 1.0));
            if (cur_alpha < initial_alpha * 0.0001) cur_alpha = initial_alpha * 0.0001;
            if ((edge_count_actual % (n_samples / 100)) < (n_samples / 1000))
            {
                printf("%cFitting model\tAlpha: %f Progress: %.3lf%%", 13, cur_alpha, (float)edge_count_actual / (float)(n_samples + 1) * 100);
                fflush(stdout);
            }
        }
        p = sample_an_edge(gsl_rng_uniform(gsl_r), gsl_rng_uniform(gsl_r));
        x = graphdata->edge_from[p];
        y = graphdata->edge_to[p];
        lx = x * out_dim;
        
        int mx, my;
        if (ml_method) {
            mx = ml->membership[x];
            for (i = 0; i < out_dim; ++i) {
                cur[i] = vis[lx + i] + ml->offset[mx][i];
                err[i] = 0;
            }
            for (i = 0; i < n_negatives + 1; ++i)
            {
                if (i > 0){
                    y = neg_table[(unsigned int)floor(gsl_rng_uniform(gsl_r) * (neg_size - 0.1))];
                    if (ml->membership[y] == ml->membership[graphdata->edge_to[p]]) continue;
                    if (ml->membership[y] == ml->membership[x]) continue;
                }
                ly = y * out_dim;
                my = ml->membership[y];
                for (j = 0, f= 0; j < out_dim; ++j) {
                    samp[j] = vis[ly + j] + ml->offset[my][j];
                    f += (cur[j] - samp[j]) * (cur[j] - samp[j]);
                }
                if (i == 0) g = -2 / (1 + f);
                else g = 2 * gamma / (1 + f) / (0.1 + f);
                for (j = 0; j < out_dim; ++j)
                {
                    gg = g * (cur[j] - samp[j]);
                    if (gg > grad_clip) gg = grad_clip;
                    if (gg < -grad_clip) gg = -grad_clip;
                    err[j] += gg * cur_alpha;
                    ml->offset[my][j] += (-gg) * cur_alpha;
                }
            }
            for (int j = 0; j < out_dim; ++j) ml->offset[mx][j] += err[j];
        } else {
            for (i = 0; i < out_dim; ++i) cur[i] = vis[lx + i], err[i] = 0;
            for (i = 0; i < n_negatives + 1; ++i)
            {
                if (i > 0)
                {
                    y = neg_table[(unsigned int)floor(gsl_rng_uniform(gsl_r) * (neg_size - 0.1))];
                    if (y == graphdata->edge_to[p]) continue;
                }
                ly = y * out_dim;
                for (j = 0, f= 0; j < out_dim; ++j) f += (cur[j] - vis[ly + j]) * (cur[j] - vis[ly + j]);
                if (i == 0) g = -2 / (1 + f);
                else g = 2 * gamma / (1 + f) / (0.1 + f);
                for (j = 0; j < out_dim; ++j)
                {
                    gg = g * (cur[j] - vis[ly + j]);
                    if (gg > grad_clip) gg = grad_clip;
                    if (gg < -grad_clip) gg = -grad_clip;
                    err[j] += gg * cur_alpha;

                    gg = g * (vis[ly + j] - cur[j]);
                    if (gg > grad_clip) gg = grad_clip;
                    if (gg < -grad_clip) gg = -grad_clip;
                    vis[ly + j] += gg * cur_alpha;
                }
            }
            for (int j = 0; j < out_dim; ++j) vis[lx + j] += err[j];
        }
        ++edge_count;
    }
    delete[] cur;
    delete[] err;
}

void visualizemod::init_alias_table()
{
    alias = new int[graphdata->n_edge];
    prob = new float[graphdata->n_edge];

    auto *norm_prob = new float[graphdata->n_edge];
    auto *large_block = new int[graphdata->n_edge];
    auto *small_block = new int[graphdata->n_edge];

    float sum = 0;
    int cur_small_block, cur_large_block;
    int num_small_block = 0, num_large_block = 0;

    for (int k = 0; k < graphdata->n_edge; ++k) sum += graphdata->edge_weight[k];
    for (int k = 0; k < graphdata->n_edge; ++k) norm_prob[k] = graphdata->edge_weight[k] * graphdata->n_edge / sum;

    for (int k = graphdata->n_edge - 1; k >= 0; --k)
    {
        if (norm_prob[k] < 1)
            small_block[num_small_block++] = k;
        else
            large_block[num_large_block++] = k;
    }

    while (num_small_block && num_large_block)
    {
        cur_small_block = small_block[--num_small_block];
        cur_large_block = large_block[--num_large_block];
        prob[cur_small_block] = norm_prob[cur_small_block];
        alias[cur_small_block] = cur_large_block;
        norm_prob[cur_large_block] = norm_prob[cur_large_block] + norm_prob[cur_small_block] - 1;
        if (norm_prob[cur_large_block] < 1)
            small_block[num_small_block++] = cur_large_block;
        else
            large_block[num_large_block++] = cur_large_block;
    }

    while (num_large_block) prob[large_block[--num_large_block]] = 1;
    while (num_small_block) prob[small_block[--num_small_block]] = 1;

    delete[] norm_prob;
    delete[] small_block;
    delete[] large_block;
}

void visualizemod::init_neg_table()
{
    int x, p, i;
    long long need_size = (long long)graphdata->n_vertices * 100;
    neg_size = need_size < 1e8 ? need_size : 1e8;
//    graphdata->reverse.clear(); vector<int> (graphdata->reverse).swap(graphdata->reverse);
    float sum_weights = 0, dd, *weights = new float[graphdata->n_vertices];
    for (i = 0; i < graphdata->n_vertices; ++i) weights[i] = 0;
    for (x = 0; x < graphdata->n_vertices; ++x)
    {
        for (p = graphdata->head[x]; p >= 0; p = graphdata->next[p])
        {
            weights[x] += graphdata->edge_weight[p];
        }
        sum_weights += weights[x] = pow(weights[x], 0.75);
    }
    graphdata->next.clear(); vector<int> (graphdata->next).swap(graphdata->next);
    delete[] graphdata->head; graphdata->head = NULL;
    neg_table = new int[neg_size];
    dd = weights[0];
    for (i = x = 0; i < neg_size; ++i)
    {
        neg_table[i] = x;
        if (i / (float)neg_size > dd / sum_weights && x < graphdata->n_vertices - 1)
        {
            dd += weights[++x];
        }
    }
    delete[] weights;
}

int visualizemod::sample_an_edge(float rand_value1, float rand_value2)
{
    int k = (int)((graphdata->n_edge - 0.1) * rand_value1);
    return rand_value2 <= prob[k] ? k : alias[k];
}

void visualizemod::save(char *outfile)
{
    FILE *fout = fopen(outfile, "wb");
    fprintf(fout, "%d %d\n", graphdata->n_vertices, out_dim);
    for (int i = 0; i < graphdata->n_vertices; ++i)
    {
        fprintf(fout, "%d ", i);
        for (int j = 0; j < out_dim; ++j)
        {
            if (j>0) fprintf(fout, " ");
            fprintf(fout, "%.6f", vis[i * out_dim + j]);
        }
        fprintf(fout, "\n");
    }
    fclose(fout);
}


