/*!
 * @brief IO module of raster layer data.
 * @version 1.0
 * @revision  17-11-21 zhanglei - initial version
 *            17-11-27 lj       - code revision and format
 */
#ifndef IO_HPP_
#define IO_HPP_
#include <string>
#include <vector>

#include "EnvUnit.h"
#include "EnvDataset.h"
#include "EnvLayer.h"

// using namespace std; // Avoid this usage, instead of specific functions. 2019/08/06 ZHULJ
using std::vector;
using std::string;

namespace solim {
bool ReadinEnvLayers(EnvDataset* eds, vector<string> envLayerFilenames, vector<string> datatypes);

bool WriteoutRaster(EnvLayer* envLayer, string filename, string type, GDALDataset* srcDs);

vector<EnvUnit *> ReadTable(string filename,
                            EnvDataset* envDataset,
                            string targetVName = "None",
                            string idName = "None",
                            string xName = "None",
                            string yName = "None");
vector<EnvUnit *> ReadTable_geocoords(string filename,
                            EnvDataset* envDataset,
                            string targetVName = "None",
                            string idName = "None",
                            string lonName = "None",
                            string latName = "None");

bool WriteTable(string filename, vector<EnvUnit *> envUnit);

bool WriteCSV_SampleCredibility(string filename, vector<EnvUnit *> samples);
}

#endif
