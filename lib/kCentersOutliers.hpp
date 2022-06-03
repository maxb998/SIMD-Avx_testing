#include <unistd.h>
#include <vector>
#include <immintrin.h>
#include <iostream>

#ifndef KCENTERS_OUTLIERS_HPP
#define KCENTERS_OUTLIERS_HPP

class KCentersOutliers
{
private:
    float* P;
    int n, dims;
public:
    KCentersOutliers();
    ~KCentersOutliers();
    bool loadDataset(char *filename);
    std::vector<float*> SeqWeightedOutliers(int k, int z, float alpha);
};


#endif // KCENTERS_OUTLIERS_HPP