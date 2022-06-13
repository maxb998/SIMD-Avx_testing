#ifndef KCENTERS_OUTLIERS_HPP
#define KCENTERS_OUTLIERS_HPP

#include <unistd.h>
#include <vector>
#include <immintrin.h>
#include <iostream>
#include <math.h>

class KCentersOutliers
{
private:
    float *P, *S;
    int *W;
    int n, dims, arrColumns;
    void buildSquaredDistanceArray(float *allDistSquared);
    float findMinDistFirstPts(float *allDistSquared, int limitFirstPts);
    int findNextCenterId(float *allDistSquared, int *iterW, float ballRadiusSquared);
    void setNewCenterPtsCovered(float *allDistSquared, int *iterW, float ballRadiusSquared, int newCenterId);
public:
    KCentersOutliers();
    ~KCentersOutliers();
    bool loadDataset(char* filename);
    void loadWeights(int* weights);
    void genUnitWeights();
    void SeqWeightedOutliers(int k, int z, float alpha);
};


#endif // KCENTERS_OUTLIERS_HPP