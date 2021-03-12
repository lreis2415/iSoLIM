#ifndef PREPROCESS_HPP_
#define PREPROCESS_HPP_
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <gdal.h>
#include <gdalwarper.h>

#include "EnvLayer.h"
#include "EnvDataset.h"

// using namespace std; // Avoid this usage, instead of specific functions. 2019/08/06 ZHULJ
using std::ostringstream;

namespace solim {
enum FilterType {
    MEAN_FILTER
};

string ConvertToString(double value);

/*!
 * @brief 解析以c为分隔符的数据
 * @param str
 * @param c
 * @param tokens
 */
void ParseStr(const string& str, char c, vector<string>& tokens);

char* FindImageTypeGDAL(char* fileName);

void Stretch(EnvLayer* lyr, double smax, double smin, double max, double min);

vector<double>* Neighborhood(EnvLayer* lyr, int pos, int radius);

void Filter(EnvLayer* lyr, int radius, FilterType type = MEAN_FILTER);

bool resample(string srcFile, string dstProjectionRefFile, string destFile);
}

#endif
