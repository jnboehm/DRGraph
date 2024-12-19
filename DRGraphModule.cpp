#include "Python.h"
#include "data.h"
#include "visualizemod.h"
#include "graph.h"
#include "evaluation.h"
#include "multilevel.h"

data data_model;
visualizemod vis;
vector<Multilevel> mls;

static PyObject *Clean (PyObject *self, PyObject *args) {
    data_model.clean();
    vis.clean();
    vector<Multilevel>().swap(mls);
    return Py_None;
}

static PyObject *LoadFromGraph (PyObject *self, PyObject *args) {
    char *infile;
    if (!PyArg_ParseTuple(args, "s", &infile)) {
        printf("Input error!\n");
        return Py_None;
    }
    data_model.load_from_graph(infile);
    return Py_None;
};


static PyObject *ShortestPathLength (PyObject *self, PyObject *args) {
    int dismax;
    if (!PyArg_ParseTuple(args, "i", &dismax)) {
        printf("Input error!\n");
        return Py_None;
    }
    graph g(data_model.n_vertices);
    g.shortest_path_length(&data_model, dismax);
    return Py_None;
};

static PyObject *GenMultilevel (PyObject *self, PyObject *args) {
    int mingroup;
    if (!PyArg_ParseTuple(args, "i", &mingroup)) {
        printf("Input error!\n");
        return Py_None;
    }
    data_model.prepare_data();
    mls = Multilevel::gen_multilevel(&data_model, mingroup);
    return Py_None;
}

static PyObject *ComputeSimilarity (PyObject *self, PyObject *args) {
    int n_threads, dismax;;
    float perplexity;
    double f = -1;
    if (!PyArg_ParseTuple(args, "idi", &n_threads, &f, &dismax)) {
        printf("Input error!\n");
        return Py_None;
    }
    perplexity = f;
    data_model.compute_similarity(n_threads, perplexity, dismax);
    return Py_None;
};


static PyObject *Run (PyObject *self, PyObject *args) {
    int out_dim = -1, n_samples = -1, n_threads = -1, n_negative = -1, n_neighbors = -1, isout = 0;
    float alpha = -1, n_gamma = -1;
    char *outfolder;
    double f1 = -1, f2 = -1;
    if (!PyArg_ParseTuple(args, "iiidid", &out_dim, &n_threads, &n_samples, &f1, &n_negative, &f2)) {
        printf("Input error  run!\n");
        return Py_None;
    }
    alpha = f1;
    n_gamma = f2;
    vis.run(&data_model, out_dim, n_threads, n_samples, alpha, n_negative, n_gamma, mls);
    return Py_None;
};

static PyObject *Save (PyObject *self, PyObject *args) {
    char *outfile;
    if (!PyArg_ParseTuple(args, "s", &outfile)) {
        printf("Input error!\n");
        return Py_None;
    }
    vis.save(outfile);
    return Py_None;
};

static PyMethodDef PyExtMethods[] = {
    {"loadFromGraph", LoadFromGraph, METH_VARARGS, ""},
    {"shortestPathLength", ShortestPathLength, METH_VARARGS, ""},
    {"computeSimilarity", ComputeSimilarity, METH_VARARGS, ""},
    {"genMultilevel", GenMultilevel, METH_VARARGS, ""},
    {"clean", Clean, METH_VARARGS, ""},
    {"run", Run, METH_VARARGS, ""},
    {"save", Save, METH_VARARGS, ""},
};

static struct PyModuleDef DRGraph =
{
    PyModuleDef_HEAD_INIT,
    "DRGraph", /* name of module */
    "",          /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    PyExtMethods
};

PyMODINIT_FUNC initDRGraph()
{
    printf("DRGraph successfully imported!\n");
    return PyModule_Create(&DRGraph);
}
