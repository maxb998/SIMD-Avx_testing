#include "kCentersOutliers.h"


float * loadDataset(char *path, int *n, int *dims)
{
    FILE *stream = fopen(path, "r");
    
    char *number
}

void seqWeightedOutliers(float *points, int n, int dims, int k, int z, float alpha)
{
    int fullNSize = n + (n % simdVecSize);
    float *allDistSquared = aligned_alloc(32, sizeof(float) * fullNSize * n);
    
    float *pt = aligned_alloc(32, sizeof(float) * dims);
    for (int i = 0; i < n; i++)
    {
        // need to fill the array pt with the current point coordinates
        for (int j = 0; j < dims; j++)
            pt[j] = points[i + j * fullNSize];

        fillDistanceMatrix(points, n, dims, pt, allDistSquared, i * fullNSize);
    }
    free(pt);





    free(allDistSquared);
}

void fillDistanceMatrixRow(float *points, int n, int dims, float *pt, float *distSquaredOut, int pos)
{
    int fullNSize = n + (n % simdVecSize);
    __m256 squaredSum;
    for (int i = 0; i < n; i += 8)
    {
        squaredSum = _mm256_setzero_ps();
        for (int j = 0; j < dims; j++)
        {
            __m256 diff = _mm256_sub_ps(_mm256_load_ps(&points[i + j * fullNSize]), _mm256_set1_ps(pt[j]));
            diff = _mm256_mul_ps(diff, diff);
            squaredSum = _mm256_add_ps(squaredSum, diff);
        }
        _mm256_store_ps(&distSquaredOut[pos + i], squaredSum);
    }
}