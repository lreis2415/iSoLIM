/*!
 * @brief Environmental dataset.
 * @todo 
 * @version 1.0
 * @revision  17-11-21 zhanglei - initial version
 *             17-11-27 lj       - code revision and format
 */
#ifndef ENVDATASET_HPP_
#define ENVDATASET_HPP_
#include <vector>
#include <algorithm>
#include <filesystem>
#include "preprocess.h"
#include "EnvLayer.h"
#include "EnvUnit.h"

// using namespace std; // Avoid this usage, instead of specific functions. 2019/08/06 ZHULJ
using std::transform;

namespace solim {
class EnvDataset {
public:
    BaseIO* LayerRef;
    vector<EnvLayer *> Layers;
	vector <string> LayerNames;
    // BasicSetting* Setting;
    double CellSize;
    double CellSizeY;
    double XMin;
    double XMax;
    double YMin;
    double YMax;
    double NoDataValue;
    int XSize; // cols of current partition
    int YSize; // rows of current partition
    int XStart; // global coordinate of current partition's first cell's position
    int YStart;
    long TotalX; // global size
    long TotalY;
    int CalcArea;

public:
    EnvDataset();
	
    EnvDataset(vector<string> &envLayerFilenames, vector<string> &datatypes, vector<string>& layernames, double ramEfficent = 1);

    EnvDataset(vector<string> &envLayerFilenames, vector<string> &datatypes);

    ~EnvDataset();

    void AddLayer(EnvLayer* layer) {
        Layers.push_back(layer);
    }

    void RemoveAllLayers();

    void ReadinLayers(vector<string> &envLayerFilenames, const vector<string> &datatypes,vector<string>& layernames, double ramEfficient = 1);

	EnvLayer* getDEM();

    EnvUnit* GetEnvUnit(const int row, const int col);

    EnvUnit* GetEnvUnit(const double x, const double y);

    EnvUnit* GetEnvUnit(Location *loc);
	
	void Writeout(string filename, float*EnvData, int blockRank=0);

private:
    EnvDataset(const EnvDataset&);

    EnvDataset& operator=(const EnvDataset&);
};
}

#endif
