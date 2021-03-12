#include "preprocess.h"

// using namespace std; // Avoid this usage, instead of specific functions. 2019/08/06 ZHULJ
using std::ostringstream;

namespace solim {

string ConvertToString(double value) {
    ostringstream os;
    os.unsetf(std::ios::scientific);
    os.precision(10);
    if (os << value)
        return os.str();
    return "invalid conversion";
}

/*!
 * @brief
 * @param str
 * @param c
 * @param tokens
 */
void ParseStr(const string& str, char c, vector<string>& tokens) {
    unsigned int posL = 0;
    unsigned int posR = 0;
    while (posR < str.length() - 1) {
        posR = str.find_first_of(c, posL);
        string sub = str.substr(posL, posR - posL);
        tokens.push_back(sub);
        posL = posR + 1;
    }
}

char* FindImageTypeGDAL(char* fileName) {
    char* dstExtension = strlwr(strrchr(fileName, '.') + 1);
    char* Gtype = NULL;
    if (0 == strcmp(dstExtension, "bmp")) Gtype = "BMP";
    else if (0 == strcmp(dstExtension, "jpg")) Gtype = "JPEG";
    else if (0 == strcmp(dstExtension, "png")) Gtype = "PNG";
    else if (0 == strcmp(dstExtension, "tif")) Gtype = "GTiff";
    else if (0 == strcmp(dstExtension, "gif")) Gtype = "GIF";

    return Gtype;
}

void Stretch(EnvLayer* lyr, double smax, double smin, double max, double min) {
    for (int i = 0; i < lyr->XSize * lyr->YSize; i++) {
        double val = lyr->EnvData[i];
        if ((val - lyr->NoDataValue) < 0.0001) { continue; }
        if (val > max) { lyr->EnvData[i] = smax; } else if (val < min) { lyr->EnvData[i] = smin; } else {
            lyr->EnvData[i] = (smax - smin) * (val - min) / (max - min) + smin;
        }
    }
	lyr->Data_Max = smax;
	lyr->Data_Min = smin;
    //lyr->CalcStat();
}

vector<double>* Neighborhood(EnvLayer* lyr, int pos, int radius) {
    // get cell's neighborhood
    // pos is cell's sequence in EnvData, e.g. lyr->Envdata[pos]
    // radius depicts the size of neighborhood, size is (2 * radius + 1) * (2 * radius + 1), e.g. radius = 1, size is 3 * 3
    // returns a vector of valid neighborhood cell values
    int x, y;
    x = pos % lyr->XSize;
    y = int(pos / lyr->XSize);
    vector<double>* neighbor = new vector<double>;

    int startRow = x - radius > 0 ? x - radius : 0;
    int endRow = x + radius < lyr->XSize ? x + radius : lyr->XSize - 1;
    int startCol = y - radius > 0 ? y - radius : 0;
    int endCol = y + radius < lyr->YSize ? y + radius : lyr->YSize - 1;

    for (int i = startRow; i <= endRow; i++) {
        for (int j = startCol; j <= endCol; j++) {
            int tmpPos = i + j * lyr->XSize;
            if ((lyr->EnvData[tmpPos] - lyr->NoDataValue) > 0.0001) {
                neighbor->push_back(lyr->EnvData[tmpPos]);
            }
        }
    }
    return neighbor;
}

void Filter(EnvLayer* lyr, int radius, FilterType type /* = MEAN */) {
    // using filter to process the image
    EnvLayer* originlyr = new EnvLayer(*lyr);
    for (int i = 0; i < lyr->XSize * lyr->YSize; i++) {
        double val = lyr->EnvData[i];
        vector<double>* neighbor = Neighborhood(originlyr, i, radius);
        if ((val - lyr->NoDataValue) < 0.0001) { continue; }
        if (type == MEAN) {
            // mean filter
            double sum = 0;
            for (vector<double>::iterator itr = neighbor->begin(); itr != neighbor->end(); ++itr) {
                sum += *itr;
            }
            if (!neighbor->empty()) {
                lyr->EnvData[i] = sum / double(neighbor->size());
            }
        }
        // TODO: add more types of filters
        neighbor->clear();
        delete neighbor;
        neighbor = nullptr;
    }
    //lyr->CalcStat();
}

bool resample(string srcFile, string dstProjectionRefFile, string destFile) {
	if (strcmp(strrchr(srcFile.c_str(), '.'), ".3dr") != 0) {
		GDALAllRegister();
		GDALDataset *src;
		src = (GDALDataset*)GDALOpen(srcFile.c_str(), GA_ReadOnly);
		if (src == NULL) {
			cout << "Error opening file " << srcFile << endl;
			return false;
		}
		GDALDriver *driver = src->GetDriver();
		const char *pszSrcWKT = src->GetProjectionRef();

		GDALDataset *dstProjRef;
		dstProjRef = (GDALDataset*)GDALOpen(dstProjectionRefFile.c_str(), GA_Update);
		if (dstProjRef == NULL) {
			cout << "Error opening file " << dstProjectionRefFile << endl;
			return false;
		}
		const char *pszDstWKT = dstProjRef->GetProjectionRef();
		GDALRasterBand* srcBand = src->GetRasterBand(1);
		GDALDataset *dest = driver->Create(destFile.c_str(),
			dstProjRef->GetRasterXSize(), dstProjRef->GetRasterYSize(), 1,
			srcBand->GetRasterDataType(), NULL);
		GDALRasterBand* destBand = dest->GetRasterBand(1);
		destBand->SetColorInterpretation(srcBand->GetColorInterpretation());
		int success = -1;
		double noDataValue = NODATA;
		noDataValue = srcBand->GetNoDataValue(&success);
		if (!success)
		{
			return false;
		}
		destBand->SetNoDataValue(noDataValue);
		GDALColorTable* colorTable = srcBand->GetColorTable();
		if (colorTable)
		{
			destBand->SetColorTable(colorTable);
		}
		dest->SetProjection(pszDstWKT);
		double adfGeoTransform[6];
		dstProjRef->GetGeoTransform(adfGeoTransform);
		dest->SetGeoTransform(adfGeoTransform);

		CPLErr result = GDALReprojectImage(src, NULL, dest, NULL, GRA_NearestNeighbour, 0.0, 0.0, NULL, NULL, NULL);
		if (result == CE_Failure) {
			return false;
		}

		return true;
	}
	return false;
}

}
