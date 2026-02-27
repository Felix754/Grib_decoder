# Grib Decoder C++

![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Format](https://img.shields.io/badge/Format-GRIB-blueviolet)

This repository was created as a working representation of a decoder based on bitwise shifting in C++.  
It is also designed for future expansion due to the modularity of the code.  
The decoder is based on the following documentation: [https://www.nco.ncep.noaa.gov/pmb/docs/on388/](https://www.nco.ncep.noaa.gov/pmb/docs/on388/)

The table below represents the elements covered by this decoder.

### Decoder coverage summary

| Section         | Octets    | Status |
|----------------|-----------|--------|
| Section 0       | 1–8       | 🟩     |
| Section 1       | 1–3       | 🟩     |
| Section 1       | 4–7       | 🟥     |
| Section 1       | 8         | 🟩     |
| Section 1       | 9–12      | 🟥     |
| Section 1       | 13–17     | 🟩     |
| Section 1       | 18–24     | 🟥     |
| Section 1       | 25        | 🟩     |
| Section 1       | 26        | 🟥     |
| Section 1       | 27–28     | 🟩     |
| Section 2       | 1–3       | 🟩     |
| Section 2       | 4–10      | 🟥     |
| Section 2       | 11–16     | 🟩     |
| Section 2       | 17        | 🟥     |
| Section 2       | 19–23     | 🟩     |
| Section 2       | 24–28     | 🟥     |
| Section 2       | 29–33     | 🟩     |
| Section 3       | 1–3       | 🟩     |
| Section 3       | 4+        | 🟥     |
| Section 4 (BDS) | 1–3       | 🟩     |
| Section 4 (BDS) | 4         | 🟥     |
| Section 4 (BDS) | 5–11      | 🟩     |
| Section 4 (BDS) | 12–16     | 🟩     |
| Section 5       | 1 2 3 4   | 🟩     |

#### Legend

| Status | Description            |
|--------|------------------------|
| 🟩     | Implemented            |
| 🟥     | skipped / not implemented  |
