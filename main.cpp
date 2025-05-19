#include <iostream>
#include <fstream>
using namespace std;

string fileName = "20150310.00.W.dwa_griby.grib";
fstream file(fileName, ios::in | ios::binary);

int main()
{
    char buffer[4];

    if (!file)
    {
        ofstream ferr("decoder_output.txt");
        ferr << "Error opening the file: " << fileName << endl;
        return 1;
    }

    ofstream fout("decoder_output.txt");
    if (!fout)
    {
        return 1;
    }

    fout << "File Opened: " << fileName << endl;

    // ------------------ Section 0 ------------------
    fout << "-------------Section 0-------------" << endl;

    while (file.read(buffer, 4))
        if (buffer[0] == 'G' && buffer[1] == 'R' && buffer[2] == 'I' && buffer[3] == 'B')
            break;

    streampos section0_start = file.tellg(); // Remember the position after "GRIB"

    file.read(buffer, 4);
    int length_of_grib_file = (unsigned char)buffer[0] << 16 |
                              (unsigned char)buffer[1] << 8 |
                              (unsigned char)buffer[2];
    int version_of_grib = (unsigned char)buffer[3];

    fout << "Length of file: " << length_of_grib_file << endl;
    fout << "GRIB version: " << version_of_grib << endl;

    // ------------------ Section 1 (PDS) ------------------
    fout << "\n-------------Section 1-------------" << endl;
    file.read(buffer, 4);
    int length_of_PDS = (unsigned char)buffer[0] << 16 |
                        (unsigned char)buffer[1] << 8 |
                        (unsigned char)buffer[2];

    fout << "Length of Section: " << length_of_PDS << endl;

    file.read(buffer, 4);
    bool hasGDS = (buffer[3] >> 7) & 1;
    bool hasBMS = (buffer[3] >> 6) & 1;

    fout << "GDS: " << (hasGDS ? "Included" : "Omitted") << endl;
    fout << "BMS: " << (hasBMS ? "Included" : "Omitted") << endl;

    file.seekg(4, ios::cur); // Skip next 4 bytes
    char datetime[5];
    file.read(datetime, 5);
    int year_of_century = (unsigned char)datetime[0];
    int month = (unsigned char)datetime[1];
    int day = (unsigned char)datetime[2];
    int hour = (unsigned char)datetime[3];
    int minute = (unsigned char)datetime[4];

    file.seekg(7, ios::cur); // Skip 7 bytes
    unsigned char century_byte;
    file.read(reinterpret_cast<char *>(&century_byte), 1);
    int full_year = (century_byte - 1) * 100 + year_of_century;

    fout << "Time of forecast: " << full_year << "-"
         << (month < 10 ? "0" : "") << month << "-"
         << (day < 10 ? "0" : "") << day << " "
         << (hour < 10 ? "0" : "") << hour << ":"
         << (minute < 10 ? "0" : "") << minute << endl;

    file.seekg(1, ios::cur); // Skip 1 byte
    file.read(buffer, 2);
    int scale_factor_D = ((buffer[0] & 0x7F) << 8) | (unsigned char)buffer[1];
    if (buffer[0] & 0x80)
        scale_factor_D = -scale_factor_D;
    fout << "Scale factor D: " << scale_factor_D << endl;

    // ------------------ Section 2 (GDS) ------------------
    fout << "\n-------------Section 2-------------" << endl;

    streampos section2_start = file.tellg(); // Position at the beginning of Section 2
    file.read(buffer, 3);
    int length_of_GDS = (unsigned char)buffer[0] << 16 |
                        (unsigned char)buffer[1] << 8 |
                        (unsigned char)buffer[2];
    fout << "Length of Section: " << length_of_GDS << endl;

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

    // B17: flags — пропускаємо
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

    // B29–B33: 5 додаткових байтів
    file.seekg(section2_start + static_cast<streamoff>(28));
    char extra[5];
    file.read(extra, 5);

    fout << "Bytes 29-33 (decimal): ";
    for (int i = 0; i < 5; ++i)
        fout << (unsigned int)(unsigned char)extra[i] << " ";
    fout << endl;

    // Move to the next section
    file.seekg(section2_start + static_cast<streamoff>(length_of_GDS));

    // ------------------ Section 3 (BMS) ------------------
    if (hasBMS)
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

    // ------------------ Section 4 (BDS) ------------------
    fout << "\n-------------Section 4-------------" << endl;
    file.read(buffer, 3);
    int length_of_BDS = (unsigned char)buffer[0] << 16 |
                        (unsigned char)buffer[1] << 8 |
                        (unsigned char)buffer[2];
    fout << "Length of Section: " << length_of_BDS << endl;

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
    } ref;

    file.read(reinterpret_cast<char *>(ref.b), 4);
    std::swap(ref.b[0], ref.b[3]);
    std::swap(ref.b[1], ref.b[2]);
    fout << "Reference value: " << ref.f << endl;

    unsigned char number_of_packed_bits;
    file.read(reinterpret_cast<char *>(&number_of_packed_bits), 1);
    fout << "Number of bits into which a datum point is packed: " << (int)number_of_packed_bits << endl;

    char data_bytes[5];
    file.read(data_bytes, 5);
    fout << "Bytes 12-16 (decimal): ";
    for (int i = 0; i < 5; ++i)
        fout << (unsigned int)(unsigned char)data_bytes[i] << " ";
    fout << endl;

    // ------------------ GRIB message length check ------------------
    fout << endl
         << "-------------GRIB Length Check-------------" << endl;

    int total_section_length = 0;
    total_section_length += length_of_PDS;
    total_section_length += length_of_GDS;
    if (hasBMS) // If BMS is present, add its length
        total_section_length += static_cast<int>(
            (file.tellg() - section2_start) - static_cast<std::streamoff>(length_of_BDS));
    total_section_length += length_of_BDS;

    int length_from_GRIB = length_of_grib_file;

    fout << "Declared GRIB message length: " << length_from_GRIB << endl;
    fout << "Sum of section lengths:       " << total_section_length << endl;

    fout.close();
    return 0;
}