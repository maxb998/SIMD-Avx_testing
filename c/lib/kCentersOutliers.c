#include "kCentersOutliers.h"

const int smidVecSize = 8, smidAllignment = 32;

kCentersData loadDataset(char * path)
{
    kCentersData data;
    FILE *stream = fopen(path, "r");

    // check if file exists
    if (stream == NULL)
    {
        printf("File could not be found");
        exit(EXIT_FAILURE);
    }
    
    // count lines(n) and elements-per-line(dims)
    data.n = 1;
    data.dims = 1;

    char *line = NULL;
    size_t lineSize = 0; 
    if (getline(&line, &lineSize, stream) == -1)    // check if file is empty
    {
        printf("File is empty");
        exit(EXIT_FAILURE);
    }
    // find dims
    char * ptr = strtok(line, ";");
    while (ptr != NULL)
    {
        data.dims++;
        ptr = strtok(NULL, ";");
    }
    // find n
    while (getline(&line, &lineSize, stream) != EOF) // read till end of file
        data.n++;

    // reset pointer to begin of file
    rewind(stream);

    
    int currentDim, ptId = 0;
    // allocate array
    data.fullNSize = data.n + smidVecSize - (data.n % smidVecSize);
    data.P = aligned_alloc(smidAllignment, sizeof(float) * data.fullNSize * data.dims);
    data.W = NULL;
    
    
    while (getline(&line, &lineSize, stream) != EOF)
    {
        currentDim = 0;
        ptr = strtok(line, ",");
        while ((line != NULL) && (currentDim < data.dims))
        {
            data.P[ptId + currentDim * data.fullNSize] = atof(ptr);
            ptr = strtok(NULL, ",");
            currentDim++;
        }
        ptId++;
    }

    if (line)
        free(line);

    // now fill the remaining values with NaN
    for (int i = data.n; i < data.fullNSize; i++)
        for (int j = 0; j < data.dims; j++)
            data.P[i + j * data.fullNSize] = nanf("");

    return data;
}

void loadUnitW(kCentersData * data)
{
    data->W = (int*)aligned_alloc(smidAllignment, sizeof(int) * data->fullNSize);
    for (int i = 0; i < data->n; i++)
        data->W[i] = 1;
    for (int i = data->n; i < data->fullNSize; i++)
        data->W[i] = 0;
    
    /*
    for (int i = 0; i < data->fullNSize; i++)
        printf("W[%d] = %d\n", i, data->W[i]);
    //*/
}

kCentersSolution seqWeightedOutliers(kCentersData data, int k, int z, float alpha)
{
    float * allDistSquared = computeDistanceMatrix(data);
    //printMatrix(allDistSquared, data.n, data.fullNSize, false);
    float initialGuess = firstGuess(allDistSquared, data.n, data.dims, z+k+1);

    printf("Initial guess = %f\n", initialGuess);

    kCentersSolution s;
    s.dims = data.dims;
    //s.S = (float*)aligned_alloc(smidAllignment, sizeof(float) * data.dims * k);

    int solutionIDs[k];
    int * currentW = (int*)aligned_alloc(smidAllignment, sizeof(int) * data.fullNSize);

    int * W8Sum = (int*)aligned_alloc(smidAllignment, sizeof(int) * smidVecSize);
    
    long uncoveredWSum = 0L;
    int center;
    
    int maxWSum, currWSum, newCenterID;

    do
    {
        // fill array of solutions with -1 in order to check how many centers are found
        for (int i = 0; i < k; i++)
            solutionIDs[i] = -1;
        // reset currentW to original weights
        memcpy((void*)currentW, (void*)data.W, sizeof(int) * data.fullNSize);

        // find centers
        center = 0;
        while (center < k)
        {
            maxWSum = 0; currWSum = 0; newCenterID = -1;

            for (int i = 0; i < data.n; i++)
            {
                // for each point calculate sum between all points that would be under that center inner radius
                memset((void*)W8Sum, 0x00, sizeof(int) * smidVecSize);

                for (int j = 0; j < data.n / smidVecSize; j += smidAllignment)
                {
                    __m256i mask = _mm256_cmpgt_epi32((__m256i)_mm256_load_ps((float*)))
                }
                // sum elements of array with 8 elems sum
                currWSum = 0;
                for (int j = 0; j < smidVecSize; j++)
                    currWSum += W8Sum[j];

                // check if better center has been found
                if (maxWSum < currWSum)
                {
                    maxWSum = currWSum;
                    newCenterID = i;
                }
            }
            
            if (newCenterID == -1) break;   // no center has been found in this iteration -> all points have already been covered
            
            // set bigger radius to find new covered points


            // set new covered points weight to 0


            center++;
        }

        if (newCenterID == -1)  // all points have been convered before reaching k number of clusters -> algorithm has found the centers
        {
            s.S = (float*)malloc(sizeof(float) * (center + 1) * data.dims);
            for (int i = 0; i < center; i++)
                for (int dim = 0; dim < data.dims; dim++)
                    s.S[i + dim * (center + 1)] = data.P[];
            
            break;
        }
        
        
    } while (uncoveredWSum > (long)z);
    
    free(currentW);
    free(W8Sum);
    free(allDistSquared);
}

float * computeDistanceMatrix(kCentersData data)
{
    float * allDistSquared = aligned_alloc(smidAllignment, sizeof(float) * data.fullNSize * data.n);
    __m256 squaredSum, diff;

    for (int ptId = 0; ptId < data.n; ptId++)
    {
        for (int i = 0; i < data.n; i += 8)
        {
            squaredSum = _mm256_setzero_ps();   // set to zero at beginning of the cummulative sum
            for (int j = 0; j < data.dims; j++)
            {
                diff = _mm256_sub_ps(_mm256_load_ps(&data.P[i + j * data.fullNSize]), _mm256_set1_ps(data.P[ptId + j * data.fullNSize]));
                diff = _mm256_mul_ps(diff, diff);
                squaredSum = _mm256_add_ps(squaredSum, diff);
            }
            _mm256_store_ps(&allDistSquared[ptId * data.fullNSize + i], squaredSum);
        }
    }

    return allDistSquared;    
}

float firstGuess(float * distances, int n, int dims, int ptToUse)
{
    int limit = ptToUse;
    // make the use a different limit if the one provided isn't valid
    if ((ptToUse > n) || (ptToUse <= 0))
        limit = n;

    int fullNSize = n + smidVecSize - (n % smidVecSize);
    float min = distances[1];
    for (int i = 1; i < ptToUse; i++)
        for (int j = i+1; j < ptToUse; j++)
            if (distances[i * fullNSize + j] < min)
                min = distances[i * fullNSize, j];
    return min;
}



void printMatrix(float * matr, int n, int m, bool issmidOptimized)  // in this case only n are the rows while m are the columns
{
    int fullMSize = m;
    if(issmidOptimized)
        fullMSize = m + smidVecSize - (m % smidVecSize);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
            printf("%.3f;  ", matr[j + i * fullMSize]);
        printf("\n");
    }
}