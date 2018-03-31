#ifndef DATA_H
#define DATA_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <string>

#define INF 2 << 22

using namespace std;

struct arg_struct{
    void *ptr;
    int id;
    int times;
    arg_struct(void *x, int y, int z) :ptr(x), id(y), times(z){};
    arg_struct(void *x, int y) :ptr(x), id(y){};
};


class data{
private:

public:
    int n_vertices, n_edge = 0, *head, n_threads;
    float perplexity;
    std::vector<string> names;
    std::vector<int> next, reverse;
    std::vector<int> edge_from, edge_to;
    std::vector<float> edge_weight;

    int disMAX;

    data();

    ~data();

    data(const data &);

    data &operator= (const data & d);

    void clean_graph();

    void load_from_graph(char *infile);

    float* load_vec(char *vecfile, int &n_vertices, int &dim);

    void prepare_data();

    void add_edge(int v1, int v2, float weight);

    void compute_similarity(int n_thre, float perp, int dismax);

    static void *compute_similarity_thread_caller(void *arg);

    void compute_similarity_thread(int id);

    static void *search_reverse_thread_caller(void *arg);

    void search_reverse_thread(int id);

    void reserver_data(int n);

    void clean();

};

#endif
