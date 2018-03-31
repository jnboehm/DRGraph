#include <fstream>
#include "multilevel.h"
#include "genrandom.h"

Multilevel::Multilevel(int vertices, int num_cluster) {
    n_vertices = vertices;
    mem_v = curV = num_cluster;

    mem = new int[curV];
    v_index = new int[curV];
    v_pool = new int[curV];

    for (int i = 0; i < curV; i++) {
        v_index[i] = v_pool[i] = i;
        mem[i] = -1;

    }
}

Multilevel::Multilevel(const Multilevel &ml) {
    n_vertices = ml.n_vertices;
    mem_v = ml.mem_v;
    dismax = ml.dismax;
    num_cluster = ml.num_cluster;
    mem = new int[mem_v];
    for (int i = 0; i < mem_v; i++) {
        mem[i] = ml.mem[i];
    }
    g = ml.g;
}

Multilevel &Multilevel::operator=(const Multilevel &ml) {

    if (mem != nullptr) {
        delete[] mem; mem = nullptr;
    }
    n_vertices = ml.n_vertices;
    mem_v = ml.mem_v;
    dismax = ml.dismax;
    num_cluster = ml.num_cluster;
    mem = new int[mem_v];
    for (int i = 0; i < mem_v; i++) {
        mem[i] = ml.mem[i];
    }
    delete g;
    g = ml.g;
    return *this;
}

Multilevel::~Multilevel() {
    delete[] mem;
    mem = NULL;
}

vector<Multilevel> Multilevel::gen_multilevel(data *gd, int min_clusters) {
    vector<Multilevel> results;

    auto *temp = new Multilevel(gd->n_vertices, gd->n_vertices);
    temp->multilevel(gd);
    results.push_back(*temp);
    int last_level = 0;
    int last_clusters = temp->num_cluster;

    while (last_clusters > min_clusters) {
        temp = new Multilevel(gd->n_vertices, last_clusters);
        temp->multilevel(results[last_level]);

        if ((temp->num_cluster * 1.0 / last_clusters) > 0.8) break;
        results.push_back(*temp);
        last_level++;
        last_clusters = temp->num_cluster;
    }
    return results;
}

void Multilevel::deleteNode (int i) {
    curV--;
    int temp = v_pool[i];
    v_pool[i] = v_pool[curV];
    v_pool[curV] = temp;

    v_index[temp] = curV;
    v_index[v_pool[i]] = i;
}

void  Multilevel::multilevel(data *gd) {
    genrandom r;
    r.init_gsl();
    int i, u, v;
    while (curV > 0) {
        i = (int)floor(r.gslRandom() * curV);
        u = v_pool[i];
        deleteNode(i);

        mem[u] = num_cluster;
        for (int p = gd->head[u]; p >= 0; p = gd->next[p]) {
            if (gd->edge_weight[p] <= dismax) {
                v = gd->edge_to[p];
                if (mem[v] != -1) continue;

                mem[v] = num_cluster;
                deleteNode(v_index[v]);
            }
        }
        num_cluster++;
    }

    clear();

    g = new graph(num_cluster);
    for (i = 0; i < gd->n_edge; i++) {
        g->addEdge(mem[gd->edge_from[i]], mem[gd->edge_to[i]]);
    }

}

void  Multilevel::multilevel(Multilevel ml) {
    genrandom r;
    r.init_gsl();
    int i, u;
    while (curV > 0) {
        i = (int)floor(r.gslRandom() * curV);
        u = v_pool[i];
        deleteNode(i);

        mem[u] = num_cluster;
        for (const auto& v : ml.g->adjList[u]) {
            if (mem[v] != -1) continue;
            mem[v] = num_cluster;
            deleteNode(v_index[v]);
        }
        num_cluster++;
    }

    clear();

    g = new graph(num_cluster);
    for (i = 0; i < ml.num_cluster; i++) {
        for (const auto& t : ml.g->adjList[i]) {
            if (t < i) continue;
            g->addEdge(mem[i], mem[t]);
        }
    }
    ml.clear_graph();
}

void Multilevel::init (vector<Multilevel> mls, int level) {
    offset = new float*[num_cluster];
    for (int j = 0; j < num_cluster; j++) offset[j] = new float[2]();

    membership = new int[n_vertices];
    for (int i = 0; i < n_vertices; i++) {
        int gid = i;
        for (int j = 0; j <= level; j++) {
            gid = mls[j].mem[gid];
        }
        membership[i] = gid;
    }
}

void Multilevel::output(float *vis) {
    for (int j = 0; j < n_vertices; j++) {
        for (int k = 0; k < 2; k++) {
            vis[j * 2 + k] += offset[membership[j]][k];
        }
    }
    delete[] offset; offset = nullptr;
    delete[] membership; membership = nullptr;
}


void Multilevel::clear() {
    delete[] v_index; v_index = nullptr;
    delete[] v_pool; v_pool = nullptr;
}

void Multilevel::clear_graph () {
    delete g;
}

