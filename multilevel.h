#ifndef GRAPHLAYOUT_MULTILEVEL_H
#define GRAPHLAYOUT_MULTILEVEL_H

#include <vector>
#include <cstdio>
#include "data.h"
#include "graph.h"

using namespace std;

class Multilevel {
private:
    int n_vertices;
    int level = 0;
    int dismax = 1;
    int mem_v;

    void clear();

public:
    int num_cluster = 0;
    int *membership = nullptr;
    int *mem = nullptr;
    float **offset = nullptr;
    int curV;
    int *v_index;
    int *v_pool;

    graph *g;

    Multilevel(int n_vertices, int num_cluster);
    Multilevel(const Multilevel &ml);
    Multilevel &operator= (const Multilevel &ml);
    ~Multilevel();

    void multilevel(data *gd);

    void deleteNode(int i);

    void output(float *vis);

    void multilevel(Multilevel ml);

    static vector<Multilevel> gen_multilevel(data *gd, int min_clusters);

    void clear_graph();

    void init(vector<Multilevel> mls, int index);

};


#endif //GRAPHLAYOUT_MULTILEVEL_H
