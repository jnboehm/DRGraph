#include <fstream>
#include "data.h"
#include "visualizemod.h"
#include "graph.h"
#include "multilevel.h"

char infile[1000], outfile[1000], outfolder[1000];
int out_dim = -1, n_threads = -1, n_negative = -1, n_neighbors = -1, k = 1;
long long n_samples = -1;
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
    if (argc < 3)
    {
        printf("-input: Input graphs\n");
        printf("-output: Graph layout results.\n");
        printf("-threads: Number of threads. Default = 8.\n");
        printf("-samples: Number of edge samples for graph layout. Default = 500.\n");
        printf("-alpha: Learning rate. Default = 1.0.\n");
        printf("-n_neg: Number of negative samples. Default = 10.\n");
        printf("-gamma: Weights of negative edges. Default = 1.\n");
        printf("-perp: Perplexity. Default = 50.0.\n");
        printf("-k: k-order nearst neighbors. Default = 1.\n");
        return 0;
    }
	if ((i = ArgPos((char *)"-input", argc, argv)) > 0) strcpy(infile, argv[i + 1]);
	if ((i = ArgPos((char *)"-output", argc, argv)) > 0) strcpy(outfile, argv[i + 1]);
	if ((i = ArgPos((char *)"-samples", argc, argv)) > 0) n_samples = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) n_threads = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-n_neg", argc, argv)) > 0) n_negative = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-alpha", argc, argv)) > 0) alpha = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-gamma", argc, argv)) > 0) n_gamma = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-perp", argc, argv)) > 0) perplexity = atof(argv[i + 1]);
    if ((i = ArgPos((char *)"-k", argc, argv)) > 0) k = atof(argv[i + 1]);

    data data_model;
	data_model.load_from_graph(infile);

	graph g(data_model.n_vertices);
    g.shortest_path_length(&data_model, k);


    data_model.prepare_data();

    vector<Multilevel> mls = Multilevel::gen_multilevel(&data_model, 100);

    data_model.compute_similarity(n_threads, perplexity, k);

    visualizemod vis;
	vis.run(&data_model, out_dim, n_threads, n_samples, alpha, n_negative, n_gamma, mls);

	vis.save(outfile);
}