#include <iostream>
#include <fstream>
#include "grib_sections.hpp"

using namespace std;

void getSection0(ifstream &file, ofstream &fout, char *buffer, GribData &data)
{
    fout << "-------------Section 0-------------" << endl;

    while (file.read(buffer, 4))
        if (buffer[0] == 'G' && buffer[1] == 'R' && buffer[2] == 'I' && buffer[3] == 'B')
            break;

    streampos section0_start = file.tellg(); // Remember the position after "GRIB"

    file.read(buffer, 4);
    data.length_of_grib_file = (unsigned char)buffer[0] << 16 |
                               (unsigned char)buffer[1] << 8 |
                               (unsigned char)buffer[2];
    int version_of_grib = (unsigned char)buffer[3];

    fout << "Length of file: " << data.length_of_grib_file << endl;
    fout << "GRIB version: " << version_of_grib << endl;
}

void getSection1(ifstream &file, ofstream &fout, char *buffer, GribData &data)
{
    fout << "\n-------------Section 1-------------" << endl;
    file.read(buffer, 4);
    data.length_of_PDS = (unsigned char)buffer[0] << 16 |
                         (unsigned char)buffer[1] << 8 |
                         (unsigned char)buffer[2];

    fout << "Length of Section: " << data.length_of_PDS << endl;

    file.read(buffer, 4);
    data.hasGDS = (buffer[3] >> 7) & 1;
    data.hasBMS = (buffer[3] >> 6) & 1;

    fout << "GDS: " << (data.hasGDS ? "Included" : "Omitted") << endl;
    fout << "BMS: " << (data.hasBMS ? "Included" : "Omitted") << endl;

    file.seekg(4, ios::cur);
    char datetime[5];
    file.read(datetime, 5);
    int year_of_century = (unsigned char)datetime[0];
    int month = (unsigned char)datetime[1];
    int day = (unsigned char)datetime[2];
    int hour = (unsigned char)datetime[3];
    int minute = (unsigned char)datetime[4];

    file.seekg(7, ios::cur);
    unsigned char century_byte;
    file.read(reinterpret_cast<char *>(&century_byte), 1);
    int full_year = (century_byte - 1) * 100 + year_of_century;

    fout << "Time of forecast: " << full_year << "-"
         << (month < 10 ? "0" : "") << month << "-"
         << (day < 10 ? "0" : "") << day << " "
         << (hour < 10 ? "0" : "") << hour << ":"
         << (minute < 10 ? "0" : "") << minute << endl;

    file.seekg(1, ios::cur);
    file.read(buffer, 2);
    int scale_factor_D = ((buffer[0] & 0x7F) << 8) | (unsigned char)buffer[1];
    if (buffer[0] & 0x80)
        scale_factor_D = -scale_factor_D;
    fout << "Scale factor D: " << scale_factor_D << endl;
}

void getSection2(ifstream &file, ofstream &fout, char *buffer, GribData &data)
{
    fout << "\n-------------Section 2-------------" << endl;

    data.section2_start = file.tellg(); // Position at the beginning of Section 2
    file.read(buffer, 3);
    data.length_of_GDS = (unsigned char)buffer[0] << 16 |
                         (unsigned char)buffer[1] << 8 |
                         (unsigned char)buffer[2];
    fout << "Length of Section: " << data.length_of_GDS << endl;

    file.seekg(7, ios::cur);

    auto decode_coord = [](unsigned char b0, unsigned char b1, unsigned char b2) -> double
    {
        int raw = ((b0 & 0x7F) << 16) | (b1 << 8) | b2;
        bool is_negative = b0 & 0x80;
        double value = raw / 1000.0;
        return is_negative ? -value : value;
    };

    // B11–13: Latitude start
    file.read(buffer, 3);
    double lat_start = decode_coord(buffer[0], buffer[1], buffer[2]);

    // B14–16: Longitude start
    file.read(buffer, 3);
    double lon_start = decode_coord(buffer[0], buffer[1], buffer[2]);

    // B17: flags — skiping this byte
    file.seekg(1, ios::cur);

    // B18–20: Latitude end
    file.read(buffer, 3);
    double lat_end = decode_coord(buffer[0], buffer[1], buffer[2]);

    // B21–23: Longitude end
    file.read(buffer, 3);
    double lon_end = decode_coord(buffer[0], buffer[1], buffer[2]);

    fout << "Latitude start: " << lat_start << endl;
    fout << "Longitude start: " << lon_start << endl;
    fout << "Latitude end: " << lat_end << endl;
    fout << "Longitude end: " << lon_end << endl;

    // B29–B33: 5 additional bytes
    file.seekg(data.section2_start + static_cast<streamoff>(28));
    char extra[5];
    file.read(extra, 5);

    fout << "Bytes 29-33 (decimal): ";
    for (int i = 0; i < 5; ++i)
        fout << (unsigned int)(unsigned char)extra[i] << " ";
    fout << endl;

    // Move to the next section
    file.seekg(data.section2_start + static_cast<streamoff>(data.length_of_GDS));
}

void getSection3(ifstream &file, ofstream &fout, char *buffer, GribData &data)
{
    if (data.hasBMS)
    {
        fout << "\n-------------Section 3-------------" << endl;
        streampos section3_start = file.tellg();
        file.read(buffer, 3);
        int length_of_BMS = (unsigned char)buffer[0] << 16 |
                            (unsigned char)buffer[1] << 8 |
                            (unsigned char)buffer[2];
        fout << "Length of Section: " << length_of_BMS << endl;

        // Move to the next section
        file.seekg(section3_start + static_cast<streamoff>(length_of_BMS));
    }
}

void getSection4(ifstream &file, ofstream &fout, char *buffer, GribData &data)
{
    fout << "\n-------------Section 4-------------" << endl;
    file.read(buffer, 3);
    data.length_of_BDS = (unsigned char)buffer[0] << 16 |
                         (unsigned char)buffer[1] << 8 |
                         (unsigned char)buffer[2];
    fout << "Length of Section: " << data.length_of_BDS << endl;

    file.seekg(1, ios::cur); // Skip 1 byte
    file.read(buffer, 2);
    int scale_factor_E = ((buffer[0] & 0x7F) << 8) | (unsigned char)buffer[1];
    if (buffer[0] & 0x80)
        scale_factor_E = -scale_factor_E;
    fout << "Scale factor E: " << scale_factor_E << endl;

    union
    {
        float f;
        unsigned char b[4];
    } reference_val;

    file.read(reinterpret_cast<char *>(reference_val.b), 4);
    std::swap(reference_val.b[0], reference_val.b[3]);
    std::swap(reference_val.b[1], reference_val.b[2]);
    fout << "Reference value: " << reference_val.f << endl;

    unsigned char number_of_packed_bits;
    file.read(reinterpret_cast<char *>(&number_of_packed_bits), 1);
    fout << "Number of bits into which a datum point is packed: " << (int)number_of_packed_bits << endl;

    char data_bytes[5];
    file.read(data_bytes, 5);
    fout << "Bytes 12-16 (decimal): ";
    for (int i = 0; i < 5; ++i)
        fout << (unsigned int)(unsigned char)data_bytes[i] << " ";
    fout << endl;
}

void LengthOfFileCheck(ifstream &file, ofstream &fout, char *buffer, GribData &data)
{
    fout << endl
         << "-------------GRIB Length Check-------------" << endl;

    int total_section_length = 0;
    total_section_length += data.length_of_PDS;
    total_section_length += data.length_of_GDS;
    if (data.hasBMS) // If BMS is present, add its length
        total_section_length += static_cast<int>(
            (file.tellg() - data.section2_start) - static_cast<std::streamoff>(data.length_of_BDS));
    total_section_length += data.length_of_BDS;

    int length_from_GRIB = data.length_of_grib_file;

    fout << "Declared GRIB message length: " << length_from_GRIB << endl;
    fout << "Sum of section lengths:       " << total_section_length << endl;
}

void getSections(ifstream &file, ofstream &fout, char *buffer, GribData &data)
{
    getSection0(file, fout, buffer, data); // Section 0
    getSection1(file, fout, buffer, data); //  (PDS)
    getSection2(file, fout, buffer, data); //  (GDS)
    getSection3(file, fout, buffer, data); //  (BMS)
    getSection4(file, fout, buffer, data); //  (BDS)
    LengthOfFileCheck(file, fout, buffer, data);
}