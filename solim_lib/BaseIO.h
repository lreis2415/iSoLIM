/* 
BaseIO: base structure for block io
@author: zhao fh
@date: 2021/1/15
*/
#ifndef BASEIO_H
#define BASEIO_H

#pragma once
#include <gdal.h>
#include <gdal_priv.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <Windows.h>
//#define MCW MPI_COMM_WORLD
#define VERY_SMALL 0.0001
#define NODATA -9999
#define FN_LEN 400

using std::string;
using std::cout;
using std::endl;

enum FileDataType {
	SHORT_TYPE,
	LONG_TYPE,
	FLOAT_TYPE
};

const double PI = 3.14159265359;
const double elipa = 6378137.000;
const double elipb = 6356752.314;
const double boa = elipb / elipa;

class BaseIO {
private:
	//int size, rank;				// MPI parameters
    GDALDataset *ds;            // gdal file handle
	bool isFileInititialized;
    GDALRasterBand *band;
	string fileName;
	int xSize;
	int ySize;
	double cellSize;	// raster location info: cell size of raster layer
	double noDataValue;
	double dx, dy;
	GDALDataType eBDataType;
	FileDataType fileDataType;
	double dataMax;
	double dataMin;
	double dataRange;
	double adfGeoTransform[6];
	double xllCenter;
	double yllCenter;
	double xLeftEdge;
	double yTopEdge;
	double dxA, dyA;
    bool openSuccess;

	//int parallelX, parallelY;
	int blockX, blockY;
	int blockSize;	// the number of block parts for block processing
	int blockRows;	// the row number of one block
	bool blockIsInitialized = FALSE;
	OGRSpatialReference srs;
	int isGeographic;

	// 3dr read, @zfh, 7/27/2020
	FILE *threeDRfp;
	bool is3dr;
	int inCurLoc;
	bool unexpectedFieldFlag;
	int NumberOfRecords;
    char fileType[FN_LEN];
    char gridUnits[FN_LEN];
	double dataMean;
	double dataStd;
	int firstDataByte;
	int dataEndLoc;

public:
	BaseIO(string filename);
    BaseIO(BaseIO *lyr);
	~BaseIO();
	//void parallelInit(MPI_Datatype MPIt);
    void blockInit(double divide=1);
    void blockNull();
	void blockCopy(BaseIO *ref);
	void read(long xStart, long yStart, long numRows, long numCols, float *dest);
	void write(long xStart, long yStart, long numRows, long numCols, float *source, string writeFilename="");
	
	void geoToGlobalXY(double geoX, double geoY, int &globalX, int &globalY);
	void globalXYToGeo(long globalX, long globalY, double &geoX, double &geoY);

	bool isInPartition(int x, int y);
	
	void geoToLength(double dlon, double dlat, double lat, double *xyc);
	bool globalToLocal(int blockRank, int globalX, int globalY, int &localX, int &localY);
//	void localToGlobal(int localX, int localY, int &globalX, int &globalY);
	void localToGlobal(int block_rank, int localX, int localY, int &globalX, int &globalY);
	bool compareIO(BaseIO *layer);
	float getValue(long col, long row);

	int getXSize() { return xSize; }
	int getYSize() { return ySize; }
	double getCellSize() { return cellSize; }
	double getNoDataValue() { return noDataValue; }
	double getDx() { return dx; }
	double getDy() { return dy; }
	double getDxA() { return dxA; }
	double getDyA() { return dyA; }
	double getXMin() { return xLeftEdge; }
	double getXMax() { 
		if (is3dr) return xLeftEdge + cellSize * xSize;
		else return xLeftEdge + dx * xSize; 
	}
	double getYMax() { return yTopEdge; }
	double getYMin() {
		if (is3dr) return yTopEdge - cellSize * ySize;
		else return yTopEdge - dy * ySize;  
	}
	double getDataMax() { return dataMax; }
	double getDataMin() { return dataMin; }
	double getDataRange() { return dataRange; }

	//int getParallelX() { return parallelX; }
	//int getParallelY() { return parallelY; }
	int getBlockX() { return blockX; }
	int getBlockY() { return blockY; }
	int getBlockSize() { return blockSize; }
	int getBlockRows() { return blockRows; }
    const char*  getProjection() { return ds->GetProjectionRef(); }

    string getFilename() { return fileName; }
    void setFileName(string name) { fileName = name; }
	void setNodataValue(double nodata) { noDataValue = nodata; }
	void writeInit() { isFileInititialized = false; }
    bool isOpened() {return openSuccess;}
    bool resample(BaseIO *dstProjectionRefLyr, string destFile);
};

#endif //BASEIO_H
