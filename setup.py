from distutils.core import setup, Extension

DRGraph = Extension('DRGraph',
                        sources=['data.cpp', 'visualizemod.cpp', 'graph.cpp', 'DRGraphModule.cpp', 'evaluation.cpp', 'multilevel.cpp', 'genrandom.cpp'],
                        depends=['data.h', 'visualizemod.h', 'shortestpath.h', 'evaluation.cpp', 'multilevel.h', 'genrandom.h'],
                        include_dirs = ['/usr/local/include'],
                        library_dirs = ['/usr/local/lib'],
                        libraries=['gsl', 'gslcblas'],
                        extra_compile_args=['-lm -pthread -lgsl -lgslcblas -Ofast -march=native -ffast-math -lboost_filesystem', '-fopenmp', '-fpic', '-std=c++11'],
                        extra_link_args=['-lgomp'])

setup (name = 'DRGraph',
       version = '1.0',
       description = 'DRGraph',
       ext_modules = [DRGraph])
