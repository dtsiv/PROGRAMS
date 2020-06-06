#ifndef ROUTINES_H
#define ROUTINES_H

#include <QCoreApplication>
#include <QtCore>
#include <iostream>
#include <fstream>

using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include "lapacke.h"

/* Auxiliary routines prototypes */
extern void print_matrix( const char* desc, lapack_int m, lapack_int n, double* a, lapack_int lda );
extern void print_int_vector( const char* desc, lapack_int n, lapack_int* a );
void inverse(double* A, int N_loc);

/* Parameters */
#define N 8
#define NFREQRESP 80
#define LDA N
#define LDVL N
#define LDVR N

// #define UNFILTERED
// #define WEIGHTED

int xcgeev();
int xdsbev();
// DPBSV computes the solution to system of linear equations A * X = B for OTHER matrices
int xdpbsv();
int hamming();

#endif // ROUTINES_H
