#include "EnvLayer.h"

#include "ogrsf_frmts.h"  // for ogr
#include "gdal_alg.h"     // for GDALPolygonize
#include "cpl_conv.h"     // for CPLMalloc()

// using namespace std; // Avoid this usage, instead of specific functions. 2019/08/06 ZHULJ
using std::string;

namespace solim {
EnvLayer::EnvLayer()
    : LayerId(-1), DataType(CONTINUOUS), baseRef(nullptr), Data_Max(NODATA),
	Data_Min(NODATA), Data_Range(NODATA), NoDataValue(NODATA),
	XSize(-1), YSize(-1) {
    for (int i = 0; i < 6; i++) GeoTransform[i] = NODATA;
}

EnvLayer::EnvLayer(const int layerId, string layerName, const string& filename, const DataTypeEnum dataType):
    LayerId(layerId), LayerName(layerName), DataType(dataType) {
	baseRef = new BaseIO(filename);
	baseRef->blockNull();
	XSize = baseRef->getBlockX();	// number of columns
	YSize = baseRef->getBlockY();
	BlockSize = baseRef->getBlockSize();
	EnvData = new float[XSize * YSize];
	for (int i = 0; i < XSize * YSize; ++i) {
		EnvData[i] = 0.0;
	}
    upperBorder = nullptr;
    lowerBorder = nullptr;
    if (BlockSize > 1) {
		upperBorder = new float[YSize];
		lowerBorder = new float[YSize];
		for (int i = 0; i < YSize; ++i) {
			upperBorder[i] = 0.0;
			lowerBorder[i] = 0.0;
		}
	}
	Data_Min = baseRef->getDataMin();
	Data_Max = baseRef->getDataMax();
	Data_Range = baseRef->getDataRange();
	CellSize = baseRef->getCellSize();
	NoDataValue = baseRef->getNoDataValue();
}

EnvLayer::EnvLayer(const int layerId, string layerName, const string& filename, const DataTypeEnum dataType, BaseIO *ref) :
	LayerId(layerId), LayerName(layerName), DataType(dataType) {
	baseRef = new BaseIO(filename);
	baseRef->blockCopy(ref);
	XSize = baseRef->getBlockX();	// number of columns
	YSize = baseRef->getBlockY();
	BlockSize = baseRef->getBlockSize();
	EnvData = new float[XSize * YSize];
	for (int i = 0; i < XSize * YSize; ++i) {
		EnvData[i] = 0.0;
	}
    upperBorder = nullptr;
    lowerBorder = nullptr;
    if (BlockSize > 1) {
		upperBorder = new float[YSize];
		lowerBorder = new float[YSize];
		for (int i = 0; i < YSize; ++i) {
			upperBorder[i] = 0.0;
			lowerBorder[i] = 0.0;
		}
    }
	Data_Min = baseRef->getDataMin();
	Data_Max = baseRef->getDataMax();
	Data_Range = baseRef->getDataRange();
	CellSize = baseRef->getCellSize();
	NoDataValue = baseRef->getNoDataValue();
}

EnvLayer::~EnvLayer(void) {
    if(EnvData) delete[]EnvData;
    //if(MembershipData) delete[]MembershipData;
    if(upperBorder != nullptr)
        delete[]upperBorder;
    if(lowerBorder != nullptr)
        delete[]lowerBorder;
}

void EnvLayer::WriteoutMembership(const string& filename, int blockRank) {
	int localx = 0;
	int localy = 0;
	int globalx, globaly;
	baseRef->localToGlobal(blockRank, localx, localy, globalx, globaly);
	int nx = baseRef->getBlockX();
	int ny = baseRef->getBlockY();
	if (blockRank == (BlockSize - 1)) {
		ny = baseRef->getYSize() - blockRank * baseRef->getBlockY();
	}
	if (blockRank == 0) baseRef->writeInit();
	baseRef->write(globalx, globaly, ny, nx, MembershipData, filename);
}

void EnvLayer::Writeout(string filename, int blockRank) {
	int localx = 0;
	int localy = 0;
	int globalx, globaly;
	baseRef->localToGlobal(blockRank, localx, localy, globalx, globaly);
	int nx = baseRef->getBlockX();
	int ny = baseRef->getBlockY();
	if (blockRank == (BlockSize - 1)) {
		ny = baseRef->getYSize() - blockRank * baseRef->getBlockY();
	}
	if (blockRank == 0) baseRef->writeInit();
	baseRef->write(globalx, globaly, ny, nx, EnvData, filename);
}

void EnvLayer::Readin(int xstart, int ystart, int ny, int nx) {
	baseRef->read(xstart, ystart, ny, nx, EnvData);
}

int EnvLayer::GetYSizeByBlock(int blockRank) {
	if (blockRank == (BlockSize - 1)) {
		return baseRef->getYSize() - blockRank * baseRef->getBlockY();
	}
	else {
		return YSize;
	}
}


void EnvLayer::ReadByBlock(int blockRank) {
	int localx = 0;
	int localy = 0;
	int globalx, globaly;
	baseRef->localToGlobal(blockRank, localx, localy, globalx, globaly);
	int nx = baseRef->getBlockX();
	int ny = baseRef->getBlockY();
	if (blockRank == (BlockSize - 1)) {
		ny = baseRef->getYSize() - blockRank * baseRef->getBlockY();
	}
	baseRef->read(globalx, globaly, ny, nx, EnvData);
	if (BlockSize > 1) {
        if (blockRank > 0) {
            //baseRef->read(globalx, globaly-1, 1, nx, upperBorder);
        }
        if (blockRank < BlockSize - 1) {
            //baseRef->read(globalx, globaly+ny, 1, nx, lowerBorder);
        }
	}
}
};
