#include "kCentersOutliers.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    int k = 3, z = 3;

    
    kCentersData data = loadDataset(argv[1]);
    loadUnitW(&data);
    seqWeightedOutliers(data, k, z, 0.);

    free(data.P);
    free(data.W);

    return EXIT_SUCCESS;
}