#ifndef KCENTERS_OUTLIERS_H
#define KCENTERS_OUTLIERS_H

#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

typedef struct kCentersData
{
    float * P;
    int * W;
    int n, dims, fullNSize;
} kCentersData ;

typedef struct kCentersSolution
{
    float * S;
    int k, dims, fullKSize;
} kCentersSolution ;

kCentersData loadDataset(char * path);
kCentersSolution seqWeightedOutliers(kCentersData data, int k, int z, float alpha);
float * computeDistanceMatrix(kCentersData data);
float firstGuess(float * distances, int n, int dims, int ptToUse);
void loadUnitW(kCentersData * data);

void printMatrix(float * matr, int n, int m, bool isSimdOptimized);


#endif // KCENTERS_OUTLIERS_H
