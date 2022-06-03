#include <iostream>
#include <unistd.h>
#include <vector>
#include <immintrin.h>  // AVX lib ehehehehhehe

using namespace std;

vector<vector<float>> SeqWeightedOutliers(int n, int dims, float * pts, int W[], int k, int z, float alpha);

int main(int argc, char *argv[])
{
    if (argc != 4)
        throw invalid_argument("usage: kcenterscpp <DATASET_PATH> <MAX_N°_OF_CENTERS> <N°_OF_OULIERS");
    
    // check if second and third arguments are numbers
    float k = 0., z = 0.;
    if (isdigit(*argv[2]))
        k = stof(argv[2]);
    if (isdigit(*argv[3]))
        z = stof(argv[3]);

    
    FILE* f = fopen(argv[1], "r");
    // does file exists?
    if (f == NULL)
    {
        cout << "file \"" << argv[1] << "\" cannot be found at the specified location" << endl;
        exit(EXIT_FAILURE);
    }

    // find shape of 2D array
    int n = 0, dims = 1;

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

    // fill array with dataset points
    float* inputPoints = (float*)aligned_alloc(32, sizeof(float) * n * dims);
    for (int i = 0; (getline(&line, &len, f)) != -1; i++) // check if end of document has been reached
    {
        string str = string(line);
        size_t first_separator = 0, second_separator = str.find(',');   // assumes there are no random commas at the start of any line in the database
        for (int j = 0; j < dims; j++)
        {
            inputPoints[i*dims+j] = stof(str.substr(first_separator, second_separator-first_separator-1));
            first_separator = second_separator+1;
            second_separator = str.find(',', second_separator);
        }
    }
    
    fclose(f);

    /* // code to print array -> DEBUG
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < dims; j++)
            cout << inputPoints[i][j] << "; ";
        cout << endl;
    }*/
    
    int W[n];
    for (int i = 0; i < n; i++)
        W[i] = 1;
    
    SeqWeightedOutliers(n, dims, inputPoints, W, k, z, 0.);
    /*
    __m256 arr[2] = {};
    arr[0] = _mm256_set1_ps(12.);
    arr[1] = _mm256_set1_ps(3.);
    arr[0] = _mm256_add_ps(arr[0], arr[0]);
    float* arr_std = (float*)&arr[0];
    for (int i = 0; i < 8; i++)
        cout << arr_std[i] << "    ";
    cout << endl;
    */
   free(inputPoints);
}



vector<vector<float>> SeqWeightedOutliers(int n, int dims, float * pts, int W[], int k, int z, float alpha)
{
    int attempt = 0;

    float (*P)[n][dims] = (float (*)[n][dims]) pts;
    
    // generate matrix with all distances squared
    float all_dist_squared[n][n];
    float* last = (float*)aligned_alloc(32, 8 * sizeof(float));

    for (int i = 0; i < n; i++)
    {
        __m256 current_pt_arr[dims] = {};    // each element will contain 8 repetition of the same value of dimension
        for (int dim = 0; dim < dims; dim++)
            current_pt_arr[dim] = _mm256_set1_ps((*P)[i][dim]);
        
        __m256 squaredSum_8pts;   // start with 8 zeros
        int j = 0;
        while (j+8 < n)
        {
            for (int dim = 0; dim < dims; dim++)
            {
                __m256 dimL_8pts = _mm256_set_ps((*P)[j][dim], (*P)[j+1][dim], (*P)[j+2][dim], (*P)[j+3][dim], (*P)[j+4][dim], (*P)[j+5][dim], (*P)[j+6][dim], (*P)[j+7][dim]);

                squaredSum_8pts = _mm256_sub_ps(dimL_8pts, current_pt_arr[dim]);
                squaredSum_8pts = _mm256_add_ps(squaredSum_8pts, squaredSum_8pts);
                squaredSum_8pts = _mm256_mul_ps(squaredSum_8pts, squaredSum_8pts);
            }
            float* squaredSum = (float*)&squaredSum_8pts;   // retrive result
            for (int ind = 0; ind < 8; ind++)  // reverse read because array is inverted
            {
                all_dist_squared[i][j+ind] = squaredSum[8-ind];
                cout << squaredSum[8-ind] << "   ";
            }
            cout << endl << endl;
            j += 8;
        }

        // add last poins of array
        int remaining = (n-1)-(j-8);
        j -= 8;
        if (remaining != 0)
        {
            for (int dim = 0; dim < 8; dim++)
            {
                for (int ind = 0; ind < dims; ind++)
                {
                    if (ind + j < n)
                        last[ind] = (*P)[j+ind][dim];
                    else
                        last[ind] = 0.;
                }
                __m256 dimL_lastPts = _mm256_load_ps(last);
                squaredSum_8pts = _mm256_sub_ps(dimL_lastPts, current_pt_arr[dim]);
                squaredSum_8pts = _mm256_add_ps(squaredSum_8pts, squaredSum_8pts);
                squaredSum_8pts = _mm256_mul_ps(squaredSum_8pts, squaredSum_8pts);
            }
            float* squaredSum = (float*)&squaredSum_8pts;
            for (int ind = 0; ind < remaining; ind++)
                all_dist_squared[i][j+ind] = squaredSum[remaining-ind];
        }
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            cout << all_dist_squared[i][j] << "; ";
        }
        cout << "\b" << endl;
    }
    free(last);

    vector<vector<float>> temp;
    return temp;
    
    
    
}