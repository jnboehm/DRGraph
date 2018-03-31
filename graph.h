
#ifndef LARGEVIS_SHORTESTPATH_H
#define LARGEVIS_SHORTESTPATH_H

#include <iostream>
#include <vector>
#include <list>
#include "data.h"
#include <unordered_set>


#define INF 2 << 22

using namespace std;

class graph {
private:
    int N, disMAX = INF;

public:
    unordered_set<int> *adjList;

    graph(int n);
    graph(const graph &g);
    graph &operator=(graph &g);
    ~graph();

    void addEdge(int u, int v);
    void shortest_path_length (data *gdata, int dismax);
    void bfs(int source, vector<int> *distance, vector<int> *adjtarget);
};


#endif //LARGEVIS_SHORTESTPATH_H
