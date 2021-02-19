#ifndef SOLIMLIBFORQT_H
#define SOLIMLIBFORQT_H

#include "QProgressBar"
#include "QApplication"
#include "QException"
#include "QDebug"
#include "solim-lib-forQt_global.h"
#include <algorithm>
#include <numeric>
#include <gdal.h>
#include <ogr_spatialref.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <Windows.h>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>  // for ogr
#include <gdal_alg.h>     // for GDALPolygonize
#include <cpl_conv.h>     // for CPLMalloc()
#include <omp.h>
#include "third_party\tinyxml.h"

#define VERY_SMALL 0.0001
#define NODATA -9999
#define FN_LEN 400
#define MAXLN_LAYERS 128
#define MAXLN 4096
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;
using std::invalid_argument;
using std::to_string;
using std::transform;
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
    GDALDatasetH fh;            // gdal file handle
    bool isFileInititialized;
    GDALDriverH hDriver;
    GDALRasterBandH bandh;
    string fileName;
    int xSize;
    int ySize;
    double cellSize;	// raster location info: cell size of raster layer
    double noDataValue;
    double dx, dy;
    FileDataType fileDataType;
    double dataMax;
    double dataMin;
    double dataRange;
    double adfGeoTransform[6];
    double dlon, dlat;
    double xllCenter;
    double yllCenter;
    double xLeftEdge;
    double yTopEdge;
    double dxA, dyA;

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
    BaseIO(string filename, FileDataType newFileDataType = FLOAT_TYPE);
    ~BaseIO();
    //void parallelInit(MPI_Datatype MPIt);
    string getFilename() { return fileName; }
    void blockInit(double divide=1);
    void blockCopy(BaseIO *ref);
    void read(long xStart, long yStart, long numRows, long numCols, float *dest);
    void write(long xStart, long yStart, long numRows, long numCols, float *source);

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

    void setFileName(string name) { fileName = name; }
    void setNodataValue(double nodata) { noDataValue = nodata; }
    bool openSuccess;
};

namespace solim {

    struct Location {
    public:
        Location() : Row(-1), Col(-1), X(-9999.), Y(-9999.) {
        }

        Location(const int row, const int col)
            : Row(row), Col(col), X(-9999.), Y(-9999.) {
        }

        Location(const int row, const int col, const double x, const double y)
            : Row(row), Col(col), X(x), Y(y) {
        }

        ~Location() {
        }

    public:
        int Row;  // Row number of current cell
        int Col;  // Col number of current cell
        double X; // X coordinate of current cell at upper left
        double Y; // Y coordinate of current cell at upper left
    };

    enum DataTypeEnum {
        CATEGORICAL,
        CONTINUOUS,
        OTHER
    };
    static const char* DataTypeEnum_str[]={"CATEGORICAL","CONTINUOUS","OTHER"};
    enum CurveTypeEnum {
        BELL_SHAPED,
        S_SHAPED,
        Z_SHAPED,
        ORDER_SHAPED,
        NAME_SHAPED
    };
    enum RuleType {
        RANGE_RULE,
        FREEHAND_RULE,
        POINT_RULE,
        WORD_RULE,
        ENUMERATED_RULE,
    };
    enum IntegrationMethod {
        MINIMUM,
        MEAN
    };
    enum ComputationMode {
        SINGLE_LAYER,
        MULTI_LAYER
    };
    enum PrototypeSource {
        SAMPLE,
        EXPERT,
        MAP,
        UNKNOWN
    };
    static const char* PrototypeSource_str[] = {"SAMPLE","EXPERT","MAP","UNKNOW"};
    static PrototypeSource getSourceFromString(string source_str) {
        for (int i = 0; i<source_str.length(); i++)
            source_str[i] = toupper(source_str[i]);

        if (source_str == "SAMPLE")
            return SAMPLE;
        else if(source_str=="EXPERT")
            return EXPERT;
        else if(source_str=="MAP")
            return MAP;
        else
            return UNKNOWN;
    }
    enum ExceptionType {
        OCCURRENCE,
        EXCLUSION
    };
    struct SoilProperty {
        string propertyName;
        double propertyValue;
        DataTypeEnum soilPropertyType;
    };
    static void ParseStr(string str, char c, vector<string>& tokens) {
        unsigned int posL = 0;
        unsigned int posR = 0;
        while (posR < str.length() - 1) {
            posR = str.find_first_of(c, posL);
            string sub = str.substr(posL, posR - posL);
            tokens.push_back(sub);
            posL = posR + 1;
        }
    }

    static DataTypeEnum getDatatypeFromString(string sDatatype) {
        for (int i = 0; i<sDatatype.length(); i++)
            sDatatype[i] = toupper(sDatatype[i]);

        if (sDatatype == "CATEGORICAL")
            return CATEGORICAL;
        else
            return CONTINUOUS;
    }
    static string getDatatypeInString(DataTypeEnum datatype) {
        if (datatype == CATEGORICAL)
            return "CATEGORICAL";
        else if (datatype == OTHER)
            return "OTHER";
        else
            return "CONTINUOUS";
    }

    class EnvUnit {
    public:
        bool IsCal;                      // involed in calculation or not
        vector<string> LayerNames;
        vector<double> EnvValues;        // all environment values of the current cell
        vector<DataTypeEnum> DataTypes;  // types of all environment variables
        vector<double> MembershipValues; // all fuzzy membership values of the current cell
                                         //vector<EnvUnit *> SimiEnvUnits;  // other cells that similar to the current one
        double SoilType;                 // soil type
        double SoilVariable;             // soil property
        string SampleID;                 // record sample ID
        Location* Loc;                   // location information
        double Uncertainty;              // uncertainty
        double Uncertainty_tmp;          // temporary uncertainty value
        double MaxSimi;                  // maximum similarity of the current cell to samples
        double CellSize;                 // cell size
        bool isCanPredict;               // can the current cell be predicted

        double PredictValue; // prediction value
        double PredictUncertainty; // uncertainty of prediction
        double PredictCredibility; // credibility of prediction

        double Credibility;    // For sample point only! credibility of sample point
        int Number_Support;    // samples number
        int Number_Contradict; // number of samples with contradiction

        int Density;  // density of points that have similarity values greater than threshold
        double DSimi; // similarity of the point that has higher density and highest similarity than the current cell

    public:
        EnvUnit() {
            IsCal = true;
            Loc = new Location();
            MaxSimi = 0;
            CellSize = 10;
            isCanPredict = false;
            Credibility = 0;
            Number_Contradict = 0;
            Number_Support = 0;
            PredictCredibility = -1;
            PredictValue = -1;
            PredictUncertainty = -1;
            SoilType = 0.0;
            SoilVariable = 0.0;
            SampleID = "id-none";
            LayerNames.clear();
            EnvValues.clear();
            MembershipValues.clear();
            DataTypes.clear();
            Density = 0;
            DSimi = 0;
        }

        ~EnvUnit() {
            //if(Loc != nullptr)
            //	delete Loc;
            LayerNames.clear();
            EnvValues.clear();
            MembershipValues.clear();
            DataTypes.clear();
            //SimiEnvUnits.clear();
        }

        void AddEnvValue(string layername, double envValue, DataTypeEnum type) {
            LayerNames.push_back(layername);
            EnvValues.push_back(envValue);
            DataTypes.push_back(type);
        }

        void AddEnvValue(double envValue) {
            EnvValues.push_back(envValue);
        }

        void AddMembershipValue(double membershipValue) {
            MembershipValues.push_back(membershipValue);
        }

    };

    class EnvLayer {
    public:
        int LayerId;
        string LayerName;
        // Adding MembershipData to use save membershipValue
        float* EnvData;
        DataTypeEnum DataType;
        //GDALDataset* GdalEnvData;
        BaseIO *baseRef;
        float* upperBorder;
        float* lowerBorder;
        double Data_Max;
        double Data_Min;
        double Data_Range;
        double NoDataValue;
        double CellSize;
        int XSize;
        int YSize;
        int BlockSize;

    public:
        EnvLayer();

        EnvLayer(const int layerId, string layerName, const string& filename, const DataTypeEnum dataType, BaseIO *ref);

        ~EnvLayer();

        //void WriteoutMembership(const string& filename, const string& type, GDALDataset* srcDs);
        void WriteByBlock(BaseIO *outFile, int blockRank);
        //void Writeout(string filename, string type, GDALDataset* srcDs);
        void Writeout(BaseIO *outFile, int xstart, int ystart, int ny, int nx);

        void Readin(int xstart, int ystart, int ny, int nx);
        void ReadByBlock(int blockRank);
        int GetYSizeByBlock(int blockRank);
    };
    class EnvDataset {
    public:
        // basic setting
        BaseIO *LayerRef;
        double CellSize;
        double CellSizeY;
        double XMin;
        double XMax;
        double YMin;
        double YMax;
        double NoDataValue;
        //int PartitionSizeX;
        //int PartitionSizeY;
        int BlockSizeX; // cols of current partition
        int BlockSizeY; // rows of current partition
                        //int XStart; // global coordinate of current partition's first cell's position
                        //int YStart;
        long TotalX; // global size
        long TotalY;
        int CalcArea;
        // layers
        vector <EnvLayer*> Layers;
        vector <string> LayerNames;
    public:
        EnvDataset();

        EnvDataset(vector<string> &envLayerFilenames, vector<string> &datatypes, vector<string>& layernames, double ramEfficent=0.25);

        ~EnvDataset();

        void AddLayer(EnvLayer* layer, string layername) {
            Layers.push_back(layer);
            LayerNames.push_back(layername);
        }

        void RemoveAllLayers();

        void ReadinLayers(vector<string> &envLayerFilenames, const vector<string> &datatypes, vector<string> &layerNames, double ramEfficent);

        EnvLayer* getDEM();

        //void RefreshEnvUnits();

        // void RefreshSetting();

        //void RefreshCalArea();

        // void RefreshAll();

        EnvUnit* GetEnvUnit(const int row, const int col);

        EnvUnit* GetEnvUnit(const double x, const double y);

        static vector<EnvUnit> *ReadTable(string filename,
            EnvDataset* envDataset,
            string targetVName/* = "None" */,
            string idName/* = "None" */);

    private:
        EnvDataset(const EnvDataset&);

        EnvDataset& operator=(const EnvDataset&);

    };

    class Curve {
    public:
        string covariateName;
        DataTypeEnum dataType;
        double typicalValue;
        double range;
        vector <double> vecKnotX;
    private:
        vector <double> vecKnotY;
        vector <double> vecDY;
        vector <double> vecDDY;
        vector <double> vecS;
        int iKnotNum;
    public:
        Curve();
        Curve(string covName, DataTypeEnum type);
        Curve(string covName, DataTypeEnum type, int knotNum, string coords,double valueRange =0);
        Curve(string covName, DataTypeEnum type, vector<double> *x, vector<double> *y);	// freehand rule
        Curve(string covName, double lowUnity, double highUnity, double lowCross, double highCross, CurveTypeEnum curveType);	// range rule
        Curve(string covName, double xcoord, double ycoord, EnvLayer *layer);	// point rule
        Curve(string covName, vector<float> *values);	// data mining continuous
        Curve(string covName, vector<int> *values);		// data mining categorical
        Curve(string covName, vector<Curve> *curves);
        double KernelEst(double x, int n, double h, vector<float> *values);
        void addKnot(double x, double y);
        double getOptimality(double x);
        void updateCurve();
        int getKnotNum();
        string getCoords();

    private:
        void bubbleSort();
        void calcSpline();
        int bsearch(int low, int high, double x);
        //double rangeFunction(double value, double unity, double cross);
    };

    class Prototype {
    public:
        string prototypeBaseName;
        string prototypeID;
        vector<Curve> envConditions;
        vector<SoilProperty> properties;
        PrototypeSource source;
    public:
        double uncertainty;
        int envConditionSize;
    //private:
        bool envConsIsSorted;

    public:
        Prototype();
        Prototype(EnvDataset* eds, int iSoilIDField, string prototypeBasename, OGRFeature* poFeature, int fid);
        Prototype(EnvDataset* eds, int iSoilIDField, string prototypeBasename, OGRLayer* poLayer, vector<int> fids);
        static vector<Prototype> *getPrototypesFromSample(string filename, EnvDataset* eds, string prototypeBaseName="", string xfield="x", string yfield="y");
        static vector<Prototype> *getPrototypesFromMining_soilType(string filename, EnvDataset* eds, string soilIDFieldName, string prototypeBasename, QProgressBar *progressBar);
        static vector<Prototype> *getPrototypesFromMining_polygon(string filename, EnvDataset *eds, string soilIDFieldName, string prototypeBasename, QProgressBar *progressBar);
        void addConditions(string filename);
        void addConditions(Curve con) { envConditions.push_back(con); envConditionSize++; }
        void addProperties(string propertyName, double propertyValue, DataTypeEnum type=CONTINUOUS);
        double getProperty(string propName);
        void writeRules(string fileName);
        void writePrototype(string fileName);
        TiXmlElement* writePrototypeXmlElement();
        void readPrototype(string filename);
        double calcSimi(EnvUnit *e);
        double calcSimi_preChecked(EnvUnit *e);
        bool checkEnvConsIsSorted(EnvDataset *eds);
        void sortEnvCons(vector<string> layernames);
        static double getOptimality(vector<Prototype> *prototypes, EnvUnit *e, string soilPropertyName, double soilTypeTag);
    };

    class Exception {
    public:
        vector<Curve> envConditions;
        ExceptionType exceptionType;
        double centralX, centralY;
        double impactRadius;
        bool distanceDecayEnabled;
        double distanceDecayFactor;
        vector<SoilProperty> properties;
    private:
        int envConditionSize;
        bool envConsIsSorted;

    public:
        Exception(string exceptionType, double x, double y);
        Exception(string exceptionType, double x, double y, bool decay, double radius);
        void addConditions(Curve c) { envConditions.push_back(c); }
        void addProperties(string propertyName, double propertyValue/*, DataTypeEnum type*/);
        double getProperty(string propName);
        void writeRules(string fileName);
        double calcSimi(EnvUnit *e);
        void sortEnvCons(vector<string> layernames);
        double calcOccurrExcluSim(vector<Prototype> *instances, EnvUnit *e, string soilPropertyName, double soilTypeTag, EnvDataset *eds, double zFactor);
        static double getOccurrenceOpt(vector<Exception> *prototypes, vector<Prototype> *instances, EnvUnit *e, string soilPropertyName, double soilTypeTag, EnvDataset *eds, double zFactor);
        static double getExclusionOpt(vector<Exception> *prototypes, vector<Prototype> *instances, EnvUnit *e, string soilPropertyName, double soilTypeTag, EnvDataset *eds, double zFactor);
        static double getOptimality(vector<Exception> *occurrences, vector<Exception> *exclusions, vector<Prototype> *instances, EnvUnit *e, string soilPropertyName, double soilTypeTag, EnvDataset *eds, double zFactor);
    private:
        double distSimilarity(double dSim, double dDist, double dBoundSim, double dBoundDist, double dDistDecayFactor);
        double topoDistanceXY(double dX, double dY, EnvLayer *layer_DEM, double dZFactor);
    };

    class Inference {
    public:
        static void inferMap(EnvDataset *eds, vector<Prototype>* prototypes, string targetVName, double threshold, string outSoilFile, string outUncerFile, QProgressBar *progressBar);
        static void iPSMInferSoil(vector<string> filenames, vector<string> datatypes, vector<string> layernames,
            double threshold, string sampleFilename, string targetVName, string idName,
            string outSoil, string outUncer, double ramEfficient, QProgressBar *progressBar);
        static void typeInference();
        static void propertyInference();
    };

}

#endif // SOLIMLIBFORQT_H
