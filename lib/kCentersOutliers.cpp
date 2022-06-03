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
    
    int arrSize = dims * (n + (8 - (n % 8)));    // create proper size in order to load better arrays into m256

    // load dataset
    P = (float*)aligned_alloc(32, sizeof(float)*arrSize);   // must be aligned to 32 to load better m256

    for (int i = 0; (getline(&line, &len, f)) != -1; i++) // check if end of document has been reached
    {
        string str = string(line);
        size_t first_separator = 0, second_separator = str.find(',');   // assumes there are no random commas at the start of any line in the database
        for (int j = 0; j < dims; j++)
        {
            P[i + j * n] = stof(str.substr(first_separator, second_separator-first_separator));
            first_separator = second_separator+1;
            second_separator = str.find(',', second_separator);
        }
    }
    
    fclose(f);

    /*/// code to print array -> DEBUG
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < dims; j++)
            cout << P[i + j*n] << "; ";
        cout << endl;
    }*/

    return true;
}


vector<float*> KCentersOutliers::SeqWeightedOutliers(int k, int z, float alpha)
{
    int attempt = 0;

    
    
    // generate matrix with all distances squared
    int arrSize = (n + (8 - (n % 8))) ^ 2;
    float* all_dist_squared = (float*)aligned_alloc(32, sizeof(float) * );
    float* last = (float*)aligned_alloc(32, 8 * sizeof(float));





    free(all_dist_squared);
    free(last);

    vector<float*> temp;
    return temp;
}