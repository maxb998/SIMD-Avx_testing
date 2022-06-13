#include "kCentersOutliers.hpp"

using namespace std;

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
    

    KCentersOutliers kc;
    kc.loadDataset(argv[1]);
    kc.genUnitWeights();
    kc.SeqWeightedOutliers(k,z,0.);
    //kc.~KCentersOutliers();

    
    

    // find shape of 2D array
    

    
    /*
    int W[n];
    for (int i = 0; i < n; i++)
        W[i] = 1;*/
    
}