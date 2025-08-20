#include <iostream>
#include <fstream>

using namespace std;

struct GribData
{
    // Section 0
    int length_of_grib_file = 0;

    // Section 1
    bool hasGDS = false;
    bool hasBMS = false;
    int length_of_PDS = 0;

    // Section 2
    streampos section2_start{};
    int length_of_GDS = 0;

    // Section 4
    int length_of_BDS = 0;
};

void getSections(ifstream &file, ofstream &fout, char *buffer, GribData &data);