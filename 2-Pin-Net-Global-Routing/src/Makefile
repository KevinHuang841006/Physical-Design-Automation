CC			:= gcc
CXX			:= g++

MPICC			:= mpicc
MPICXX			:= mpicxx
HERDERS = compare.h
CCFLAGS			:= -O3 -march=native -Wall -std=gnu11
CXXFLAGS		:= -O3 -march=native -Wall -std=c++0x

PFLAGS			:= -lpthread

all: route

main: route.cpp
	$(CXX) $(CXXFLAGS) -o $@ $?
clean:
	rm -f route
veryclean:
	rm -f route *.out