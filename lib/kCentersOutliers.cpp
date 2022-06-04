#include "kCentersOutliers.hpp"

using namespace std;

KCentersOutliers::KCentersOutliers()
{
    n = 0;
    dims = 1;
}

KCentersOutliers::~KCentersOutliers()
{
    free(P);
    free(W);
    free(S);
}

bool KCentersOutliers::loadDataset(char *filename)
{
    FILE* f = fopen(filename, "r");
    // does file exists?
    if (f == NULL)
    {
        cout << "file \"" << filename << "\" cannot be found at the specified location" << endl;
        return false;
    }

    // count number of points and dims
    char* line = NULL;
    size_t len = 0;
    if ((getline(&line, &len, f)) != -1)    // check if end of document has been reached
    {
        for (int i = 0; i < sizeof(line)/sizeof(line[0]); i++)
            if (line[i] == ',')
                dims++;
    }
    n++;
    while ((getline(&line, &len, f)) != -1) // check if end of document has been reached
        n++;

    fseek(f, 0, SEEK_SET);  // restart reading from beginning of file
    
    arrColumns = n + (8 - (n % 8));    // create proper size in order to load better arrays into m256

    // load dataset
    P = (float*)aligned_alloc(32, sizeof(float)*arrColumns*dims);   // must be aligned to 32 to load better m256

    for (int i = 0; (getline(&line, &len, f)) != -1; i++) // check if end of document has been reached
    {
        string str = string(line);
        size_t first_separator = 0, second_separator = str.find(',');   // assumes there are no random commas at the start of any line in the database
        for (int j = 0; j < dims; j++)
        {
            P[i + j * arrColumns] = stof(str.substr(first_separator, second_separator-first_separator));
            first_separator = second_separator+1;
            second_separator = str.find(',', second_separator);
        }
    }
    
    fclose(f);

    for (int i = n; i < arrColumns; i++)
        for (int j = 0; j < dims; j++)
            P[i + j * arrColumns] = nanf("");
    

    /*// DEBUG -> code to print array
    for (int i = 0; i < arrColumns; i++)
    {
        for (int j = 0; j < dims; j++)
            cout << P[i + j * arrColumns] << "; ";
        cout << endl;
    }// */

    return true;
}

void KCentersOutliers::loadWeights(int *weights)
{
    if (n != 0)
    {
        W = (int*)aligned_alloc(32, sizeof(int) * arrColumns);
    }
    else
        cout << "You must load the dataset before setting the weights" << endl;
}

void KCentersOutliers::genUnitWeights()
{
    if (n != 0)
    {
        W = (int*)aligned_alloc(32, sizeof(int) * arrColumns);
        fill(&W[0], &W[n], 1);
        if (n < arrColumns)
            fill(&W[n], &W[arrColumns], 0);  
    }
    else
        cout << "you must load the dataset before setting the weights" << endl;
}


void KCentersOutliers::SeqWeightedOutliers(int k, int z, float alpha)
{
    int attempt = 0;
    
    // squared distance array allocation
    float* allDistSquared = (float*)aligned_alloc(32, sizeof(float) * arrColumns * n);
    buildSquaredDistanceArray(allDistSquared);
    
    float rSquared = findMinDistFirstPts(allDistSquared, z + k + 1) / 16.;
    float firstGuess = sqrt(rSquared*4.);
    cout << firstGuess << endl;
    

    int outliersW = n;
    S = (float*)aligned_alloc(32, k * dims * sizeof(float));
    int *iterW = (int*)aligned_alloc(32, arrColumns*sizeof(int));
    do
    {
        attempt++;
        rSquared *= 4.;

        /*cout << "attempt n°: " << attempt << endl;
        cout << "radius = " << rSquared << endl;*/

        fill(S, &S[k * dims], nanf(""));
        
        copy(W, &W[arrColumns], iterW);

        for (int center = 0; center < k; center++)
        {
            float ballRadiusSquared = (1.+2.*alpha)*(1.+2.*alpha)*rSquared;

            // find next candidate for the given radius
            int bestPtId = findNextCenterId(allDistSquared, iterW, ballRadiusSquared);

            // save candidate into solution array
            for (int i = 0; i < dims; i++)
                S[center + i * k] = P[bestPtId + i * arrColumns];

            // set newly covered points weight in iterW to 0
            ballRadiusSquared = (3.+4.*alpha)*(3.+4.*alpha)*rSquared;
            setNewCenterPtsCovered(allDistSquared, iterW, ballRadiusSquared, bestPtId);

            if (attempt == 6)
            {
                for (int i = 0; i < arrColumns; i++)
                    cout << iterW[i] << ";  ";
                cout << endl;
            }
            
        }

        // compute outliers weight
        __m256i vecSum = _mm256_set1_epi32(0);
        for (int i = 0; i < n; i += 8)
            vecSum = _mm256_add_epi32(vecSum, (__m256i)_mm256_load_ps((float*)&iterW[i]));
        int *arrSum = (int*)&vecSum;
        outliersW = 0;
        for (int i = 0; i < 8; i++)
            outliersW += arrSum[i];
        
    } while (outliersW > z);

    for (int i = 0; i < k; i++)
    {
        for (int j = 0; j < dims; j++)
            cout << S[i + j * k] << ";  ";
        cout << endl;
    }
    
    
    free(iterW);
    free(allDistSquared);
}

void KCentersOutliers::buildSquaredDistanceArray(float* allDistSquared)
{
    // begin filling array by 8 points at a time
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j += 8)
        {
            __m256 squaredSum = _mm256_setzero_ps();
            for (int l = 0; l < dims; l++)
            {
                __m256 difference = _mm256_sub_ps(_mm256_set1_ps(P[i + l * arrColumns]), _mm256_load_ps(&P[j + l * arrColumns]));
                difference = _mm256_mul_ps(difference, difference);
                squaredSum = _mm256_add_ps(squaredSum, difference);
            }
            float* vecOpResult = (float*)&squaredSum;
            copy(vecOpResult, &vecOpResult[8], &allDistSquared[i * arrColumns + j]);
        }
    }
    
    /*//print allDistSquared
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < arrColumns; j++)
            cout << allDistSquared[i*arrColumns + j] << ",";
        cout << "\b\b" << endl;
    }*/
}

float KCentersOutliers::findMinDistFirstPts(float* allDistSquared, int limitFirstPts)
{
    int limit = limitFirstPts;
    if (limit > n)
        limit = n;

    float min = allDistSquared[1];
    for (int i = 0; i < limit; i++)
        for (int j = i+1; j < limit; j++)
            if (allDistSquared[i * arrColumns + j] < min)
                min = allDistSquared[i * arrColumns + j];
    return min;
}

int KCentersOutliers::findNextCenterId(float *allDistSquared, int *iterW, float ballRadiusSquared)
{
    int bestPtId = -1, bestWConcentration = 0;
    for (int ptId = 0; ptId < n; ptId++)
    {
        if (iterW[ptId] == 0)  continue;    // if the point has already been covered skip to the next one

        // compute weight around current point
        __m256i wSum = _mm256_set1_epi32(0);
        for (int i = 0; i < n; i += 8)
        {
            __m256i mask = (__m256i)_mm256_cmp_ps(_mm256_load_ps(&allDistSquared[ptId * arrColumns + i]), _mm256_set1_ps(ballRadiusSquared), _CMP_LT_OS);
            wSum = _mm256_add_epi32(_mm256_maskload_epi32(&iterW[i], mask), wSum);
        }

        int *wSumResult = (int *)&wSum, currentW = 0;
        for (int i = 0; i < 8; i++)
            currentW += wSumResult[i];

        if (currentW > bestWConcentration)
        {
            bestWConcentration = currentW;
            bestPtId = ptId;
        }
    }
    return bestPtId;
}

void KCentersOutliers::setNewCenterPtsCovered(float *allDistSquared, int *iterW, float ballRadiusSquared, int newCenterId)
{
    for (int i = 0; i < n; i += 8)
    {
        __m256i mask = (__m256i)_mm256_cmp_ps(_mm256_load_ps(&allDistSquared[newCenterId * arrColumns + i]), _mm256_set1_ps(ballRadiusSquared), _CMP_LT_OS);
        mask = _mm256_andnot_si256(mask, _mm256_set1_epi32(-1));
        __m256i loaded = _mm256_maskload_epi32(&iterW[i], mask);

        // store result into iterW
        int *loadedCast = (int *)&loaded;
        copy(loadedCast, &loadedCast[8], &iterW[i]);
    }
}