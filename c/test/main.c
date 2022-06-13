#include "kCentersOutliers.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    int k = 3, z = 3;

    
    kCentersData data = loadDataset(argv[1]);
    seqWeightedOutliers(data, k, z, 0.);

    return EXIT_SUCCESS;
}