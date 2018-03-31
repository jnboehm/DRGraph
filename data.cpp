#ifndef DATA
#define DATA

#include "data.h"
#include "genrandom.h"
#include <map>
#include <iostream>
#include <fstream>
#include <malloc.h>
#include <sstream>
#include <cfloat>

data::data() {}

data::~data() {
    vector<int>().swap(edge_from);
    vector<int>().swap(edge_to);
    vector<float>().swap(edge_weight);
    vector<int>().swap(reverse);
}

data::data(const data &d) {
    n_vertices = d.n_vertices;
    n_edge = d.n_edge;
    n_threads = d.n_threads;
    perplexity = d.perplexity;
    disMAX = d.disMAX;
    edge_from.assign(d.edge_from.begin(), d.edge_from.end());
    edge_to.assign(d.edge_to.begin(), d.edge_to.end());
    reverse.assign(d.reverse.begin(), d.reverse.end());
    edge_weight.assign(d.edge_weight.begin(), d.edge_weight.end());
}

data &data::operator=(const data &d) {
    clean();

    n_vertices = d.n_vertices;
    n_edge = d.n_edge;
    n_threads = d.n_threads;
    perplexity = d.perplexity;
    disMAX = d.disMAX;
    edge_from.assign(d.edge_from.begin(), d.edge_from.end());
    edge_to.assign(d.edge_to.begin(), d.edge_to.end());
    reverse.assign(d.reverse.begin(), d.reverse.end());
    edge_weight.assign(d.edge_weight.begin(), d.edge_weight.end());
    return *this;
}

void data::clean() {
    vector<int>().swap(edge_from);
    vector<int>().swap(edge_to);
    vector<float>().swap(edge_weight);
    vector<int>().swap(reverse);
}

void data::clean_graph()
{
    n_edge = 0;
    next.clear(); vector<int>().swap(next);
    edge_from.clear(); vector<int>().swap(edge_from);
    edge_to.clear(); vector<int>().swap(edge_to);
    reverse.clear(); vector<int>().swap(reverse);
    edge_weight.clear(); vector<float >().swap(edge_weight);
//    names.clear();
}



void data::load_from_graph(char *infile)
{
    clean_graph();
    int edgenum, x, y;
    float weight;
    ifstream fin(infile);
    string line;
    if (fin) {
        cout << "Reading graph edges from " << infile << "......" << endl;
        getline(fin, line);
        istringstream inputstr(line);

        inputstr >> n_vertices >> edgenum;
        reserver_data(n_vertices);
        int i ;
        for ( i = 0; i < edgenum; i++) {
            getline(fin, line);
            istringstream inputline(line);
            inputline >> x >> y >> weight;
            add_edge(x, y, weight);

            if (n_edge % 5000 == 0) {
                cout << "\r" << n_edge / 1000 << "K" << flush;
            }
        }
        cout << endl;
    } else {
        cout << "File not found!\n" << endl;
        exit(1);
    }
    fin.close();
}

float* data::load_vec(char *infile, int &vertices, int &out_dim)
{
    float *res;
    string line;
    ifstream fin(infile);
    if (fin) {
        cout << "Read vec from " << infile << endl;
        getline(fin, line);
        istringstream inputstr(line);
        inputstr >> vertices >> out_dim;
        res = new float[vertices * out_dim];
        int i = 0;
        while (getline(fin, line) && i < vertices) {
            istringstream inputline(line);
            float a ;
            inputline >> a ;
            for (int j = 0; j < out_dim; j++) {
                inputline >> res[i * out_dim + j];
            }
            i++;
        }
    } else {
        cout << "File not found!\n" << endl;
        exit(1);
    }
    fin.close();
    return res;
}

void data::reserver_data(int n) {
    edge_to.reserve(n);
    edge_from.reserve(n);
    edge_weight.reserve(n);
    next.reserve(n);
    reverse.reserve(n);
}

void data::prepare_data() {
    int x = -1;
    head = new int[n_vertices];
    for (int i = 0; i < n_vertices; ++i) head[i] = -1;
    for (int p = 0; p < n_edge; ++p)
    {
        x = edge_from[p];
        next[p] = head[x];
        head[x] = p;
    }
    printf("Total vertices : %d\tTotal edges : %d\n", n_vertices, n_edge);

}

void data::add_edge(int v1, int v2, float weight) {

    edge_from.push_back(v1);
    edge_to.push_back(v2);
    edge_weight.push_back(weight);
    reverse.push_back(++n_edge);

    edge_from.push_back(v2);
    edge_to.push_back(v1);
    edge_weight.push_back(weight);
    reverse.push_back(n_edge - 1);
    ++n_edge;

    next.push_back(-1);
    next.push_back(-1);
}

void data::compute_similarity(int n_thre, float perp, int dismax)
{
    n_threads = n_thre < 0 ? 8 : n_thre;
    perplexity = perp < 0 ? 50.0 : perp;
    disMAX = dismax < 0  ? 1 : dismax;
    if (disMAX > n_vertices) disMAX = n_vertices;

    printf("Computing similarities ......"); fflush(stdout);

    pthread_t *pt = new pthread_t[n_threads];
    for (int j = 0; j < n_threads; ++j) pthread_create(&pt[j], NULL, data::compute_similarity_thread_caller, new arg_struct(this, j));
    for (int j = 0; j < n_threads; ++j) pthread_join(pt[j], NULL);
    delete[] pt;

    int x, y, p, q;

    for (x = 0; x < n_vertices; ++x)
    {
        for (p = head[x]; p >= 0; p = next[p])
        {
            y = edge_to[p];
            q = reverse[p];
            if (q == -1)
            {
                edge_from.push_back((int)y);
                edge_to.push_back((int)x);
                edge_weight.push_back(0);
                next.push_back(head[y]);
                reverse.push_back(p);
                q = reverse[p] = head[y] = n_edge++;
            }
            if (x > y){
                edge_weight[p] = edge_weight[q] = (edge_weight[p] + edge_weight[q]) / 2;
            }
        }
    }
    printf(" Done.\n");
}

void *data::compute_similarity_thread_caller(void *arg)
{
    data *ptr = (data*)(((arg_struct*)arg)->ptr);
    ptr->compute_similarity_thread(((arg_struct*)arg)->id);
    pthread_exit(NULL);
}

void data::compute_similarity_thread(int id)
{
    int lo = id * n_vertices / n_threads;
    int hi = (id + 1) * n_vertices / n_threads;
    int x, iter, p;
    float exparr[disMAX + 1];

    float beta, lo_beta, hi_beta, sum_weight, H, tmp;
    for (x = lo; x < hi; ++x)
    {
        beta = 1;
        lo_beta = hi_beta = -1;
        for (iter = 0; iter < 200; ++iter)
        {
            H = 0;
            sum_weight = FLT_MIN;
            for (int i = 0; i <= disMAX + 1; i++) {
                exparr[i] = exp(-beta * i);
            }
            for (p = head[x]; p >= 0; p = next[p])
            {
                sum_weight += tmp = exparr[(int)edge_weight[p]];
                H += beta * (edge_weight[p] * tmp);
            }
            H = (H / sum_weight) + log(sum_weight);
            if (fabs(H - log(perplexity)) < 1e-5) {
                break;
            }
            if (H > log(perplexity))
            {
                lo_beta = beta;
                if (hi_beta < 0) beta *= 2; else beta = (beta + hi_beta) / 2;
            }
            else{
                hi_beta = beta;
                if (lo_beta < 0) beta /= 2; else beta = (lo_beta + beta) / 2;
            }
            if(beta > FLT_MAX) beta = FLT_MAX;
        }

        for (int i = 0; i <= disMAX + 1; i++) {
            exparr[i] = exp(-beta * i);
        }
        for (p = head[x], sum_weight = FLT_MIN; p >= 0; p = next[p])
        {
            sum_weight += edge_weight[p] = exparr[(int)edge_weight[p]];
        }
        for (p = head[x]; p >= 0; p = next[p])
        {
            edge_weight[p] /= sum_weight;
        }
    }
}

void *data::search_reverse_thread_caller(void *arg)
{
    data *ptr = (data*)(((arg_struct*)arg)->ptr);
    ptr->search_reverse_thread(((arg_struct*)arg)->id);
    pthread_exit(NULL);
}

void data::search_reverse_thread(int id)
{
    int lo = id * n_vertices / n_threads;
    int hi = (id + 1) * n_vertices / n_threads;
    int x, y, p, q;
    for (x = lo; x < hi; ++x)
    {
        for (p = head[x]; p >= 0; p = next[p])
        {
            y = edge_to[p];
            for (q = head[y]; q >= 0; q = next[q])
            {
                if (edge_to[q] == x) break;
            }
            reverse[p] = q;
        }
    }
}

#endif
