# DRGraph: An Efficient Graph Layout Algorithm for Large-scaleGraphs by Dimensionality Reduction &middot; [![GitHub license](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/DRGraphVIS/DRGraph/blob/master/LICENSE)

## Introduction
DRGraph achieves a linear complexity for both the computation and memory consumption, and scales up to large-scale graphs with millions of nodes. Experimental results and comparisons with state-of-the-art graph layout methods demonstrate the superiority of our approach.

## Building
The algorithm is mostly implemented in C++11, with an optional Python2.7 interface. We recommend you run it on Ubuntu 16.04.
### Requirments
Recommend you install them by compiling.
- G++(>= 5.4)
- [CMake](https://cmake.org/) (>= 3.6)
- [BOOST](http://www.boost.org/) (>= 1.66)
- [GSL (GNU Scientific Library)](http://www.gnu.org/software/gsl/) (>= 2.4)


# How to use

To install the package, please do:
```
$ git clone https://github.com/DRGraphVIS/DRGraph.git
$ cd DRGraph/
```

## C++
### Install
Make sure `cmake` is installed on your system, and you will also need a sensible C++ compiler, such as `gcc` or `llvm-clang`.    
```
$ cmake .
$ make
```


### Run
```
$ ./DRGraph -input ./data/dwt_419.txt -output ./python/dwt_419_vec2D.txt
```

### Evaluation
    $ ./Evaluation -graph ./data/dwt_419.txt -layout ./python/dwt_419_vec2D.txt



## Python

Run with Python 2.7 (recommend conda) and Ubuntu 16.04.

### Install
```
$ cd DRGraph/
$ sudo pip install -r requirements.txt
$ sudo python setup.py install
```

### Run
In this step, you can get both the layout file and the visualization results.
```
$ cd python/
$ python graphLayout.py -input ../data/dwt_419.txt -output dwt_419_vec2D.txt -outpng ./dwt_419.png
```
### Parameters

- `-input`: Input graphs.
- `-output`: Graph layout result.
- `-threads`: Number of threads. Default = 8.
- `-samples`: Number of edge samples for graph layout. Default = 500.
- `-alpha`: Learning rate. Default = 1.0.
- `-n_neg`: Number of negative samples. Default = 5.
- `-gamma`: Weights of negative edges. Default = 1. Set `gamma` = 0.01, you can get a better visualization results.
- `-perp`: Perplexity. Default = 50.
- `-k`: k-order nearst neighbors. Default = 1.

### Visualize 
We provide some examples including `dwt_419, dwt_1005, 3elt, etc` in the `data/` folder (You can get these data on [Google drive](https://drive.google.com/drive/folders/1_MDKW_QcK1MsvL23ECmLv-mCUmxykce-?usp=sharing).). For example, to visualize the `dwt_419` dataset.
If you run it by Python, you can skip this step. 
```
cd python/
python layout.py -layout ./dwt_419_vec2D.txt -outpng ./dwt_419.png -graph ../data/dwt_419.txt
```
    

## Web
The visualization results on the browser.
```
$ cd web/backend
$ python manage.py runserver

```
## License
DRGraph is [Apache 2.0 licensed](https://github.com/DRGraphVIS/DRGraph/blob/master/LICENSE). 