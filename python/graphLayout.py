#coding=utf-8
import DRGraph as gl
import argparse
import warnings
from layout import txtToPng

def fxn():
    warnings.warn("deprecated", DeprecationWarning)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-input', default = '', help = 'Input graphs')
    parser.add_argument('-output', default = '', help = 'Graph layout results')
    parser.add_argument('-outpng', default = '', help = 'Visualization results')
    parser.add_argument('-threads', default = 8, type = int, help = 'Number of threads')
    parser.add_argument('-samples', default = 500, type = int, help = 'Number of edge samples for graph layout.')
    parser.add_argument('-alpha', default = 1.0, type = float, help = 'Learning rate')
    parser.add_argument('-neg', default = 10, type = int, help = 'Number of negative samples')
    parser.add_argument('-gamma', default = 1, type = float, help = 'Weights of negative edges')
    parser.add_argument('-perp', default = 50.0, type = float, help = 'Perplexity')
    parser.add_argument('-k', default = 1, type = int, help = 'k-order nearst neighbors')

    args = parser.parse_args()

    gl.loadFromGraph(args.input)
    gl.shortestPathLength(args.k)
    gl.genMultilevel(100)
    gl.computeSimilarity(args.threads, args.perp, args.k)

    gl.run(2, args.threads, args.samples, args.alpha, args.neg, args.gamma)
    gl.save(args.output)
    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        txtToPng(args.output, args.outpng, args.input)
        fxn()