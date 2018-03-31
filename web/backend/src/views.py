# -*- coding: utf-8 -*-
from __future__ import unicode_literals
from django.http import JsonResponse
import os
import json
import DRGraph as gl
import networkx as nx
import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot
import numpy as np
import sys
import warnings
import random
import time

datasets_path = './src/static/data/'
outfile_path = './src/static/out/'
png_path = './src/static/'

# Create your views here.
def fxn():
    warnings.warn("deprecated", DeprecationWarning)

def getDatasets(request):
    if not os.path.exists(datasets_path):  
        os.makedirs(datasets_path)  
    if not os.path.exists(png_path):  
        os.makedirs(png_path)  
    if not os.path.exists(outfile_path):  
        os.makedirs(outfile_path)  
    results = []
    files = os.listdir(datasets_path)
    # print(files)
    for file in files:
        results.append(file.replace('.txt', ''))
    return JsonResponse(results, safe=False)

# Normalize in [0, 1] x [0, 1] (without changing aspect ratio)
def normalize(Y):
    Y_cpy = Y.copy()
    # Translate s.t. smallest values for both x and y are 0.
    for dim in range(Y.shape[1]):
        Y_cpy[:, dim] += -Y_cpy[:, dim].min()
    # Scale s.t. max(max(x, y)) = 1 (while keeping the same aspect ratio!)
    scaling = 1 / (np.absolute(Y_cpy).max())
    Y_cpy *= scaling
    return Y_cpy


class ShowProcess():
    
    i = 0 
    max_steps = 0 
    max_arrow = 50 

    def __init__(self, max_steps):
        self.max_steps = max_steps
        self.i = 0

  
    def show_process(self, i=None):
        if i is not None:
            self.i = i
        else :
            self.i += 1
        num_arrow = int(self.i * self.max_arrow / self.max_steps) 
        num_line = self.max_arrow - num_arrow 
        percent = self.i * 100.0 / self.max_steps 
        process_bar = '[' + '>' * num_arrow + '-' * num_line + ']'\
                      + '%.2f' % percent + '%' + '\r' 
        sys.stdout.write(process_bar) 
        sys.stdout.flush()

    def close(self, words='done'):
        print ''
        print words
        self.i = 0

def load_graphpos(filename):
    nnode = 0
    dim = 0
    with open(filename) as f:
        header = f.readline()
        nnode,dim = [int(i) for i in header.split()]
    rawdata = np.loadtxt(filename,dtype=float,skiprows=1)
    data = [(n[1],n[2]) for n in rawdata]
    return nnode, dim, np.array(data)

def txtToPng(infile, outpng, edgesfile):
    G=nx.Graph()
    nnode, dim, Y = load_graphpos(infile)
    data = normalize(Y)

    index = 0
    while index < nnode:
        i = data[index][0]
        j = data[index][1]
        G.add_node(index, pos=(i,j))
        index += 1


    f = open(edgesfile)
    line = f.readline()
    l1 = line.split(' ')
    nnode = int(l1[0])
    nedge = int(l1[1])
    index = 0
    while line:
        index += 1
        line = f.readline()
        ll = line.split(' ')
        if len(ll) < 3:
            continue
        i = int(ll[0])
        j = int(ll[1])
        if i == j:
            continue
        if nedge < 600000:
                G.add_edges_from([(i,j)])
        else:
            if nedge < 2000000:
                if index % 10 == 0:
                    G.add_edges_from([(i,j)])
            else:
                if index % 100 == 0:
                    G.add_edges_from([(i,j)])
    f.close()
    pos=nx.get_node_attributes(G,'pos')
    edges = G.edges()
    edge_length = []
    edge_length = [np.sqrt((pos[x][0]-pos[y][0])**2 + (pos[x][1]-pos[y][1])**2) for (x,y) in edges]
    edge_length = np.array(edge_length)
    edge_length = edge_length - np.min(edge_length)
    edge_length = edge_length / np.max(edge_length)
    
    pyplot.axis('off')
    set_width = 0.2
    if nedge > 400000:
        set_width = 0.02
    else:
        if nedge < 5000:
            set_width = 0.4

    infilestr = infile.split('/')
    infilestr = infilestr[len(infilestr) - 1]
    if infilestr.replace('.txt', '') == 'troll':
        set_width = 0.1
        # print(infilestr)

    nx.draw_networkx(G,pos, node_color='white',node_size=0, alpha=1, width = set_width, with_labels=False, edge_color = edge_length, edge_cmap = pyplot.get_cmap('jet_r'))
    # nx.draw(G,pos, node_color=nodecolor, edge_color=edgecolor, node_size=0, alpha=0.5, width = edgewidth)
    # pyplot.gca().xaxis.set_major_locator(plt.NullLocator())
    # pyplot.gca().yaxis.set_major_locator(plt.NullLocator())
    pyplot.savefig(outpng, dpi = 300, pad_inches = 0)
    pyplot.close('all')

def runGraphLayout(request):
    params = json.loads(request.body)
    # print(params)
    dismax = int(params['dismax']) if (int(params['dismax']) != 0) else 2<<22
    dataset = params['input']
    samples = params['sample']
    gamma = params['gamma']
    neg = params['negative']
    threads = 8
    perp = -1
    alpha = -1
    neigh = -1
    inputfile = os.path.join(datasets_path, dataset + '.txt')
    outfile = os.path.join(outfile_path, dataset + '.txt')
    outpng = os.path.join(png_path + dataset + str(random.random()) + '.png')

    result = {}
    try:
        gl.loadFromGraph(inputfile)
        runstart = time.time()
        gl.shortestPathLength(dismax)
        gl.genMultilevel(100)
        gl.computeSimilarity(threads, perp, dismax)
        
        gl.run(2, threads, samples, alpha, neg, gamma)
        runend = time.time()
        gl.save(outfile)
        renderstart = time.time()
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            txtToPng(outfile, outpng, inputfile)
            fxn()
        renderend = time.time()
    except Exception:
        print(Exception)
        result['status'] = 'failed'
    else:
        result['status'] = 'success' 
        result['png_path'] = outpng.replace(png_path, '')
        result['runningtime'] = runend - runstart
        result['renderingtime'] = renderend - renderstart
    return JsonResponse(result, safe=False)


def upload (request):
    # params = json.loads(request.body)
    # print(params)
    ret = {'status': 2, 'data': '', 'error': None}
    try:
        file = request.FILES.get('file')
        filename = str(request.FILES['file'])
        print(file)
        ret['data'] = filename.replace('.txt', '')
        if not os.path.exists(datasets_path+filename):
            with open(datasets_path+filename,'wb+') as destination:  
                for chunk in file.chunks():  
                    destination.write(chunk)
            ret['status'] = 0
        else:
            ret['status'] = 1
    except Exception as e:
        ret['error'] = e
    return JsonResponse(ret, safe=False)