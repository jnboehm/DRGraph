#include "data.h"
#include "graph.h"
#include "evaluation.h"

int k = 2;

char _graph[1000], _layout[1000];
int out_dim = -1, n_threads = -1, n_negative = -1;

float alpha = -1, n_gamma = -1, perplexity = -1;
int ArgPos(char *str, int argc, char **argv) {
    int a;
    for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
            if (a == argc - 1) {
                printf("Argument missing for %s\n", str);
                exit(1);
            }
            return a;
        }
    return -1;
}

int main(int argc, char **argv)
{
    int i;
    if (argc < 2)
    {
        printf("-graph: Input graphs.\n");
        printf("-layout: Graph layout result.\n");
        printf("-k: k-order nearst neighbors. Default = 2.\n");
        return 0;
    }
    if ((i = ArgPos((char *)"-graph", argc, argv)) > 0) strcpy(_graph, argv[i + 1]);
    if ((i = ArgPos((char *)"-layout", argc, argv)) > 0) strcpy(_layout, argv[i + 1]);
    if ((i = ArgPos((char *)"-k", argc, argv)) > 0) k = atof(argv[i + 1]);

        data data_model;
        data_model.load_from_graph(_graph);

        graph g(data_model.n_vertices);
        g.shortest_path_length(&data_model, k);
        data_model.prepare_data();

        evaluation eva(&data_model);
        eva.load_data(_layout);
        eva.jaccard();
}
