#ifndef KCENTERS_OUTLIERS_HPP
#define KCENTERS_OUTLIERS_HPP

#include <immintrin.h>
#include <math.h>
#include <stdio.h>

const int simdVecSize = 8;
float * loadDataset(char *path, int *n, int *dims);
void seqWeightedOutliers(float *points, int n, int dims, int k, int z, float alpha);
void fillDistanceMatrixRow(float *points, int n, int dims, float *pt, float *distSquaredOut, int pos);


#endif // KCENTERS_OUTLIERS_HPP