#include <iostream>
#include <fstream>
#include "grib_sections.hpp"
using namespace std;

// FIX: Delete output file before commiting
// SUGGESTION: Make header for structures and functions

int main()
{
    string file_name; //"20150310.00.W.dwa_griby.grib"
    string decoded_file_name;

    char buffer[4];
    GribData data;

    cout << "Simple GRIB file decoder" << endl
         << "Provide the path to the file for decoding with file type: ";
    cin >> file_name;

    cout << endl
         << "Provide the name of the decoded file: ";

    cin >> decoded_file_name;
    decoded_file_name += ".txt";

    ifstream file(file_name, ios::in | ios::binary);

    if (!file)
    {
        ofstream ferr(decoded_file_name);
        ferr << "Error opening the file: " << file_name << endl;
        return 1;
    }

    ofstream fout(decoded_file_name);
    if (!fout)
    {
        cout << "An error occurred with decoded file" << endl;
        return 1;
    }

    fout << "File Opened: " << file_name << endl;

    getSection0(file, fout, buffer, data); // Section 0
    getSection1(file, fout, buffer, data); //  (PDS)
    getSection2(file, fout, buffer, data); //  (GDS)
    getSection3(file, fout, buffer, data); //  (BMS)
    getSection4(file, fout, buffer, data); //  (BDS)

    // GRIB message length check
    LengthOfFileCheck(file, fout, buffer, data);
    
    file.close();
    fout.close();
    return 0;
}