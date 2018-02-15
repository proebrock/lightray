CXX=g++
CPPFLAGS=-W -Wall -O2

PYTHON=python3

.PHONY: all
all: module

module: RS_FEC_module.cpp RS_FEC.h
	$(PYTHON) setup.py build_ext --inplace

.PHONY: clean
clean:
	rm -rf RS_FEC*.so build
