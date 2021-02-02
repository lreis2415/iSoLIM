# iSoLIM
intelligent SoLIM
## Overview
SoLIM (Soil Land Inference Model) is a new technology for soil mapping based on recent developments in geographic information science (GISc), artificial intelligence (AI), and information representation theory. This is the standalone version of SoLIM Software. The main difference between this version and the [older version](https://github.com/lreis2415/SoLIM-Solutions) is the support for block reading of big raster data. It is designed to solve the "out of memory" problem that comes up in the old version when big data is used as input layer.

## Development Framework
This software is developed based on Qt Framework, with the C++ language.
### File Description
*solim-lib-forqt.* - This file includes all the computational codes that can conduct the actual SoLIM functions.
*Group of file with the same filename and different suffix *.io, *.cpp, *.h - the codes for one interface of the software
  */*.io - The outlook of interface
  */*.h, *.cpp - The interactive functions of the interface

## Current Functions
2021/02/01
1) Read and write of raster data (in blocks).
2) Knowledge mining (creating prototypes that can be used to implement spatial inference) from experts, samples, and maps.
3) Basic SoLIM inference.

## Future Development
1) Add sampling functions
2) Enable environment variables preprocessing

@Coder: Fanghe Zhao
