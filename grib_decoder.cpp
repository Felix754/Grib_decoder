#include <iostream>
#include <fstream>
#include "grib_sections.hpp"
using namespace std;

int main(int argc, char *argv[])
{
    string file_name; //"20150310.00.W.dwa_griby.grib"
    string decoded_file_name;

    char buffer[4];
    GribData data;
    if (argc == 3)
    {
        file_name = argv[1];
        decoded_file_name = argv[2];
    }
    else
    {
        cout << "Invalid number of arguments(" << argc - 1 << " providet)" << ", switching to manual mode" << endl
             << "Usage: GRIB_decoder <input.grib> <output.txt>" << endl
             << endl
             << "Provide the path to the file for decoding with file type: ";

        cin >> file_name;

        cout << endl
             << "Provide the name of the decoded file: ";

        cin >> decoded_file_name;
    }
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

    getSections(file, fout, buffer, data);

    file.close();
    fout.close();
    return 0;
}