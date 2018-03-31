#include <queue>
#include "graph.h"
#include <tr1/unordered_map>
using namespace std::tr1;

graph::graph(int n) {
    N = n;
    adjList = new unordered_set<int>[N];
}

graph::graph(const graph &g) {
    N = g.N;
    disMAX = g.disMAX;
    adjList = new unordered_set<int>[N];
    for (int i = 0; i < N; i++) {
        adjList->insert(g.adjList->begin(), g.adjList->end());
    }
}

graph &graph::operator=(graph &g) {
    if (adjList != NULL) {
        for (int i = 0; i < N; i++) {
            unordered_set<int>().swap(adjList[i]);
        }
        delete[] adjList; adjList = NULL;
    }
    N = g.N;
    disMAX = g.disMAX;
    adjList = new unordered_set<int>[N];
    for (int i = 0; i < N; i++) {
        adjList->insert(g.adjList->begin(), g.adjList->end());
    }
    return *this;
}

graph::~graph() {
    if (adjList != NULL) {
        for (int i = 0; i < N; i++) {
            unordered_set<int>().swap(adjList[i]);
        }
        delete[] adjList; adjList = NULL;
    }
}

void graph::shortest_path_length(data *gdata, int dismax) {

    if (dismax == 1) return;
    disMAX = dismax < 0 ? 1 : dismax;

    for (int i = 0; i < gdata->n_edge; i++) {
        addEdge(gdata->edge_from[i], gdata->edge_to[i]);
    }

    gdata->clean_graph();

    vector<int> *distances = new vector<int>[N];
    vector<int> *adjtarget = new vector<int>[N];

#pragma omp parallel for
    for (int i = 0; i < gdata->n_vertices; i++ ) {

        bfs(i, &distances[i], &adjtarget[i]);

    }
    for (int i = 0; i < gdata->n_vertices; i++) {
        for (int j = 0; j < adjtarget[i].size(); j++) {
            if (adjtarget[i][j] < i) continue;
            gdata->add_edge(i, adjtarget[i][j], distances[i][j]);
        }
    }

    for (int i = 0; i < N; i++) {
        unordered_set<int>().swap(adjList[i]);
        vector<int>().swap(distances[i]);
        vector<int>().swap(adjtarget[i]);
    }
    delete[] adjList; adjList = NULL;
    delete[] distances; distances = NULL;
    delete[] adjtarget; adjtarget = NULL;
}

void graph::bfs(int s, vector<int> *distance, vector<int> *adjtarget) {

    unordered_map<int, int> dist;

    dist[s] = 0;
    queue<int> Q;
    Q.push(s);
    int level = 0;

    while( !Q.empty() ){
        int u = Q.front();
        Q.pop();
        if (dist[u] + 1 > disMAX) break;

        for(const auto & v : adjList[u]){
            unordered_map<int, int>::const_iterator got = dist.find(v);
            if( got == dist.end() ){
                Q.push(v);
                dist[v] = dist[u] + 1;
                level = dist[v];

                (*distance).push_back(level);
                (*adjtarget).push_back(v);
            }
        }
    }
}

void graph::addEdge(int u, int v) {
    adjList[u].insert(v);
    adjList[v].insert(u);
}

