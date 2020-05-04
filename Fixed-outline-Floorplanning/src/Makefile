CC			:= gcc
CXX			:= g++

MPICC			:= mpicc
MPICXX			:= mpicxx

CCFLAGS			:= -O3 -march=native -Wall -std=gnu11
CXXFLAGS		:= -O3 -march=native -Wall -std=c++0x

PFLAGS			:= -lpthread

all: hw3

main: hw3.cpp
	$(CXX) $(CXXFLAGS) -o $@ $?
clean:
	rm -f hw3
veryclean:
	rm -f hw3 *.out