/*!
 * @brief Environmental layer
 * @todo
 * @version 1.0
 * @revision  17-11-21 zhanglei - initial version
 *            17-11-27 lj       - code revision and format
 */
#ifndef ENVLAYER_HPP_
#define ENVLAYER_HPP_

#include <string>
#include <vector>
#include <gdal_priv.h>
#include "BaseIO.h"

#include "DataTypeEnum.h"

// using namespace std; // Avoid this usage, instead of specific functions. 2019/08/06 ZHULJ
using std::string;
using std::vector;

namespace solim {
class EnvLayer {
public:
    int LayerId;
    string LayerName;
	BaseIO *baseRef;
	float* EnvData;
	float* upperBorder;
	float* lowerBorder;
	int borderRowNum;
    // Adding MembershipData to use save membershipValue
    float* MembershipData;
    DataTypeEnum DataType;
    double Data_Max;
    double Data_Min;
    double Data_Mean;
    double Data_StdDev;
    double Data_Range;
    double NoDataValue;
    double GeoTransform[6];
	double CellSize;
    int XSize;
    int YSize;
	int BlockSize;

public:
    EnvLayer();

	EnvLayer(const int layerId, string layerName, const string& filename, const DataTypeEnum dataType, BaseIO *ref);

    EnvLayer(const int layerId, string layerName, const string& filename, const DataTypeEnum dataType);

    ~EnvLayer();

    void WriteoutMembership(const string& filename, int blockRank = 0);

	void Writeout(string filename, int blockRank = 0);
	void Readin(int xstart, int ystart, int ny, int nx);
	void ReadByBlock(int blockRank);
    float GetValue(int col, int row);
	int GetYSizeByBlock(int blockRank);
};
}

#endif
