#ifndef GRAPHLAYOUT_EVALUATION_H
#define GRAPHLAYOUT_EVALUATION_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <list>
#include <pthread.h>
#include <iomanip>
#include <set>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include "data.h"
#include "graph.h"

using namespace std;

typedef float real;

struct arg_evaluation{
    void *ptr;
    int id;
    arg_evaluation(void *x, int y) :ptr(x), id(y){};
};

class evaluation{
private:
    int N = 0;

    float *vis;
    float *sum;
    float average = 0;

    int n_vertices, out_dim;
    int n_threads = 8;

    real CalcDist2D(int x, int y);


public:

    data *ned =  new data();

    evaluation(data *gdata);

    float jaccard();
    vector<int> vectors_intersection(vector<int> v1,vector<int> v2);
    vector<int> vectors_union(vector<int> v1,vector<int> v2);

    void load_data(char *visfile);
    void accuracy_thread(int id);
    static void *accuracy_thread_caller(void* arg);
};

class Dis{
public:
    int index;
    float distance;
    Dis(int index, float dis){
        this->index = index;
        this->distance = dis;
    }
};


#endif //GRAPHLAYOUT_EVALUATION_H
