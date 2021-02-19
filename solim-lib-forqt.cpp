#include "solim-lib-forqt.h"

BaseIO::BaseIO(string filename, FileDataType newFileDataType) {
    fileName=filename;
    if (strcmp(strrchr(filename.c_str(), '.'), ".3dr") == 0) {
        is3dr = TRUE;
        fileName = filename;
        char utilityString[50];
        inCurLoc = 0;
        if ((threeDRfp = fopen(filename.c_str(), "r")) == NULL) {
            cout << "Cannot open inputFile for reading header." << endl;
            openSuccess = false;
            return;	//Cannot open inputFile for reading header.
        }
        fscanf(threeDRfp, "%s", utilityString);
        if (strcmp(utilityString, "NumberOfRecords:") != 0) {
            openSuccess = false;
            return;	//File format of inputFile has error(s).
        }
        else fscanf(threeDRfp, "%d", &NumberOfRecords);
        int getMaxMin = 0;
        int getGeo = 0;
        for (int nl = 0; nl < NumberOfRecords; nl++) {
            fscanf(threeDRfp, "%s", utilityString);
            if (strcmp(utilityString, "NumberOfColumns:") == 0) {
                fscanf(threeDRfp, "%d", &xSize);
            }
            else if (strcmp(utilityString, "NumberOfRows:") == 0) {
                fscanf(threeDRfp, "%d", &ySize);
            }
            else if (strcmp(utilityString, "FileType:") == 0) {
                fscanf(threeDRfp, "%s", &fileType[0]);
            }
            else if (strcmp(utilityString, "DataName:") == 0) {
                char dataName[FN_LEN];
                fgets(&dataName[0], FN_LEN, threeDRfp);
            }
            else if (strcmp(utilityString, "BaseFilename:") == 0) {
                char baseFilename[FN_LEN];
                fgets(&baseFilename[0], FN_LEN, threeDRfp);
            }
            else if (strcmp(utilityString, "GridUnits:") == 0) {
                fscanf(threeDRfp, "%s", &gridUnits[0]);
            }
            else if (strcmp(utilityString, "Xmin:") == 0) {
                fscanf(threeDRfp, "%lf", &xLeftEdge);
                getGeo += 1;
            }
            else if (strcmp(utilityString, "Ymin:") == 0) {
                fscanf(threeDRfp, "%lf", &yllCenter);
                getGeo += 1;
            }
            else if (strcmp(utilityString, "CellSize:") == 0) {
                fscanf(threeDRfp, "%lf", &cellSize);
                getGeo += 1;
            }
            else if (strcmp(utilityString, "Neighborhood:") == 0) {
                double Neighborhood;	// todo: meaning of neighborhood?
                fscanf(threeDRfp, "%lf", &Neighborhood);
            }
            else if (strcmp(utilityString, "DataUnits:") == 0) {
                char DataUnits[FN_LEN];	// todo: meaning of dataunits?
                fscanf(threeDRfp, "%s", &DataUnits[0]);
            }
            else if (strcmp(utilityString, "DataMin:") == 0) {
                fscanf(threeDRfp, "%lf", &dataMin);
                getMaxMin += 1;
            }
            else if (strcmp(utilityString, "DataMax:") == 0) {
                fscanf(threeDRfp, "%lf", &dataMax);
                getMaxMin += 1;

            }
            else if (strcmp(utilityString, "DataMean:") == 0) {
                fscanf(threeDRfp, "%lf", &dataMean);
            }
            else if (strcmp(utilityString, "DataStd:") == 0) {
                fscanf(threeDRfp, "%lf", &dataStd);
            }
            else if (strcmp(utilityString, "NumberOfColors:") == 0) {
                int NumberOfColors;		// todo: meaning?
                fscanf(threeDRfp, "%d", &NumberOfColors);
            }
            else if (strcmp(utilityString, "DataType:") == 0) {
                char DataType[FN_LEN];		// todo: what are the types?
                fscanf(threeDRfp, "%s", &DataType[0]);
            }
            else if (strcmp(utilityString, "DataClampMin:") == 0) {
                double DataClampMin;	// todo: meaning?
                fscanf(threeDRfp, "%lf", &DataClampMin);
            }
            else if (strcmp(utilityString, "DataClampMax:") == 0) {
                double DataClampMax;	// todo: meaning?
                fscanf(threeDRfp, "%lf", &DataClampMax);
            }
            else if (strcmp(utilityString, "NullDataValue:") == 0) {
                fscanf(threeDRfp, "%lf", &noDataValue);
            }
            else if (strcmp(utilityString, "NoData:") == 0) {
                fscanf(threeDRfp, "%lf", &noDataValue);
            }
            else if (strcmp(utilityString, "FirstDataByte:") == 0) {
                fscanf(threeDRfp, "%d", &firstDataByte);
                inCurLoc = firstDataByte;
            }
            else {
                unexpectedFieldFlag = true; //Unexpected field in inputFile
            }
        }
        if (getGeo > 2) {
            xllCenter = xLeftEdge + cellSize / 2.;
            yllCenter += cellSize / 2.;
            yTopEdge = yllCenter + (ySize * cellSize) - cellSize / 2.;
        }
        else {
            cout << "Input file has no geographic information" << endl;
            return;

        }
        if (getMaxMin>1)
            dataRange = dataMax - dataMin;
        else {}//todo get max and min
        inCurLoc = firstDataByte;
        dataEndLoc = xSize*ySize * sizeof(float) + firstDataByte;
        fclose(threeDRfp);
    }
    else {
        is3dr = FALSE;
        GDALAllRegister();
        fh = GDALOpen(filename.c_str(), GA_ReadOnly);
        if (fh == NULL) {
            cout << "Error opening file " << filename << endl;
            return;
        }
        hDriver = GDALGetDatasetDriver(fh);

        //OGRSpatialReferenceH  hSRS;
        char *pszProjection;
        pszProjection = (char *)GDALGetProjectionRef(fh);
        srs = OGRSpatialReference(pszProjection);
        isGeographic = srs.IsGeographic();
        if (isGeographic == 0) {
            cout << "Input file " << filename << " has projected coordinate system." << endl;
        }
        else
            cout << "Input file " << filename << " has geographic coordinate system." << endl;
        // cout<<getproj<<endl; // for test

        bandh = GDALGetRasterBand(fh, 1);

        xSize = GDALGetRasterXSize(fh);
        ySize = GDALGetRasterYSize(fh);
        GDALGetGeoTransform(fh, adfGeoTransform);
        dx = abs(adfGeoTransform[1]);
        dy = abs(adfGeoTransform[5]);
        dlon = abs(adfGeoTransform[1]);
        dlat = abs(adfGeoTransform[5]);
        xLeftEdge = adfGeoTransform[0];
        yTopEdge = adfGeoTransform[3];
        xllCenter = xLeftEdge + dlon / 2.;
        yllCenter = yTopEdge - (ySize * dlat) - dlat / 2.;

        double xp[2];

        double *dxc = new double[ySize];
        double *dyc = new double[ySize];
        if (isGeographic == 1) {
            for (int j = 0; j < ySize; ++j) {
                // latitude corresponding to row
                float rowlat = yllCenter + (ySize - j - 1) * dlat;
                geoToLength(dlon, dlat, rowlat, xp);
                dxc[j] = xp[0];
                dyc[j] = xp[1];
            }
        }
        else {
            for (int j = 0; j < ySize; ++j) {
                dxc[j] = dlon;
                dyc[j] = dlat;
            }
        }

        dxA = fabs(dxc[ySize / 2]);
        dyA = fabs(dyc[ySize / 2]);

        fileDataType = newFileDataType;
        noDataValue = GDALGetRasterNoDataValue(bandh, NULL);
        // calculate max, min
        int fGotMax = 0;
        int fGotMin = 0;
        dataMax = GDALGetRasterMaximum(bandh, &fGotMax);
        dataMin = GDALGetRasterMinimum(bandh, &fGotMin);
        if (!(fGotMax&&fGotMin)) {
            double adfMinMax[2];
            GDALComputeRasterMinMax(bandh, TRUE, adfMinMax);
            dataMin = adfMinMax[0];
            dataMax = adfMinMax[1];
        }
        dataRange = dataMax - dataMin;
        dataMax += dataRange / 100;
        dataMin -= dataRange / 100;
    }
    blockIsInitialized = false;
    isFileInititialized = false;
    openSuccess = true;
}

BaseIO::~BaseIO() {
    if (is3dr) {
        fclose(threeDRfp);
    }
    else {
        GDALClose((GDALDatasetH)fh);
    }
}

void BaseIO::read(long xStart, long yStart, long numRows, long numCols, float *dest) {
    if (is3dr) {
        if ((threeDRfp = fopen(fileName.c_str(), "rb")) == NULL) {
            return;			//Cannot open inputFile for reading data.
        }
        fseek(threeDRfp, inCurLoc + (xSize*yStart + xStart) * sizeof(float), SEEK_SET);
        fread(dest, sizeof(float), numRows*numCols, threeDRfp);
        fclose(threeDRfp);
    }
    else {
        GDALDataType eBDataType = GDT_Float32;
        if (fileDataType == FLOAT_TYPE) {
            eBDataType = GDT_Float32;
        }
        else if (fileDataType == SHORT_TYPE) {
            eBDataType = GDT_Int16;
        }
        else if (fileDataType == LONG_TYPE) {
            eBDataType = GDT_Int32;
        }

        CPLErr result = GDALRasterIO(bandh, GF_Read, xStart, yStart, numCols, numRows,
            dest, numCols, numRows, eBDataType, 0, 0);
        if (result != CE_None) {
            cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
        }
    }
}

float BaseIO::getValue(long col, long row) {
    float *value = new float;
    if (is3dr) {
        fseek(threeDRfp, inCurLoc + (xSize*row + col) * sizeof(float), SEEK_SET);
        fread(value, sizeof(float), 1, threeDRfp);
    }
    read(col, row, 1, 1, value);
    return *value;
}


void BaseIO::write(long xStart, long yStart, long numRows, long numCols, float *source) {
    if (is3dr) {
        if (!isFileInititialized) {
            if ((threeDRfp = fopen(fileName.c_str(), "w")) == NULL) {
                return;	//Cannot open outputFile for writing header.
            }
            NumberOfRecords = 0;
            fprintf(threeDRfp, "%s %d\n", "NumberOfRecords: ", 99);
            if (xSize > 0) {
                fprintf(threeDRfp, "%s %d\n", "NumberOfColumns: ", xSize);
                NumberOfRecords++;
            }
            else {
                cout << "Number of columns of outputFile is less than 1. Stop writing." << endl;
                return;	//Number of columns of outputFile is less than 1. Stop writing.
            }
            if (ySize > 0) {
                fprintf(threeDRfp, "%s %d\n", "NumberOfRows: ", ySize);
                NumberOfRecords++;
            }
            else {
                cout << "Number of rows of outputFile is less than 1. Stop writing." << endl;
                return;	//Number of rows of outputFile is less than 1. Stop writing.
            }

            if (xLeftEdge > noDataValue && yllCenter > noDataValue && cellSize > noDataValue) {
                fprintf(threeDRfp, "%s %lf\n", "Xmin: ", xLeftEdge);
                fprintf(threeDRfp, "%s %lf\n", "Ymin: ", yllCenter - cellSize / 2.0);
                fprintf(threeDRfp, "%s %lf\n", "CellSize: ", cellSize);
                NumberOfRecords += 3;
            }
            if (dataMin > noDataValue) {
                fprintf(threeDRfp, "%s %lf\n", "DataMin: ", dataMin);
                NumberOfRecords++;
            }
            if (dataMax > noDataValue) {
                fprintf(threeDRfp, "%s %lf\n", "DataMax: ", dataMax);
                NumberOfRecords++;
            }
            fprintf(threeDRfp, "%s %f\n", "NoData: ", noDataValue);
            NumberOfRecords++;

            fprintf(threeDRfp, "%s %d\n", "FirstDataByte: ", 999);
            NumberOfRecords++;




            firstDataByte = ftell(threeDRfp); //get the offset location.
            rewind(threeDRfp);

            fprintf(threeDRfp, "%s %d\n", "NumberOfRecords: ", NumberOfRecords);
            fprintf(threeDRfp, "%s %d\n", "NumberOfColumns: ", xSize);
            fprintf(threeDRfp, "%s %d\n", "NumberOfRows: ", ySize);
            //if (strlen(FileType) > 0)	fprintf(fp, "%s %s\n", "FileType: ", FileType);
            //if (strlen(GridUnits) > 0)		fprintf(fp, "%s %s\n", "GridUnits: ", GridUnits);
            if (xLeftEdge > noDataValue && yllCenter > noDataValue && cellSize > noDataValue) {
                fprintf(threeDRfp, "%s %lf\n", "Xmin: ", xLeftEdge);
                fprintf(threeDRfp, "%s %lf\n", "Ymin: ", yllCenter - cellSize / 2.0);
                fprintf(threeDRfp, "%s %lf\n", "CellSize: ", cellSize);
            }
            //if (strlen(DataUnits) > 0)		fprintf(fp, "%s %s\n", "DataUnits: ", DataUnits);
            if (dataMin > noDataValue)	fprintf(threeDRfp, "%s %f\n", "DataMin: ", dataMin);
            if (dataMax > noDataValue)	fprintf(threeDRfp, "%s %f\n", "DataMax: ", dataMax);
            fprintf(threeDRfp, "%s %lf\n", "NoData: ", noDataValue);
            fprintf(threeDRfp, "%s %d\n", "FirstDataByte: ", firstDataByte);
            fclose(threeDRfp);
            isFileInititialized = TRUE;
        }
        threeDRfp = fopen(fileName.c_str(), "ab");
        fseek(threeDRfp, firstDataByte + (xSize*yStart + xStart) * sizeof(float), SEEK_SET);
        fwrite(source, sizeof(float), numRows*numCols, threeDRfp);
        fclose(threeDRfp);
    }
    else {
        char *cFileName = new char[fileName.length() + 1];
        strcpy(cFileName, fileName.c_str());

        fflush(stdout);
        char **papszMetadata;
        char **papszOptions = NULL;
        const char
            *extension_list[6] = { ".tif", ".img", ".sdat", ".bil", ".bin", ".tiff" };  // extension list --can add more
        const char *driver_code[6] = { "GTiff", "HFA", "SAGA", "EHdr", "ENVI", "GTiff" };   //  code list -- can add more
        const char *compression_meth[6] = { "LZW", "YES", " ", " ", " ", " " };   //  code list -- can add more
        size_t extension_num = 6;
        char *ext;
        int index = -1;

        // get extension  of the file
        ext = strrchr(cFileName, '.');
        if (!ext) {
            strcat(cFileName, ".tif");
            index = 0;
        }
        else {

            //  convert to lower case for matching
            for (int i = 0; ext[i]; i++) {
                ext[i] = tolower(ext[i]);
            }
            // if extension matches then set driver
            for (size_t i = 0; i < extension_num; i++) {
                if (strcmp(ext, extension_list[i]) == 0) {
                    index = i; //get the index where extension of the outputfile matches with the extensionlist
                    break;
                }
            }
            if (index < 0)  // Extension not matched so set it to tif
            {
                char filename_withoutext[MAXLN]; // layer name is file name without extension
                size_t len = strlen(cFileName);
                size_t len1 = strlen(ext + 1);
                memcpy(filename_withoutext, cFileName, len - len1);
                filename_withoutext[len - len1] = 0;
                strcpy(cFileName, filename_withoutext);
                strcat(cFileName, "tif");
                index = 0;
                fileName = cFileName;
            }
        }

        if (!isFileInititialized) {
            hDriver = GDALGetDriverByName(driver_code[index]);
            if (hDriver == NULL) {
                printf("GDAL driver is not available\n");
                fflush(stdout);
                return;
            }
            // Set options
            if (index == 0) {  // for .tif files.  Refer to http://www.gdal.org/frmt_gtiff.html for GTiff options.
                papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", compression_meth[index]);
            }
            else if (index
                == 1) { // .img files.  Refer to http://www.gdal.org/frmt_hfa.html where COMPRESSED = YES are create options for ERDAS .img files
                papszOptions = CSLSetNameValue(papszOptions, "COMPRESSED", compression_meth[index]);
            }
            int cellbytes = 4;
            if (fileDataType == SHORT_TYPE)cellbytes = 2;
            double fileGB = (double)cellbytes * (double)xSize * (double)ySize
                / 1000000000.0;  // This purposely neglects the lower significant digits to overvalue GB to allow space for header information in the file
            if (fileGB > 4.0) {
                if (index == 0 || index
                    == 6) {  // .tiff files.  Need to explicity indicate BIGTIFF.  See http://www.gdal.org/frmt_gtiff.html.
                    papszOptions = CSLSetNameValue(papszOptions, "BIGTIFF", "YES");
                    printf("Setting BIGTIFF, File: %s, Anticipated size (GB):%.2f\n", fileName.c_str(), fileGB);
                }
            }

            GDALDataType eBDataType = GDT_Float32;
            if (fileDataType == FLOAT_TYPE) {
                eBDataType = GDT_Float32;
            }
            else if (fileDataType == SHORT_TYPE) {
                eBDataType = GDT_Int16;
            }
            else if (fileDataType == LONG_TYPE) {
                eBDataType = GDT_Int32;
            }

            char *pszProjection;
            pszProjection = (char *)GDALGetProjectionRef(fh);

            fh = GDALCreate(hDriver, fileName.c_str(), xSize, ySize, 1, GDT_Float32, NULL);

            GDALSetProjection(fh, pszProjection);
            GDALSetGeoTransform(fh, adfGeoTransform);

            bandh = GDALGetRasterBand(fh, 1);
            GDALSetRasterNoDataValue(bandh, noDataValue);  // noDatarefactor 11/18/17

            isFileInititialized = true;
        }
        else {
            // Open file if it has already been initialized
            fh = GDALOpen(fileName.c_str(), GA_Update);
            bandh = GDALGetRasterBand(fh, 1);
        }
        //  Now write the data from rank 0 and close the file
        GDALDataType eBDataType;
        if (fileDataType == FLOAT_TYPE)
            eBDataType = GDT_Float32;
        else if (fileDataType == SHORT_TYPE)
            eBDataType = GDT_Int16;
        else if (fileDataType == LONG_TYPE)
            eBDataType = GDT_Int32;

        CPLErr result = GDALRasterIO(bandh, GF_Write, xStart, yStart, numCols, numRows,
            source, numCols, numRows, GDT_Float32, 0, 0);
        if (result != CE_None) {
            cout << "RaterIO trouble: " << CPLGetLastErrorMsg() << endl;
        }

        GDALFlushCache(fh);  //  DGT effort get large files properly written
        GDALClose(fh);

    }
}

void BaseIO::geoToGlobalXY(double geoX, double geoY, int &globalX, int &globalY) {
    double dlon, dlat;
    if (is3dr) {
        dlon = cellSize;
        dlat = cellSize;
    }
    else {
        dlon = abs(adfGeoTransform[1]);
        dlat = abs(adfGeoTransform[5]);
    }

    globalX = (int)((geoX - xLeftEdge) / dlon);
    globalY = (int)((yTopEdge - geoY) / dlat);
}
void BaseIO::globalXYToGeo(long globalX, long globalY, double &geoX, double &geoY) {
    double dlon, dlat;
    if (is3dr) {
        dlon = cellSize;
        dlat = cellSize;
    }
    else {
        dlon = abs(adfGeoTransform[1]);
        dlat = abs(adfGeoTransform[5]);
    }

    geoX = xLeftEdge + dlon / 2. + globalX * dlon;
    geoY = yTopEdge - dlat / 2. - globalY * dlat;
}

bool BaseIO::isInPartition(int x, int y) {
    if (x > 0 && x < blockX &&y>0 && y < blockY) {
        return TRUE;
    }
    else
        return FALSE;
}
bool BaseIO::globalToLocal(int blockRank, int globalX, int globalY, int &localX, int &localY) {
    localX = globalX;
    localY = globalY - blockRank * blockRows;
    if (blockRank == blockSize - 1 && blockRank > 0) {
        localY = globalY - blockRank * (blockRows - ySize % blockRank);
    }
    return isInPartition(localX, localY);
}
void BaseIO::localToGlobal(int blockRank, int localX, int localY, int &globalX, int &globalY) {
    globalX = localX;
    globalY = blockRows * blockRank + localY;
}

void BaseIO::geoToLength(double dlon, double dlat, double lat, double *xyc) {
    double ds2, beta, dbeta;

    dlat = dlat * PI / 180.;
    dlon = dlon * PI / 180.;
    lat = lat * PI / 180.;
    beta = atan(boa * tan(lat));
    dbeta = dlat * boa * (cos(beta) / cos(lat)) * (cos(beta) / cos(lat));
    ds2 = (pow(elipa * sin(beta), 2) + pow(elipb * cos(beta), 2)) * pow(dbeta, 2);
    xyc[0] = elipa * cos(beta) * abs(dlon);
    xyc[1] = double(sqrt(double(ds2)));
}



void BaseIO::blockInit(double divide) {
    if (blockIsInitialized) {
        return;
    }
    else {
        MEMORYSTATUSEX statusex;
        unsigned long long avl = 0;
        unsigned long long memoryForOneBlock = 0;
        statusex.dwLength = sizeof(statusex);
        if (GlobalMemoryStatusEx(&statusex)) {
            avl = statusex.ullAvailPhys; // available memory in kb
            unsigned long long memoryForOneBlock = avl * divide;
            blockRows = memoryForOneBlock / sizeof(float) / xSize;
        }
        else {
            blockRows = 100;
        }

        blockX = xSize;
        if (blockRows < ySize) {
            blockSize = ySize / blockRows;
            if (ySize % blockRows > 0)
                ++blockSize;
        }
        else {
            blockSize = 1;
            blockRows = ySize;
        }
        blockY = blockRows;
        blockIsInitialized = true;
    }
}

void BaseIO::blockCopy(BaseIO *ref) {
    if (blockIsInitialized) {
        return;
    }
    else {

        blockRows = ref->blockRows;
        blockSize = ref->blockSize;
        blockX = xSize;
        blockY = blockRows;
        blockIsInitialized = true;
    }
}
bool BaseIO::compareIO(BaseIO *layer) {
    if (xSize != layer->xSize || ySize != layer->ySize) {
        cout << "Columns or Rows do not match" << endl;
        return false;
    }
    if (abs(dxA - layer->dxA) > VERY_SMALL || abs(dyA - layer->dyA)> VERY_SMALL) {
        cout << "dx or dy do not match" << endl;
        return false;
    }

    if (abs(xLeftEdge - layer->xLeftEdge) > VERY_SMALL) {
        cout << "Warning! Left edge does not match exactly" << endl;
        cout << xLeftEdge << " in file " << fileName << endl;
        cout << layer->xLeftEdge << " in file " << layer->fileName << endl;
    }
    if (abs(yTopEdge - layer->yTopEdge) > VERY_SMALL) {
        cout << "Warning! Left edge does not match exactly" << endl;
        cout << yTopEdge << " in file " << fileName << endl;
        cout << layer->yTopEdge << " in file " << layer->fileName << endl;
    }
    return true;
}

namespace solim {
    EnvLayer::EnvLayer()
        : LayerId(-1), DataType(CONTINUOUS), baseRef(nullptr), Data_Max(NODATA),
        Data_Min(NODATA), Data_Range(NODATA), NoDataValue(NODATA),
        XSize(-1), YSize(-1) {}


    EnvLayer::EnvLayer(const int layerId, string layerName, const string& filename, const DataTypeEnum dataType, BaseIO *ref) :
        LayerId(layerId), LayerName(layerName), DataType(dataType) {
        baseRef = new BaseIO(filename, FLOAT_TYPE);
        //baseRef->parallelInit(MPI_FLOAT);
        //baseRef->blockInit();
        baseRef->blockCopy(ref);
        XSize = baseRef->getBlockX();	// number of columns
        YSize = baseRef->getBlockY();
        BlockSize = baseRef->getBlockSize();
        EnvData = new float[XSize * YSize];
        for (int i = 0; i < XSize * YSize; ++i) {
            EnvData[i] = 0.0;
        }
        Data_Min = baseRef->getDataMin();
        Data_Max = baseRef->getDataMax();
        Data_Range = baseRef->getDataRange();
        CellSize = baseRef->getCellSize();
        NoDataValue = baseRef->getNoDataValue();
        //CalcStat();
    }

    EnvLayer::~EnvLayer(void) {
        delete[]EnvData;
        delete baseRef;
    }

    void EnvLayer::WriteByBlock(BaseIO *outFile, int blockRank) {
        int localx = 0;
        int localy = 0;
        int globalx, globaly;
        baseRef->localToGlobal(blockRank, localx, localy, globalx, globaly);
        int nx = baseRef->getBlockX();
        int ny = baseRef->getBlockY();
        if (blockRank == (BlockSize - 1)) {
            ny = baseRef->getYSize() - blockRank * baseRef->getBlockY();
        }
        outFile->write(globalx, globaly, ny, nx, EnvData);
    }
    void EnvLayer::Writeout(BaseIO *outFile, int xstart, int ystart, int ny, int nx) {
        outFile->write(xstart, ystart, ny, nx, EnvData);
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
    }

    EnvDataset::EnvDataset()
        : LayerRef(nullptr), CellSize(-9999.), CellSizeY(-9999.), XMin(-9999.), XMax(-9999.),
        YMin(-9999.), YMax(-9999.), TotalX(0), TotalY(0), CalcArea(0) {
    }

    EnvDataset::EnvDataset(vector<string>& envLayerFilenames, vector<string>& datatypes, vector<string>& layernames, double ramEfficent)
        : LayerRef(nullptr), CellSize(-9999.), CellSizeY(-9999.), XMin(-9999.), XMax(-9999.),
        YMin(-9999.), YMax(-9999.), TotalX(0), TotalY(0), CalcArea(0) {
        ReadinLayers(envLayerFilenames, datatypes, layernames, ramEfficent);
    }

    EnvDataset::~EnvDataset() {
        delete LayerRef;
        RemoveAllLayers();
    }


    void EnvDataset::RemoveAllLayers() {
        for(EnvLayer* layer : Layers){
            delete layer;
        }
        Layers.clear();
    }

    void EnvDataset::ReadinLayers(vector<string>& envLayerFilenames, const vector<string>& datatypes, vector<string>& layerNames, double ramEfficent) {
        /* Use the parallel framework of TauDEM to read raster data */
        if (envLayerFilenames.empty() || datatypes.empty()) {
            // Print some error information and return.
            return;
        }
        int layerNum = int(envLayerFilenames.size());
        if (layerNum != int(datatypes.size())) {
            // Print some error information and return.
            return;
        }
        // Step 1. Read the header information of the first environment layer (as reference for comparison) using tiffIO
        LayerRef = new BaseIO(envLayerFilenames[0], FLOAT_TYPE);
        TotalX = LayerRef->getXSize();
        TotalY = LayerRef->getYSize();
        CellSize = LayerRef->getDxA();
        CellSizeY = LayerRef->getDyA(); // Assuming dx==dy
        NoDataValue = LayerRef->getNoDataValue();

        // Read tiff data into partitions and blocks
        //LayerRef->parallelInit(MPI_FLOAT);
        LayerRef->blockInit(ramEfficent / double(layerNum));
        // Get the size of current partition and block
        //PartitionSizeX = LayerRef->getParallelX();
        //PartitionSizeY = LayerRef->getParallelY();
        BlockSizeX = LayerRef->getBlockX();
        BlockSizeY = LayerRef->getBlockY();
        //LayerRef->localToGlobal(0, 0, XStart, YStart);	// get the position of the current partition

        // get the global coordinates
        XMin = LayerRef->getXMin();
        YMax = LayerRef->getYMax();
        XMax = XMin + CellSize * TotalX;
        YMin = YMax - CellSizeY * TotalY;

        // Step 3. Create EnvLayer objects using linearpart data
        for (int i = 0; i < layerNum; ++i) {
            string datatype = datatypes[i];
            transform(datatype.begin(), datatype.end(), datatype.begin(), ::toupper);
            EnvLayer *newLayer = nullptr;
            if (datatype == "CATEGORICAL") {
                newLayer = new EnvLayer(i, layerNames[i], envLayerFilenames[i].c_str(), CATEGORICAL, LayerRef);
            }
            else if (datatype == "CONTINUOUS") {
                newLayer = new EnvLayer(i, layerNames[i], envLayerFilenames[i].c_str(), CONTINUOUS, LayerRef);
            }
            else {
                newLayer = new EnvLayer(i, layerNames[i], envLayerFilenames[i].c_str(), CONTINUOUS, LayerRef);
            }
            if (i == 0) {
                AddLayer(newLayer, layerNames[i]);
            }
            else {
                if (!LayerRef->compareIO(newLayer->baseRef)) {
                    cout << "File size do not match: " << envLayerFilenames[i] << endl;
                    return;
                }
                else {
                    AddLayer(newLayer, layerNames[i]);
                }
            }
        }
    }

    // void RefreshAll();

    EnvUnit* EnvDataset::GetEnvUnit(const int row, const int col) {
        // receive global col and row number
        EnvUnit *e = new EnvUnit();
        e->Loc->Row = row;
        e->Loc->Col = col;
        e->Loc->X = col * CellSize + XMin;
        e->Loc->Y = YMax - row * CellSize;
        int numRows = 1;
        int numCols = 1;
        for (int i = 0; i < Layers.size(); ++i) {
            float *value = new float;
            Layers.at(i)->baseRef->read(e->Loc->Col, e->Loc->Row, numRows, numCols, value);
            e->AddEnvValue(Layers.at(i)->LayerName, *value, Layers.at(i)->DataType);
        }
        return e;
    }

    EnvUnit* EnvDataset::GetEnvUnit(const double x, const double y) {
        int row = int((YMax - y) / CellSize);
        int col = int((x - XMin) / CellSize);
        if(row>0 && row<LayerRef->getYSize() && col>0 && col<LayerRef->getXSize()){
            EnvUnit *e = new EnvUnit();
            e->Loc->X = x;
            e->Loc->Y = y;
            e->Loc->Row = int((YMax - y) / CellSize);
            e->Loc->Col = int((x - XMin) / CellSize);
            int numRows = 1;
            int numCols = 1;
            for (int i = 0; i < Layers.size(); ++i) {
                float *value = new float;
                *value = (float)this->NoDataValue;
                Layers.at(i)->baseRef->read(e->Loc->Col, e->Loc->Row, numRows, numCols, value);
                e->AddEnvValue(Layers.at(i)->LayerName, *value, Layers.at(i)->DataType);
            }
            return e;
        } else {
            return nullptr;
        }
    }

    EnvDataset& EnvDataset::operator=(const EnvDataset&) {
        return *this;
    }

    EnvLayer *EnvDataset::getDEM() {
        for (auto it = Layers.begin(); it != Layers.end(); ++it) {
            string name = (*it)->LayerName;
            for (int i = 0; i < name.length(); ++i) {
                toupper(name[i]);
            }
            if (name == "DEM" || name == "ELEVATION") {
                return (*it);
            }
        }
        return nullptr;
    }

    vector<EnvUnit> *EnvDataset::ReadTable(string filename,
        EnvDataset* envDataset,
        string targetVName/* = "None" */,
        string idName/* = "None" */) {
        vector<EnvUnit> *envUnits = new vector<EnvUnit>;
        ifstream file(filename); // declare file stream:

        string line;
        getline(file, line);
        vector<string> names;
        int pos_X = 0;
        int pos_Y = 1;
        int pos_targetVName = -1;
        int pos_idName = 0;
        ParseStr(line, ',', names);
        for (int i = 0; i < names.size(); ++i) {
            if (names[i] == "X" || names[i] == "x") {
                pos_X = i;
                break;
            }
        }
        for (int i = 0; i < names.size(); ++i) {
            if (names[i] == "Y" || names[i] == "y") {
                pos_Y = i;
                break;
            }
        }
        if (targetVName != "None") {
            for (int i = 0; i < names.size(); ++i) {
                if (names[i] == targetVName) {
                    pos_targetVName = i;
                    break;
                }
            }
        }
        if (idName != "None") {
            for (int i = 0; i < names.size(); ++i) {
                if (names[i] == idName) {
                    pos_idName = i;
                    break;
                }
            }
        }

        while (getline(file, line)) {
            vector<string> values;
            ParseStr(line, ',', values);
            const char* xstr = values[pos_X].c_str();
            const char* ystr = values[pos_Y].c_str();
            double x = atof(xstr);
            double y = atof(ystr);

            double targetV = 0.0;
            if (targetVName != "None") {
                if (pos_targetVName == -1) {
                    throw "Target name does not exist in the file.";
                    return envUnits;
                }
                const char* targetVstr = values[pos_targetVName].c_str();
                targetV = atof(targetVstr);
            }
            else {
                pos_targetVName = 2;
                const char* targetVstr = values[pos_targetVName].c_str();
                targetV = atof(targetVstr);
            }
            string id = "";
            if (idName != "None") {
                id = values[pos_idName];
            }
            EnvUnit* e = envDataset->GetEnvUnit(x, y);
            bool nullSample = false;
            for (int i = 0; i < e->EnvValues.size(); ++i) {
                if (fabs(e->EnvValues.at(i) - envDataset->Layers.at(i)->NoDataValue) < VERY_SMALL) {
                    nullSample = true;
                    break;
                }
            }
            if (e != NULL && (!nullSample)) {
                if (targetVName != "None") { e->SoilVariable = targetV; }
                if (idName != "None") { e->SampleID = id; }
                envUnits->push_back(*e);
            }
        }
        file.close();
        return envUnits;
    }

    Curve::Curve() {
        covariateName = "";
        dataType = CONTINUOUS;	// default data type is continuous
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        iKnotNum = 0;
        typicalValue = NODATA;
    }

    Curve::Curve(string covName, DataTypeEnum type, vector<double> *x, vector<double> *y) {
        // knowledge from experts: freehand rule
        covariateName = covName;
        dataType = type;
        vecKnotX = *x;
        vecKnotY = *y;
        iKnotNum = vecKnotX.size();
        typicalValue = NODATA;
        range=0;
        if (iKnotNum != vecKnotY.size()) {
            throw invalid_argument("Error in knot coordinates");
        }
        for(int i = 0;i<iKnotNum;i++){
            if(fabs(vecKnotY[i]-1)<VERY_SMALL){
                typicalValue = vecKnotX[i];
                break;
            }
        }
        if (dataType == CONTINUOUS) {
            bubbleSort();
            calcSpline();
            range=fabs(vecKnotX[0])>fabs(vecKnotX[iKnotNum-1])?fabs(vecKnotX[0]):fabs(vecKnotX[iKnotNum-1]);
        }
    }

    Curve::Curve(string covName, DataTypeEnum type) {
        covariateName = covName;
        dataType = type;
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        iKnotNum = 0;
        typicalValue = NODATA;
        range=0;

    }

    Curve::Curve(string covName, DataTypeEnum type, int knotNum, string coords, double valueRange) {
        // knowledge from experts: word rule
        covariateName = covName;
        dataType = type;
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        iKnotNum = knotNum;
        range = valueRange;

        vector<string> xycoord;
        ParseStr(coords, ',', xycoord);
        if (xycoord.size() != iKnotNum) iKnotNum = xycoord.size();
        vecKnotX.reserve(iKnotNum);
        vecKnotY.reserve(iKnotNum);

        for (int i = 0; i < iKnotNum; ++i) {
            vector<string> coord;
            ParseStr(xycoord[i], ' ', coord);
            if (coord.size() != 2)
                throw invalid_argument("invalid coordinate description.");
            vecKnotX.push_back(atof(coord[0].c_str()));
            vecKnotY.push_back(atof(coord[1].c_str()));
            if(fabs(atof(coord[1].c_str())-1)<VERY_SMALL)
                typicalValue = atof(coord[0].c_str());
        }
        if (type == CATEGORICAL || knotNum < 1) return;
        bubbleSort();
        calcSpline();
    }

    Curve::Curve(string covName, double lowUnity, double highUnity, double lowCross, double highCross, CurveTypeEnum curveType) {
        // knowledge from experts: range rule
        covariateName = covName;
        dataType = CONTINUOUS;
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        iKnotNum = 0;
        double rangePar=2.5775678826705466;  //sqrt(ln(0.01)/ln(0.5))
        double rangePar_75=0.64423404076379;    //sqrt(ln(0.75)/ln(0.5))
        double rangePar_25 = 1.41421356237309;
        double lowRange=lowUnity-(lowUnity - lowCross)*rangePar;
        double low_75=lowUnity-(lowUnity - lowCross)*rangePar_75;
        double highRange=highUnity+(highCross-highUnity)*rangePar;
        double high_75=highUnity+(highCross-highUnity)*rangePar_75;
        range=fabs(lowRange)>fabs(highRange)?fabs(lowRange):fabs(highRange);

        // add 6 points to generate the spline curve
        // two unity points, two range points, and two cross points
        if (curveType == BELL_SHAPED) {
            addKnot(lowRange, 0);
            addKnot(lowUnity-(lowUnity - lowCross)*rangePar_25,0.25);
            addKnot(lowCross, 0.5);
            addKnot(low_75,0.75);
            addKnot(lowUnity, 1);
            addKnot(highUnity, 1);
            addKnot(high_75,0.75);
            addKnot(highCross, 0.5);
            addKnot(highUnity+(highCross-highUnity)*rangePar_25,0.25);
            addKnot(highRange, 0);
            typicalValue = (highUnity+lowUnity)*0.5;
        }
        else if (curveType == S_SHAPED) {
            range=fabs(range)>fabs(lowUnity+1)?fabs(range):fabs(lowUnity+1);
            addKnot(lowRange, 0);
            addKnot(lowUnity-(lowUnity - lowCross)*rangePar_25,0.25);
            addKnot(lowCross, 0.5);
            addKnot(low_75,0.75);
            addKnot(lowUnity, 1);
            typicalValue = lowUnity;
        }
        else if (curveType == Z_SHAPED) {
            range=fabs(range)>fabs(highUnity-1)?fabs(range):fabs(highUnity-1);
            addKnot(highUnity, 1);
            addKnot(high_75,0.75);
            addKnot(highCross, 0.5);
            addKnot(highUnity+(highCross-highUnity)*rangePar_25,0.25);
            addKnot(highRange, 0);
            typicalValue = lowUnity;
        }
        calcSpline();
    }

    Curve::Curve(string covName, double x, double y, EnvLayer *layer) {
        // knowledge from sample
        // knowledge from experts: point rule
        covariateName = covName;
        dataType = layer->DataType;
        iKnotNum = 0;
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        int row, col;
        double max = layer->Data_Max;
        double min = layer->Data_Min;
        range = (fabs(max)>fabs(min))?fabs(max):fabs(min);
        layer->baseRef->geoToGlobalXY(x, y, col, row);
        typicalValue = layer->baseRef->getValue(col, row);
        if (dataType == CATEGORICAL) {
            addKnot(typicalValue, 1);
            return;
        }
        int cellNum = 0;
        double sum = 0;
        double squareSum = 0;
        double SDjSquareSum = 0;
        for (int k = 0; k < layer->BlockSize; ++k) {
            layer->ReadByBlock(k);
            int n = layer->XSize*layer->GetYSizeByBlock(k);
            for (int i = 0; i < n; ++i) {
                double value = layer->EnvData[i];
                if (fabs(value - layer->NoDataValue) < VERY_SMALL || value<NODATA) continue;
                sum += value;
                squareSum += value*value;
                SDjSquareSum += pow(value - typicalValue, 2);
                ++cellNum;
            }
        }
        double mean = sum / (double)cellNum;
        double SDSquare = squareSum / (double)cellNum - mean * mean;
        double SDjSquare = SDjSquareSum / (double)cellNum;

        double halfPar = SDSquare/sqrt(SDjSquare)*1.17741002251547469101;    // 1.117=sqrt(-2*ln0.5);
        double zeroPar = SDSquare/sqrt(SDjSquare)*3.03485425877029270172;    // 3.034=sqrt(-2*ln0.01);
        //addKnot(dataMax, exp(-pow(dataMax-typicalValue,2) * 0.5/SDSquare*SDSquare * SDjSquare));
        addKnot(typicalValue - zeroPar,0);
        addKnot(typicalValue - halfPar,0.5);
        addKnot(typicalValue,1);
        addKnot(typicalValue + halfPar, 0.5);
        addKnot(typicalValue + zeroPar, 0);
        //addKnot(dataMin, exp(-pow(dataMin-typicalValue,2) * 0.5/SDSquare*SDSquare * SDjSquare));

        bubbleSort();
        calcSpline();
    }

    Curve::Curve(string covName, vector<float> *values) {	// add rules from data mining-continuous
        covariateName = covName;
        dataType = CONTINUOUS;
        iKnotNum = 0;
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        int n = values->size();
        if (n < 4) {
            throw invalid_argument("Knot number less than 3, unable to generate spline");
            return;
        }
        double mean = std::accumulate(values->begin(), values->end(), 0.0) / n;
        double sq_sum = std::inner_product(values->begin(), values->end(), values->begin(), 0.0);
        double stdev = std::sqrt(sq_sum / n - mean * mean);
        int const Q1 = n / 4;
        int const Q3 = 3 * Q1;
        std::nth_element(values->begin(), values->begin() + Q1, values->end());
        std::nth_element(values->begin() + Q1 + 1, values->begin() + Q3, values->end());
        double quartile = values->at(Q3)-values->at(Q1);
        double p = (stdev < quartile) ? stdev : quartile;
        double h = 1.06*p*pow(n, -0.2);	// 1.06min(std,quartile range)n^(-0.2)
        float xmin = *std::min_element(values->begin(),values->end());	//minimum value
        float xrange = *std::max_element(values->begin(), values->end())-xmin;
        double x_pre = xmin;
        double y_pre = KernelEst(x_pre, n, h, values);
        addKnot(x_pre, y_pre);
        double ymax = y_pre, ymin = y_pre;
        int interval_num = xrange / h * 10;
        double interval= xrange/interval_num;	//10*h interval
        double x = xmin + interval;
        double y = KernelEst(x, n, h, values);
        double x_next,y_next;
        for (int i = 1; i < interval_num; i++) {
            x_next = x + interval;	//x_min+interval*(i+1)
            y_next= KernelEst(x_next, n, h, values);
            double flag = (y - y_pre)*(y - y_next);
            x_pre = x;
            y_pre = y;
            x = x_next;
            y = y_next;
            if (flag< 0) continue;
            addKnot(x_pre, y_pre);
            if (y_pre > ymax) { ymax = y_pre; typicalValue = x_pre;}
            if (y_pre < ymin) ymin = y_pre;
        }
        if (iKnotNum == 1) {
            x = xmin + xrange*0.5;
            y = KernelEst(xmin + xrange*0.5, n, h, values);
            addKnot(x, y);
            if (y > ymax) { ymax = y; typicalValue = x; }
            if (y < ymin) ymin = y;
        }
        addKnot(x_next, y_next);
        if (y_next > ymax) { ymax = y_next; typicalValue = x_next; }
        if (y_next < ymin) ymin = y_next;
        // strech y to 0-1
        if (ymax > ymin && !(fabs(ymax - 1)<VERY_SMALL && fabs(ymin)<VERY_SMALL)) {
            double strechRatio = 1.0 / (ymax - ymin);
            for (int i = 0; i < iKnotNum; i++) vecKnotY[i] = (vecKnotY[i] - ymin) * strechRatio;
        }
        bubbleSort();
        if(1 - vecKnotY[0]>VERY_SMALL) vecKnotY[0] = 0;
        if(1-vecKnotY[iKnotNum-1]>VERY_SMALL) vecKnotY[iKnotNum-1]=0;
        calcSpline();
    }
    Curve::Curve(string covName, vector<Curve> *curves) {
        covariateName = covName;
        dataType = CONTINUOUS;
        iKnotNum = 0;
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        typicalValue = curves->at(0).typicalValue;
        int knotNumSum = 0;
        vector<float> vecXCollect;
        for (int i = 0; i < curves->size(); ++i) {
            knotNumSum += curves->at(i).iKnotNum;
            vecXCollect.insert(vecXCollect.end(), curves->at(i).vecKnotX.begin(), curves->at(i).vecKnotX.end());
        }
        std::sort(vecXCollect.begin(), vecXCollect.end());
        float x_pre = vecXCollect[0];
        float y_pre = curves->at(0).getOptimality(x_pre);
        float x, y, x_next, y_next;
        for (int n = 1; n < curves->size(); ++n) {
            float y_tmp = curves->at(n).getOptimality(x_pre);
            if (y_tmp > y_pre) y_pre = y_tmp;
        }
        addKnot(x_pre, y_pre);
        x = x_pre + (vecXCollect[1] - x_pre)*0.1;
        y = curves->at(0).getOptimality(x);
        for (int n = 1; n < curves->size(); ++n) {
            float y_tmp = curves->at(n).getOptimality(x);
            if (y_tmp > y) y = y_tmp;
        }
        for (int i = 1; i < vecXCollect.size(); ++i) {
            float x_interval = (vecXCollect[i] - vecXCollect[i-1])*0.1;
            int k = 0;
            if (i == 1) k = 1;
            for (; k < 10; k++) {
                x_next = x + x_interval;
                y_next = curves->at(0).getOptimality(x_next);
                for (int n = 1; n < curves->size(); ++n) {
                    float y_tmp = curves->at(n).getOptimality(x_next);
                    if (y_tmp > y_next) y_next = y_tmp;
                }
                if ((y - y_pre)*(y - y_next) < 0);
                else if (fabs(y - y_pre) < VERY_SMALL&&fabs(y - y_next) < VERY_SMALL);
                else addKnot(x, y);
                x_pre = x;
                y_pre = y;
                x = x_next;
                y = y_next;
            }
        }
        addKnot(x_next, y_next);
        bubbleSort();
        if(1 - vecKnotY[0]>VERY_SMALL) vecKnotY[0] = 0;
        if(1-vecKnotY[iKnotNum-1]>VERY_SMALL) vecKnotY[iKnotNum-1]=0;
        calcSpline();
    }

    double Curve::KernelEst(double x,int n,double h, vector<float> *values){
        double sum = 0;
        for (int i = 0; i < values->size(); i++) {
            sum += exp(-pow(x - values->at(i), 2) / h*0.5);
        }
        return sum / (n*h*sqrt(2 * PI));
    }

    Curve::Curve(string covName, vector<int> *values) {	// add rules from data mining-categorical
        int mode = 0;
        int count = 0;
        for (size_t n = 0; n < values->size(); ++n)
        {
            int tmp_mode = values->at(n);
            int tmp_count = std::count(values->begin() + n, values->end(), tmp_mode);
            if (tmp_count > count)
            {
                mode = tmp_mode;
                count = tmp_count;
            }
        }
        covariateName = covName;
        dataType = CATEGORICAL;
        iKnotNum = 1;
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        typicalValue = mode;
        vecKnotX.push_back(mode);
        vecKnotY.push_back(1);
    }

    void Curve::addKnot(double x, double y) {
        for(int i = 0;i<iKnotNum;i++){
            if(fabs(vecKnotX[i]-x)<VERY_SMALL)
                return;
        }
        vecKnotX.push_back(x);
        vecKnotY.push_back(y);
        ++iKnotNum;
    }

    void Curve::updateCurve() {
        iKnotNum = vecKnotX.size();
        if (vecKnotY.size() != iKnotNum) {
            throw invalid_argument("Error in knot coordinates");
        }
        bubbleSort();
        calcSpline();
    }

    double Curve::getOptimality(double envValue) {
        // for categorical value
        if (dataType == CATEGORICAL) {
            for (int i = 0; i < iKnotNum; ++i)
                if (fabs(int(envValue) - int(vecKnotX[i])) < VERY_SMALL)
                    return vecKnotY[i];
            return 0;
        }
        // for continuous value
        double result;
        if (envValue < vecKnotX[0]){
            if(vecKnotY[0]<VERY_SMALL || fabs(vecKnotY[0]-1) < VERY_SMALL) return vecKnotY[0];
            result = vecDY[0] * (envValue - vecKnotX[0]) + vecKnotY[0];
        }
        else if (envValue > vecKnotX[iKnotNum - 1]){
            if(vecKnotY[iKnotNum - 1]<VERY_SMALL || fabs(vecKnotY[iKnotNum - 1]-1) < VERY_SMALL) return vecKnotY[iKnotNum - 1];
            result = vecDY[iKnotNum - 1] * (envValue - vecKnotX[iKnotNum - 1]) + vecKnotY[iKnotNum - 1];
        }
        else {
            int i = 0;
            while (envValue>vecKnotX[i + 1])
                i = i + 1;
            double h0, h1;
            h1 = (vecKnotX[i + 1] - envValue) / vecS[i];
            h0 = h1*h1;
            result = (3.0*h0 - 2.0*h0*h1)*vecKnotY[i];
            result += vecS[i] * (h0 - h0*h1)*vecDY[i];
            h1 = (envValue - vecKnotX[i]) / vecS[i];
            h0 = h1*h1;
            result += (3.0*h0 - 2.0*h0*h1)*vecKnotY[i + 1];
            result -= vecS[i] * (h0 - h0*h1)*vecDY[i + 1];
        }
        if (result>1)
            result = 1;
        else if (result<0)
            result = 0;
        return result;
    }

    int Curve::getKnotNum() {
        iKnotNum = vecKnotX.size();
        if (iKnotNum == vecKnotY.size()) return iKnotNum;
        else throw invalid_argument("Error: inconsistent knotX number and knotY number.");
    }
    string Curve::getCoords() {
        iKnotNum = vecKnotX.size();
        if (iKnotNum != vecKnotY.size())
            throw invalid_argument("Error: inconsistent knotX number and knotY number.");
        string coords = "";
        if (iKnotNum == 0) return coords;
        for (int i = 0; i < iKnotNum; ++i) {
            string tmp;
            if (i == iKnotNum - 1)
                tmp = to_string(vecKnotX[i]) + " " + to_string(vecKnotY[i]);
            else
                tmp = to_string(vecKnotX[i]) + " " + to_string(vecKnotY[i]) + ",";
            coords += tmp;
        }
        return coords;
    }


    void Curve::bubbleSort() {
        double tempx, tempy;
        for (int i = 0; i<iKnotNum; ++i) {
            for (int j = 0; j<iKnotNum - 1; ++j) {
                if (vecKnotX[j]>vecKnotX[j + 1]) {
                    tempx = vecKnotX[j + 1];
                    vecKnotX[j + 1] = vecKnotX[j];
                    vecKnotX[j] = tempx;
                    tempy = vecKnotY[j + 1];
                    vecKnotY[j + 1] = vecKnotY[j];
                    vecKnotY[j] = tempy;
                }
            }
        }
    }

    void Curve::calcSpline() {
        if (iKnotNum < 3) {
            throw invalid_argument("Knot number less than 3, unable to generate spline");
            return;
        }
        vecDY.reserve(iKnotNum);
        vecDY.resize(iKnotNum);
        vecDDY.reserve(iKnotNum);
        vecDDY.resize(iKnotNum);
        vecS.reserve(iKnotNum);
        vecS.resize(iKnotNum);

        int i, j;
        double h0, h1, alpha, beta;

        vecDDY[0] = 0;
        vecDDY[iKnotNum - 1] = 0;
        vecDY[0] = -0.5;
        h0 = vecKnotX[1] - vecKnotX[0];
        vecS[0] = 3.0 * (vecKnotY[1] - vecKnotY[0]) / (2.0 * h0) - vecDDY[0] * h0 / 4.0;
        for (j = 1; j <= iKnotNum - 2; ++j)
        {
            h1 = vecKnotX[j + 1] - vecKnotX[j];
            alpha = h0 / (h0 + h1);
            beta = ((1.0 - alpha)*(vecKnotY[j] - vecKnotY[j - 1])) / h0;
            beta = 3.0*beta + 3.0*alpha*(vecKnotY[j + 1] - vecKnotY[j]) / h1;
            vecDY[j] = -alpha / (2.0 + ((1.0 - alpha)*vecDY[j - 1]));
            vecS[j] = beta - (1.0 - alpha)*vecS[j - 1];
            vecS[j] = vecS[j] / (2.0 + ((1.0 - alpha)*vecDY[j - 1]));
            h0 = h1;
        }
        vecDY[iKnotNum - 1] = (3.0*(vecKnotY[iKnotNum - 1] - vecKnotY[iKnotNum - 2]) / h1 + vecDDY[iKnotNum - 1] * h1 / 2 - vecS[iKnotNum - 2]) / (2.0 + vecDY[iKnotNum - 2]);
        for (j = iKnotNum - 2; j >= 0; j--)
            vecDY[j] = vecDY[j] * vecDY[j + 1] + vecS[j];

        for (j = 0; j <= iKnotNum - 2; ++j)
            vecS[j] = vecKnotX[j + 1] - vecKnotX[j];

        for (j = 0; j <= iKnotNum - 2; ++j)
        {
            h1 = vecS[j] * vecS[j];
            vecDDY[j] = 6.0*(vecKnotY[j + 1] - vecKnotY[j]) / h1 - 2.0*(2.0*vecDY[j] + vecDY[j + 1]) / vecS[j];
        }
        h1 = vecS[iKnotNum - 2] * vecS[iKnotNum - 2];
        vecDDY[iKnotNum - 1] = 6 * (vecKnotY[iKnotNum - 2] - vecKnotY[iKnotNum - 1]) / h1 + 2 * (2 * vecDY[iKnotNum - 1] + vecDY[iKnotNum - 2]) / vecS[iKnotNum - 2];
    }

    int Curve::bsearch(int low, int high, double envValue)
    {
        if (low > high || low == high) return -1;
        int mid = (low + high) / 2;

        if (vecKnotX[mid] < envValue || fabs(vecKnotX[mid] - envValue) < VERY_SMALL) {
            if (vecKnotX[mid + 1] > envValue || fabs(vecKnotX[mid + 1] - envValue) < VERY_SMALL) {
                return mid;
            }
            return bsearch(mid + 1, high, envValue);
        }
        else {// if (vecKnotX[mid]>envValue) {
            if (mid < 1)	return -1;
            if (vecKnotX[mid - 1] < envValue) {
                return mid - 1;
            }
            return bsearch(low, mid - 1, envValue);
        }
    }

    Prototype::Prototype() {
        envConditions.clear();
        properties.clear();
        envConsIsSorted = false;
        envConditionSize = 0;
        uncertainty = 0;
        source = UNKNOWN;
        prototypeBaseName="";
    }

    vector<Prototype> *Prototype::getPrototypesFromSample(string filename, EnvDataset* eds, string prototypeName, string xfield, string yfield) {
        vector<Prototype> *prototypes = new vector<Prototype>;
        ifstream file(filename); // declare file stream:
        if(!file.is_open()) return nullptr;
        string line;
        getline(file, line);
        vector<string> names;
        int pos_X = -1;
        int pos_Y = -1;
        int pos_idName = -1;
        bool id_found = false;
        ParseStr(line, ',', names);
        for (size_t i = 0; i < names.size(); ++i) {
            if (names[i] == xfield||names[i] == "X" || names[i] == "x") {
                pos_X = i;
                break;
            }
        }
        for (size_t i = 0; i < names.size(); ++i) {
            if (names[i] == yfield||names[i] == "Y" || names[i] == "y") {
                pos_Y = i;
                break;
            }
        }

        for (size_t i = 0; i < names.size(); ++i) {
            if (names[i] == "ID" || names[i] == "id") {
                pos_idName = i;
                id_found = true;
                break;
            }
        }

        while (getline(file, line)) {
            vector<string> values;
            ParseStr(line, ',', values);
            const char* xstr = values[pos_X].c_str();
            const char* ystr = values[pos_Y].c_str();
            double x = atof(xstr);
            double y = atof(ystr);
            bool nullSample = false;

            EnvUnit* e = eds->GetEnvUnit(x, y);
            if(e==nullptr) continue;
            for (size_t i = 0; i < e->EnvValues.size(); ++i) {
                if (fabs(e->EnvValues.at(i) - eds->Layers.at(i)->NoDataValue) < VERY_SMALL) {
                    nullSample = true;
                    break;
                }
            }
            if (e != NULL && (!nullSample)) {
                Prototype pt;
                pt.source = SAMPLE;
                for (size_t i = 0; i < eds->Layers.size(); ++i) {
                    EnvLayer *layer = eds->Layers[i];
                    Curve *condition = new Curve(layer->LayerName, x, y, layer);
                    pt.envConditions.push_back(*condition);
                    ++(pt.envConditionSize);
                }
                for (int i = 0; i < values.size(); ++i) {
                    if (i == pos_X || i == pos_Y || i == pos_idName) continue;
                    pt.addProperties(names[i], atof(values[i].c_str()));
                }
                pt.prototypeBaseName = prototypeName;
                if(id_found)
                    pt.prototypeID = values[pos_idName];
                pt.uncertainty = 0;
                prototypes->push_back(pt);
            }
        }
        file.close();
        return prototypes;
    }

    Prototype::Prototype(EnvDataset* eds, int iSoilIDField, string prototypeBasename, OGRFeature* poFeature, int fid){
        envConditions.clear();
        properties.clear();
        envConsIsSorted = false;
        envConditionSize = 0;
        uncertainty = 0;
        source = MAP;
        prototypeBaseName=prototypeBasename;
        if (poFeature->GetGeometryRef() == NULL) return;
        OGREnvelope *extent = new OGREnvelope;
        poFeature->GetGeometryRef()->getEnvelope(extent);
        int globalXMin, globalXMax, globalYMin, globalYMax;
        eds->LayerRef->geoToGlobalXY(extent->MinX, extent->MinY, globalXMin, globalYMax);
        eds->LayerRef->geoToGlobalXY(extent->MaxX, extent->MaxY, globalXMax, globalYMin);
        // iterate over features
        OGRGeometry *poGeometry;
        poGeometry = poFeature->GetGeometryRef();
        vector<vector<float>*> freq;
        for (size_t i = 0; i < eds->Layers.size(); i++) {
            freq.push_back(new vector<float>);
        }
        // iterate over pixel
        int block_size = eds->Layers.at(0)->BlockSize;
        int nx = eds->BlockSizeX;
        int ny = eds->BlockSizeY;
        for (int i = 0; i < block_size; ++i) {
            // check if this block is within the extent of the feature
            if (i == (block_size - 1)) {
                ny = eds->TotalY - i * eds->BlockSizeY;
            }
            int localymin, localxmin, localymax, localxmax;
            eds->Layers.at(0)->baseRef->globalToLocal(i, globalXMin, globalYMin, localxmin, localymin);
            eds->Layers.at(0)->baseRef->globalToLocal(i, globalXMax, globalYMax, localxmax, localymax);
            if (localymin > ny || localymax < 0 || localxmin > nx || localxmax < 0) continue;
            // read the data into all the env layers
            for (size_t k = 0; k < eds->Layers.size(); ++k) {
                eds->Layers.at(k)->ReadByBlock(i);
            }
            int startcol=localxmin > 0 ? localxmin : 0;
            int endcol = localxmax < nx ? localxmax : nx;
            int startrow = localymin > 0 ? localymin : 0;
            int endrow = localymax < ny ? localymax : ny;
#pragma omp parallel
            {
                vector<vector<float>*> freq_private;
                for (size_t i = 0; i < eds->Layers.size(); i++) {
                    freq_private.push_back(new vector<float>);
                }
#pragma omp for schedule(dynamic)
                for (int ncol = startcol; ncol < endcol; ++ncol) {
                    for (int nrow = startrow; nrow < endrow; ++nrow) {
                        int iloc = nrow*nx + ncol;
                        double geoX, geoY;
                        eds->LayerRef->globalXYToGeo(ncol, nrow, geoX, geoY);
                        OGRBoolean within = OGRPoint(geoX, geoY).Within(poGeometry);
                        if (within != 0) {
                            for (size_t k = 0; k < eds->Layers.size(); ++k) {
                                freq_private[k]->push_back(eds->Layers.at(k)->EnvData[iloc]);
                            }
                        }
                    }
                }
#pragma omp critical
                {
                    for (size_t i = 0; i < eds->Layers.size(); i++) {
                        freq[i]->insert(freq[i]->end(),freq_private[i]->begin(),freq_private[i]->end());
                    }
                }
            }
        }
        if (freq[0]->size() < 4) return;
        for (size_t i = 0; i < eds->Layers.size(); i++) {
            if (eds->Layers.at(i)->DataType == CATEGORICAL) {
                vector<int>*values = new vector<int>(freq[i]->begin(), freq[i]->end());
                addConditions(Curve(eds->Layers.at(i)->LayerName, values));
            }
            else {
                addConditions(Curve(eds->Layers.at(i)->LayerName, freq[i]));
            }
        }
        for (int iField = 0; iField < poFeature->GetFieldCount(); iField++)
        {
            if (iField == iSoilIDField) {
                prototypeID = poFeature->GetFieldAsString(iField);
                continue;
            }
            // iterate over fields
            OGRFieldDefn *poFieldDefn = poFeature->GetFieldDefnRef(iField);
            string fieldname = poFieldDefn->GetNameRef();

            SoilProperty sp;
            sp.propertyName = fieldname;
            switch (poFieldDefn->GetType())
            {
            case OFTInteger:
                sp.propertyValue = poFeature->GetFieldAsInteger(iField);
                break;
            case OFTInteger64:
                sp.propertyValue = poFeature->GetFieldAsInteger64(iField);
                break;
            case OFTReal:
                sp.propertyValue = poFeature->GetFieldAsDouble(iField);
                break;
            case OFTString:
                sp.propertyName = sp.propertyName + poFeature->GetFieldAsString(iField);
                sp.propertyValue = NODATA;
                break;
            default:
                sp.propertyName = sp.propertyName + poFeature->GetFieldAsString(iField);
                sp.propertyValue = NODATA;
                break;
            }
            properties.push_back(sp);
        }
        if(iSoilIDField<0) prototypeID = prototypeBasename + to_string(fid);
        //GDALClose(poDS);
    }

    Prototype::Prototype(EnvDataset* eds, int iSoilIDField, string prototypeBasename, OGRLayer* poLayer, vector<int> fids){
        envConditions.clear();
        properties.clear();
        envConsIsSorted = false;
        envConditionSize = 0;
        uncertainty = 0;
        source = MAP;
        prototypeBaseName=prototypeBasename;
        vector<vector<float>*> freq;
        for (size_t i = 0; i < eds->Layers.size(); i++) {
            freq.push_back(new vector<float>);
        }
        for(size_t iFid = 0; iFid<fids.size();iFid++){
            OGRFeature *poFeature = poLayer->GetFeature(fids[iFid]);
            if (poFeature->GetGeometryRef() == NULL) return;
            OGREnvelope *extent = new OGREnvelope;
            poFeature->GetGeometryRef()->getEnvelope(extent);
            int globalXMin, globalXMax, globalYMin, globalYMax;
            eds->LayerRef->geoToGlobalXY(extent->MinX, extent->MinY, globalXMin, globalYMax);
            eds->LayerRef->geoToGlobalXY(extent->MaxX, extent->MaxY, globalXMax, globalYMin);
            // iterate over features
            OGRGeometry *poGeometry;
            poGeometry = poFeature->GetGeometryRef();

            // iterate over pixel
            int block_size = eds->Layers.at(0)->BlockSize;
            int nx = eds->BlockSizeX;
            int ny = eds->BlockSizeY;
            for (int i = 0; i < block_size; ++i) {
                // check if this block is within the extent of the feature
                if (i == (block_size - 1)) {
                    ny = eds->TotalY - i * eds->BlockSizeY;
                }
                int localymin, localxmin, localymax, localxmax;
                eds->Layers.at(0)->baseRef->globalToLocal(i, globalXMin, globalYMin, localxmin, localymin);
                eds->Layers.at(0)->baseRef->globalToLocal(i, globalXMax, globalYMax, localxmax, localymax);
                if (localymin > ny || localymax < 0 || localxmin > nx || localxmax < 0) continue;
                // read the data into all the env layers
                for (int k = 0; k < eds->Layers.size(); ++k) {
                    eds->Layers.at(k)->ReadByBlock(i);
                }
                int startcol=localxmin > 0 ? localxmin : 0;
                int endcol = localxmax < nx ? localxmax : nx;
                int startrow = localymin > 0 ? localymin : 0;
                int endrow = localymax < ny ? localymax : ny;
//#pragma omp declare reduction (merge : std::vector<int> : omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()))
#pragma omp parallel
                {
                    vector<vector<float>*> freq_private;
                    for (size_t i = 0; i < eds->Layers.size(); i++) {
                        freq_private.push_back(new vector<float>);
                    }

    #pragma omp for schedule(dynamic)
                    for (int ncol = startcol; ncol < endcol; ++ncol) {
                        for (int nrow = startrow; nrow < endrow; ++nrow) {
                            int iloc = nrow*nx + ncol;
                            double geoX, geoY;
                            eds->LayerRef->globalXYToGeo(ncol, nrow, geoX, geoY);
                            OGRBoolean within = OGRPoint(geoX, geoY).Within(poGeometry);
                            if (within != 0) {
                                for (size_t k = 0; k < eds->Layers.size(); ++k) {
                                    freq_private[k]->push_back(eds->Layers.at(k)->EnvData[iloc]);
                                }
                            }
                        }
                    }

    #pragma omp critical
                    {
                        for (size_t i = 0; i < eds->Layers.size(); i++) {
                            freq[i]->insert(freq[i]->end(),freq_private[i]->begin(),freq_private[i]->end());
                        }
                    }
                }
            }
            if(iFid==0){
                for (int iField = 0; iField < poFeature->GetFieldCount(); iField++)
                {
                    if (iField == iSoilIDField) {
                        prototypeID = poFeature->GetFieldAsString(iField);
                        continue;
                    }
                    // iterate over fields
                    OGRFieldDefn *poFieldDefn = poFeature->GetFieldDefnRef(iField);
                    string fieldname = poFieldDefn->GetNameRef();

                    SoilProperty sp;
                    sp.propertyName = fieldname;
                    switch (poFieldDefn->GetType())
                    {
                    case OFTInteger:
                        sp.propertyValue = poFeature->GetFieldAsInteger(iField);
                        break;
                    case OFTInteger64:
                        sp.propertyValue = poFeature->GetFieldAsInteger64(iField);
                        break;
                    case OFTReal:
                        sp.propertyValue = poFeature->GetFieldAsDouble(iField);
                        break;
                    case OFTString:
                        sp.propertyName = sp.propertyName + poFeature->GetFieldAsString(iField);
                        sp.propertyValue = NODATA;
                        break;
                    default:
                        sp.propertyName = sp.propertyName + poFeature->GetFieldAsString(iField);
                        sp.propertyValue = NODATA;
                        break;
                    }
                    properties.push_back(sp);
                }
                if(iSoilIDField<0) prototypeID = prototypeBasename + to_string(fids[0]);
            }
        }
        if (freq[0]->size() < 4) return;
        for (size_t i = 0; i < eds->Layers.size(); i++) {
            if (eds->Layers.at(i)->DataType == CATEGORICAL) {
                vector<int>*values = new vector<int>(freq[i]->begin(), freq[i]->end());
                addConditions(Curve(eds->Layers.at(i)->LayerName, values));
            }
            else {
                addConditions(Curve(eds->Layers.at(i)->LayerName, freq[i]));
            }
        }
        //GDALClose(poDS);
    }

    vector<Prototype> *Prototype::getPrototypesFromMining_soilType(string filename, EnvDataset *eds, string soilIDFieldName, string prototypeBasename, QProgressBar *progressBar) {
        vector<Prototype> *prototypes = new vector<Prototype>;
        vector<double> ranges;
        for(size_t i =0; i < eds->Layers.size(); i++){
            ranges.push_back(fabs(eds->Layers.at(i)->Data_Max)>fabs(eds->Layers.at(i)->Data_Min)?fabs(eds->Layers.at(i)->Data_Max):fabs(eds->Layers.at(i)->Data_Min));
        }
        GDALAllRegister();
        GDALDataset *poDS;
        poDS = (GDALDataset*)GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR&GDAL_OF_READONLY&GDAL_OF_SHARED, NULL, NULL, NULL);
        if (poDS == NULL)
        {
            cout << "Open failed." << endl;
            exit(1);
        }
        OGRLayer  *poLayer;
        poLayer = poDS->GetLayer(0);
        // check if shapefile type is polygon
        OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
        if (poFDefn->GetGeomType() != wkbPolygon) {
            cout << "Feature type is not polygon type. Cannot be used for data mining." << endl;
            return nullptr;
        }
        // check the extent of the layer
        OGREnvelope *extent = new OGREnvelope;
        poLayer->GetExtent(extent);
        if (extent->MinX > eds->LayerRef->getXMax() || extent->MaxX < eds->LayerRef->getXMin() ||
            extent->MinY > eds->LayerRef->getYMax() || extent->MaxY < eds->LayerRef->getYMin()) {
            cout << "Feature extent does not match covariate extent. Cannot be used for data mining." << endl;
            return nullptr;
        }
        vector<string> soilIDs;
        int iIdField = -1;
        for (int iField = 0; iField < poFDefn->GetFieldCount(); iField++) {
            OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
            string fieldname = poFieldDefn->GetNameRef();
            if (fieldname == "soilID") {
                iIdField = iField;
                break;
            }
        }
        int feature_count = poLayer->GetFeatureCount();
        bool polyAsTypeFlag = true;
        progressBar->setRange(0,feature_count);
        progressBar->setValue(0);
        if(iIdField>-1){
            polyAsTypeFlag = false;
            for (int feature_num = 0; feature_num < feature_count; feature_num++) {
                soilIDs.push_back(poLayer->GetFeature(feature_num)->GetFieldAsString(iIdField));
            }
            vector<string> polygonLabels = soilIDs;
            std::sort(soilIDs.begin(), soilIDs.end());
            vector<string>::iterator unique_it = std::unique(soilIDs.begin(), soilIDs.end());
            soilIDs.resize(std::distance(soilIDs.begin(), unique_it));
            if(soilIDs.size()==polygonLabels.size()) polyAsTypeFlag = true;
            else {
                for(size_t iSoilType = 0; iSoilType<soilIDs.size(); iSoilType++){
                    vector<int> polyIDs;
                    for(size_t iPolyLabel = 0; iPolyLabel<polygonLabels.size();iPolyLabel++){
                        if(polygonLabels[iPolyLabel]==soilIDs[iSoilType]) polyIDs.push_back(iPolyLabel);
                    }
                    Prototype p(eds,iIdField,prototypeBasename,poLayer,polyIDs);
                    if(p.envConditionSize==0) continue;
                    for (size_t i = 0; i < eds->Layers.size(); i++) {
                        p.envConditions.at(i).range=ranges[i];
                    }
                    prototypes->push_back(p);
                    progressBar->setValue(progressBar->value()+polyIDs.size());
                }
            }
        }
        if(polyAsTypeFlag){
            for (int feature_num = 0; feature_num < feature_count; feature_num++) {
                // iterate over features
                Prototype p(eds,iIdField,prototypeBasename,poLayer->GetFeature(feature_num),feature_num);
                if(p.envConditionSize==0) continue;
                for (size_t i = 0; i < eds->Layers.size(); i++) {
                    p.envConditions.at(i).range=ranges[i];
                }
                prototypes->push_back(p);
                progressBar->setValue(feature_num);
            }
            progressBar->setValue(feature_count);
        }
        return prototypes;
    }
    vector<Prototype> *Prototype::getPrototypesFromMining_polygon(string filename, EnvDataset *eds,string soilIDFieldName, string prototypeBasename, QProgressBar *progressBar) {
        vector<Prototype> *prototypes = new vector<Prototype>;
        vector<double> ranges;
        for(size_t i =0; i < eds->Layers.size(); i++){
            ranges.push_back(fabs(eds->Layers.at(i)->Data_Max)>fabs(eds->Layers.at(i)->Data_Min)?fabs(eds->Layers.at(i)->Data_Max):fabs(eds->Layers.at(i)->Data_Min));
        }
        GDALAllRegister();
        GDALDataset *poDS;
        vector<string> soilIDs;
        poDS = (GDALDataset*)GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR&GDAL_OF_READONLY&GDAL_OF_SHARED, NULL, NULL, NULL);
        if (poDS == NULL)
        {
            cout << "Open failed." << endl;
            exit(1);
        }
        OGRLayer  *poLayer;
        poLayer = poDS->GetLayer(0);
        // check if shapefile type is polygon
        OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
        if (poFDefn->GetGeomType() != wkbPolygon) {
            cout << "Feature type is not polygon type. Cannot be used for data mining." << endl;
            return nullptr;
        }
        // check the extent of the layer
        OGREnvelope *extent = new OGREnvelope;
        poLayer->GetExtent(extent);
        if (extent->MinX > eds->LayerRef->getXMax() || extent->MaxX < eds->LayerRef->getXMin() ||
            extent->MinY > eds->LayerRef->getYMax() || extent->MaxY < eds->LayerRef->getYMin()) {
            cout << "Feature extent does not match covariate extent. Cannot be used for data mining." << endl;
            return nullptr;
        }
        int iIdField = -1;
        for (int iField = 0; iField < poFDefn->GetFieldCount(); iField++) {
            OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
            string fieldname = poFieldDefn->GetNameRef();
            if (fieldname == "soilID") {
                iIdField = iField;
                break;
            }
        }
        int feature_count = poLayer->GetFeatureCount();
        progressBar->setRange(0,feature_count*2);
        progressBar->setValue(0);
//#pragma omp parallel for schedule(dynamic)
        for (int feature_num = 0; feature_num < feature_count; feature_num++) {
            // iterate over features
            Prototype p(eds,iIdField,prototypeBasename,poLayer->GetFeature(feature_num),feature_num);

            if(p.envConditionSize==0) continue;
            for (size_t i = 0; i < eds->Layers.size(); i++) {
                p.envConditions.at(i).range=ranges[i];
            }
            soilIDs.push_back(p.prototypeID);
            prototypes->push_back(p);
            //if(omp_get_thread_num()==0)
                progressBar->setValue(feature_num);
        }
        //if(omp_get_thread_num()==0)
            progressBar->setValue(feature_count);
        std::sort(soilIDs.begin(), soilIDs.end());
        vector<string>::iterator unique_it = std::unique(soilIDs.begin(), soilIDs.end());
        soilIDs.resize(std::distance(soilIDs.begin(), unique_it));
        vector<Prototype> *soiltypes_proto = new vector<Prototype>;
        for (vector<string>::iterator it = soilIDs.begin(); it != soilIDs.end(); ++it) {
            vector<Prototype> tmp_protos;
            vector<Prototype>::iterator it_proto = prototypes->begin();
            while ( it_proto != prototypes->end()) {
                if ((*it_proto).prototypeID == *it) {
                    tmp_protos.push_back(*it_proto);
                    it_proto = prototypes->erase(it_proto);
                }
                else {
                    ++it_proto;
                }
            }
            if (tmp_protos.size() == 1) soiltypes_proto->push_back(tmp_protos[0]);
            else if (tmp_protos.size() > 1) {
                Prototype p;
                p.source = MAP;
                p.prototypeBaseName=prototypeBasename;
                p.prototypeID = *it;
                for (int iCon = 0; iCon < tmp_protos[0].envConditionSize; ++iCon) {
                    string covname = tmp_protos[0].envConditions[iCon].covariateName;
                    vector<Curve>* curves = new vector<Curve>;
                    for (int iProto = 0; iProto < tmp_protos.size(); ++iProto) {
                        curves->push_back(tmp_protos[iProto].envConditions[iCon]);
                    }
                    p.addConditions(Curve(covname, curves));
                    p.envConditions.at(iCon).range=ranges[iCon];
                }
                for (int i = 0; i < tmp_protos[0].properties.size(); i++) {
                    string propertyName = tmp_protos[0].properties[i].propertyName;
                    double value = tmp_protos[0].properties[i].propertyValue;
                    for (int iProto = 1; iProto < tmp_protos.size(); ++iProto) {
                        if(propertyName!=tmp_protos[iProto].properties[i].propertyName||
                            fabs(value- tmp_protos[iProto].properties[i].propertyValue)<VERY_SMALL)
                            continue;
                    }
                    p.addProperties(propertyName, value);
                }
                soiltypes_proto->push_back(p);
            }
            if(omp_get_thread_num()==0) progressBar->setValue(progressBar->value()+tmp_protos.size());
        }
        return soiltypes_proto;
    }


    void Prototype::addConditions(string filename) {
        // read word rule
        TiXmlDocument doc(filename.c_str());
        bool loadOK = doc.LoadFile();
        if (!loadOK) {
            throw invalid_argument("Failed to read xml file");
        }
        TiXmlHandle docHandle(&doc);
        TiXmlHandle curveHandle = docHandle.FirstChildElement("CurveLib");
        for (TiXmlElement* envAttri = curveHandle.FirstChildElement("EnvAttri").ToElement();
            envAttri; envAttri = envAttri->NextSiblingElement("EnvAttri")) {
            TiXmlElement *curveElement = envAttri->FirstChildElement("Curve");
            string covName = envAttri->Attribute("Name");
            DataTypeEnum datatype = getDatatypeFromString(curveElement->FirstChildElement("DataType")->GetText());
            int nodeNum = atoi(curveElement->FirstChildElement("NodeNum")->GetText());
            string coords = curveElement->FirstChildElement("Coordinates")->GetText();
            Curve *c = new Curve(covName, datatype, nodeNum, coords);
            envConditions.push_back(*c);
            ++envConditionSize;
        }
    }

    void Prototype::readPrototype(string filename) {
        // read word rule
        TiXmlDocument doc(filename.c_str());
        bool loadOK = doc.LoadFile();
        if (!loadOK) {
            throw invalid_argument("Failed to read xml file");
        }
        TiXmlHandle docHandle(&doc);
        TiXmlHandle prototypesHandle = docHandle.FirstChildElement("PrototypeLib");
        TiXmlHandle prototypeHandle = prototypesHandle.FirstChildElement("Prototype");
        TiXmlHandle curveHandle = prototypeHandle.FirstChildElement("CurveLib");
        for (TiXmlElement* envAttri = curveHandle.FirstChildElement("EnvAttri").ToElement();
            envAttri; envAttri = envAttri->NextSiblingElement("EnvAttri")) {
            TiXmlElement *curveElement = envAttri->FirstChildElement("Curve");
            string covName = envAttri->Attribute("Name");
            DataTypeEnum datatype = getDatatypeFromString(curveElement->FirstChildElement("DataType")->GetText());
            int nodeNum = atoi(curveElement->FirstChildElement("NodeNum")->GetText());
            string coords = curveElement->FirstChildElement("Coordinates")->GetText();
            Curve *c = new Curve(covName, datatype, nodeNum, coords);
            envConditions.push_back(*c);
            ++envConditionSize;
        }

        TiXmlHandle propsHandle = prototypeHandle.FirstChildElement("PropertyLib");
        for (TiXmlElement* prop = propsHandle.FirstChildElement("Property").ToElement();
            prop; prop = prop->NextSiblingElement("Property")) {
            SoilProperty p;
            p.propertyName = prop->Attribute("Name");
            p.propertyValue = atof(prop->GetText());
            properties.push_back(p);
        }
    }

    void Prototype::addProperties(string propertyName, double propertyValue, DataTypeEnum type) {
        for(int i=0;i<properties.size();i++){
            if(properties[i].propertyName==propertyName){
                properties[i].propertyValue=propertyValue;
                properties[i].soilPropertyType=type;
                return;
            }
        }
        SoilProperty sp;
        sp.propertyName = propertyName;
        sp.propertyValue = propertyValue;
        sp.soilPropertyType = type;
        properties.push_back(sp);
    }


    double Prototype::getProperty(string propName) {
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            if ((*it).propertyName == propName) {
                return (*it).propertyValue;
            }
        }
        return NODATA;
    }
    void Prototype::writeRules(string fileName) {
        char *cFileName = new char[fileName.length() + 1];
        strcpy(cFileName, fileName.c_str());
        char *ext = strlwr(strrchr(cFileName, '.') + 1);
        if (strcmp(ext, "xml") != 0) {
            strcat(cFileName, ".xml");
        }

        TiXmlDocument *doc = new TiXmlDocument();
        TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
        doc->LinkEndChild(pDeclaration);
        TiXmlElement *root_node = new TiXmlElement("CurveLib");
        doc->LinkEndChild(root_node);

        for (auto it = envConditions.begin(); it != envConditions.end(); ++it) {
            // add envAttri to root
            TiXmlElement *envAttri_node = new TiXmlElement("EnvAttri");
            envAttri_node->SetAttribute("Name", (*it).covariateName.c_str());
            root_node->LinkEndChild(envAttri_node);

            // add curve to envAttri
            TiXmlElement *curve_node = new TiXmlElement("Curve");
            envAttri_node->LinkEndChild(curve_node);

            // add nodenum to curve
            TiXmlElement *nodeNum_node = new TiXmlElement("NodeNum");
            curve_node->LinkEndChild(nodeNum_node);
            TiXmlText *nodeNum_text = new TiXmlText(to_string((*it).getKnotNum()).c_str());
            nodeNum_node->LinkEndChild(nodeNum_text);

            TiXmlElement *datatype_node = new TiXmlElement("DataType");
            curve_node->LinkEndChild(datatype_node);
            TiXmlText *datatype_text = new TiXmlText(getDatatypeInString((*it).dataType).c_str());
            datatype_node->LinkEndChild(datatype_text);


            // add coordinates to curve
            TiXmlElement *coords_node = new TiXmlElement("Coordinates");
            curve_node->LinkEndChild(coords_node);
            TiXmlText *coords_text = new TiXmlText((*it).getCoords().c_str());
            coords_node->LinkEndChild(coords_text);
        }
        doc->SaveFile(cFileName);
        delete doc;
    }

    void Prototype::writePrototype(string fileName) {
        char *cFileName = new char[fileName.length() + 1];
        strcpy(cFileName, fileName.c_str());
        char *ext = strlwr(strrchr(cFileName, '.') + 1);
        if (strcmp(ext, "xml") != 0) {
            strcat(cFileName, ".xml");
        }

        TiXmlDocument *doc = new TiXmlDocument();
        TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
        doc->LinkEndChild(pDeclaration);
        TiXmlElement *root_node = new TiXmlElement("PrototypeLib");
        doc->LinkEndChild(root_node);
        TiXmlElement *prototype_node = new TiXmlElement("Prototype");
        root_node->LinkEndChild(prototype_node);
        TiXmlElement *curves_node = new TiXmlElement("CurveLib");
        prototype_node->LinkEndChild(curves_node);

        for (auto it = envConditions.begin(); it != envConditions.end(); ++it) {
            // add envAttri to curveLib
            TiXmlElement *envAttri_node = new TiXmlElement("EnvAttri");
            envAttri_node->SetAttribute("Name", (*it).covariateName.c_str());
            curves_node->LinkEndChild(envAttri_node);

            // add curve to envAttri
            TiXmlElement *curve_node = new TiXmlElement("Curve");
            envAttri_node->LinkEndChild(curve_node);

            // add nodenum to curve
            TiXmlElement *nodeNum_node = new TiXmlElement("NodeNum");
            curve_node->LinkEndChild(nodeNum_node);
            TiXmlText *nodeNum_text = new TiXmlText(to_string((*it).getKnotNum()).c_str());
            nodeNum_node->LinkEndChild(nodeNum_text);

            TiXmlElement *datatype_node = new TiXmlElement("DataType");
            curve_node->LinkEndChild(datatype_node);
            TiXmlText *datatype_text = new TiXmlText(getDatatypeInString((*it).dataType).c_str());
            datatype_node->LinkEndChild(datatype_text);


            // add coordinates to curve
            TiXmlElement *coords_node = new TiXmlElement("Coordinates");
            curve_node->LinkEndChild(coords_node);
            TiXmlText *coords_text = new TiXmlText((*it).getCoords().c_str());
            coords_node->LinkEndChild(coords_text);
        }

        TiXmlElement *props_node = new TiXmlElement("PropertyLib");
        prototype_node->LinkEndChild(props_node);
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            // add soil property to propertyLib
            TiXmlElement *prop_node = new TiXmlElement("Property");
            prop_node->SetAttribute("Name", (*it).propertyName.c_str());
            props_node->LinkEndChild(prop_node);

            TiXmlText *propValue_text = new TiXmlText(to_string((*it).propertyValue).c_str());
            prop_node->LinkEndChild(propValue_text);

        }

        doc->SaveFile(cFileName);
        delete doc;
    }

    TiXmlElement* Prototype::writePrototypeXmlElement() {
        TiXmlElement *root_node = new TiXmlElement("Prototype");
        root_node->SetAttribute("BaseName", prototypeBaseName.c_str());
        root_node->SetAttribute("ID",prototypeID.c_str());
        root_node->SetAttribute("Source",PrototypeSource_str[source]);
        TiXmlElement *curves_node = new TiXmlElement("CurveLib");
        root_node->LinkEndChild(curves_node);

        for (auto it = envConditions.begin(); it != envConditions.end(); ++it) {
            // add envAttri to curveLib
            TiXmlElement *envAttri_node = new TiXmlElement("EnvAttri");
            envAttri_node->SetAttribute("Name", (*it).covariateName.c_str());
            curves_node->LinkEndChild(envAttri_node);
            // add typical value
            TiXmlElement *typicalV_node = new TiXmlElement("TypicalValue");
            envAttri_node->LinkEndChild(typicalV_node);
            TiXmlText *typicalV_text = new TiXmlText(to_string((*it).typicalValue).c_str());
            typicalV_node->LinkEndChild(typicalV_text);

            // add curve to envAttri
            TiXmlElement *curve_node = new TiXmlElement("Curve");
            envAttri_node->LinkEndChild(curve_node);

            // add nodenum to curve
            TiXmlElement *nodeNum_node = new TiXmlElement("NodeNum");
            curve_node->LinkEndChild(nodeNum_node);
            TiXmlText *nodeNum_text = new TiXmlText(to_string((*it).getKnotNum()).c_str());
            nodeNum_node->LinkEndChild(nodeNum_text);

            TiXmlElement *datatype_node = new TiXmlElement("DataType");
            curve_node->LinkEndChild(datatype_node);
            TiXmlText *datatype_text = new TiXmlText(getDatatypeInString((*it).dataType).c_str());
            datatype_node->LinkEndChild(datatype_text);

            // add range to curve
            TiXmlElement *range_node = new TiXmlElement("Range");
            curve_node->LinkEndChild(range_node);
            TiXmlText *range_text = new TiXmlText(to_string((*it).range).c_str());
            range_node->LinkEndChild(range_text);

            // add coordinates to curve
            TiXmlElement *coords_node = new TiXmlElement("Coordinates");
            curve_node->LinkEndChild(coords_node);
            TiXmlText *coords_text = new TiXmlText((*it).getCoords().c_str());
            coords_node->LinkEndChild(coords_text);
        }

        TiXmlElement *props_node = new TiXmlElement("PropertyLib");
        root_node->LinkEndChild(props_node);
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            // add soil property to propertyLib
            TiXmlElement *prop_node = new TiXmlElement("Property");
            prop_node->SetAttribute("Name", (*it).propertyName.c_str());
            prop_node->SetAttribute("Type", getDatatypeInString((*it).soilPropertyType).c_str());
            props_node->LinkEndChild(prop_node);

            TiXmlText *propValue_text = new TiXmlText(to_string((*it).propertyValue).c_str());
            prop_node->LinkEndChild(propValue_text);

        }

        return root_node;
    }


    void Prototype::sortEnvCons(vector<string> layernames) {
        if (layernames.size() > envConditionSize)
            return;
        else {
            vector<Curve> tempCurves = envConditions;
            bool hasLayer;
            for(int i = 0; i<layernames.size();i++){
                hasLayer = false;
                for(int j =0; j<envConditionSize;j++){
                    if(tempCurves[j].covariateName==layernames[i]){
                        hasLayer = true;
                        envConditions[i] = tempCurves[j];
                    }
                }
                if(!hasLayer){
                    envConsIsSorted = false;
                    envConditions = tempCurves;
                    return;
                }
            }
            envConsIsSorted = true;
            if(layernames.size()<envConditionSize){
                int k = layernames.size();
                for(int i=0; i<envConditionSize;i++){
                    hasLayer = false;
                    for(int j = 0; j<layernames.size();j++){
                        if(tempCurves[i].covariateName==envConditions[j].covariateName){
                            hasLayer = true;
                            break;
                        }
                    }
                    if(!hasLayer){
                        envConditions[k] = tempCurves[i];
                        k++;
                    }
                }
            }
        }
    }

    double Prototype::calcSimi(EnvUnit *e) {
        if (!envConsIsSorted) sortEnvCons(e->LayerNames);
        if (!envConsIsSorted) {
            throw invalid_argument("Error: inconsistant rule names and layer names");
            return -1;
        }
        double tmpOptimity;
        double minOptimity = envConditions[0].getOptimality(e->EnvValues[0]);
        for (int i = 1; i < envConditionSize; ++i) {
            tmpOptimity = envConditions[i].getOptimality(e->EnvValues[i]);
            if (tmpOptimity < minOptimity) minOptimity = tmpOptimity;
        }
        return minOptimity;
    }

    double Prototype::calcSimi_preChecked(EnvUnit *e) {
        if (!envConsIsSorted) {
            return -1;
        }
        double tmpOptimity;
        double minOptimity = envConditions[0].getOptimality(e->EnvValues[0]);
        for (int i = 1; i < envConditionSize; ++i) {
            tmpOptimity = envConditions[i].getOptimality(e->EnvValues[i]);
            if (tmpOptimity < minOptimity) minOptimity = tmpOptimity;
        }
        return minOptimity;
    }
    bool Prototype::checkEnvConsIsSorted(EnvDataset *eds) {
        if (!envConsIsSorted) sortEnvCons(eds->LayerNames);
        return envConsIsSorted;
    }
    double Prototype::getOptimality(vector<Prototype> *prototypes, EnvUnit *e, string soilPropertyName, double soilTypeTag) {
        double instanceSimi = -1;
        double tmpSimi;
        for (auto it = prototypes->begin(); it != prototypes->end(); ++it) {
            if (fabs((*it).getProperty(soilPropertyName) - soilTypeTag) < VERY_SMALL) {
                tmpSimi = (*it).calcSimi(e);
                if (instanceSimi < tmpSimi) instanceSimi = tmpSimi;
            }
        }
        return instanceSimi;
    }

    Exception::Exception(string sExceptionType, double x, double y) {
        transform(sExceptionType.begin(), sExceptionType.end(), sExceptionType.begin(), toupper);
        if (sExceptionType == "OCCURRENCE")
            exceptionType = OCCURRENCE;
        else if (sExceptionType == "EXCLUSION")
            exceptionType = EXCLUSION;
        centralX = x;
        centralY = y;
        distanceDecayEnabled = false;
        impactRadius = 0;
        envConditions.clear();
        properties.clear();
        envConsIsSorted = false;
        envConditionSize = 0;
    }
    Exception::Exception(string sExceptionType, double x, double y, bool decay, double radius) {
        transform(sExceptionType.begin(), sExceptionType.end(), sExceptionType.begin(), toupper);
        if (sExceptionType == "OCCURRENCE")
            exceptionType = OCCURRENCE;
        else if (sExceptionType == "EXCLUSION")
            exceptionType = EXCLUSION;
        centralX = x;
        centralY = y;
        distanceDecayEnabled = decay;
        impactRadius = radius;
        envConditions.clear();
        properties.clear();
        envConsIsSorted = false;
        envConditionSize = 0;
    }

    void Exception::addProperties(string propertyName, double propertyValue/*, DataTypeEnum type*/) {
        SoilProperty sp;
        sp.propertyName = propertyName;
        sp.propertyValue = propertyValue;
        //sp.soilPropertyType = type;
        properties.push_back(sp);
    }


    double Exception::getProperty(string propName) {
        double value = NODATA;
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            if ((*it).propertyName == propName) {
                value = (*it).propertyValue;
            }
        }
        return value;
    }
    void Exception::writeRules(string fileName) {
        char *cFileName = new char[fileName.length() + 1];
        strcpy(cFileName, fileName.c_str());
        char *ext = strlwr(strrchr(cFileName, '.') + 1);
        if (strcmp(ext, "xml") != 0) {
            strcat(cFileName, ".xml");
        }

        TiXmlDocument *doc = new TiXmlDocument();
        TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
        doc->LinkEndChild(pDeclaration);
        TiXmlElement *root_node = new TiXmlElement("CurveLib");
        doc->LinkEndChild(root_node);

        for (auto it = envConditions.begin(); it != envConditions.end(); ++it) {
            // add envAttri to root
            TiXmlElement *envAttri_node = new TiXmlElement("EnvAttri");
            envAttri_node->SetAttribute("Name", (*it).covariateName.c_str());
            root_node->LinkEndChild(envAttri_node);

            // add curve to envAttri
            TiXmlElement *curve_node = new TiXmlElement("Curve");
            envAttri_node->LinkEndChild(curve_node);

            // add nodenum to curve
            TiXmlElement *nodeNum_node = new TiXmlElement("NodeNum");
            curve_node->LinkEndChild(nodeNum_node);
            TiXmlText *nodeNum_text = new TiXmlText(to_string((*it).getKnotNum()).c_str());
            nodeNum_node->LinkEndChild(nodeNum_text);

            TiXmlElement *datatype_node = new TiXmlElement("DataType");
            curve_node->LinkEndChild(datatype_node);
            TiXmlText *datatype_text = new TiXmlText(getDatatypeInString((*it).dataType).c_str());
            datatype_node->LinkEndChild(datatype_text);


            // add coordinates to curve
            TiXmlElement *coords_node = new TiXmlElement("Coordinates");
            curve_node->LinkEndChild(coords_node);
            TiXmlText *coords_text = new TiXmlText((*it).getCoords().c_str());
            coords_node->LinkEndChild(coords_text);
        }
    }
    void Exception::sortEnvCons(vector<string> layernames) {
        if (layernames.size() != envConditionSize)
            throw invalid_argument("Error: inconsistant rule number and layer number");
        envConsIsSorted = true;
        for (int i = 0; i < envConditionSize; ++i) {
            if (envConditions[i].covariateName != layernames[i]) {
                envConsIsSorted = false;
                break;
            }
        } if (!envConsIsSorted) {
            vector<Curve> tempCurves = envConditions;
            for (int i = 0; i < envConditionSize; ++i) {
                for (int j = 0; j < envConditionSize; ++j) {
                    if (tempCurves[i].covariateName == layernames[j]) {
                        envConditions[j] = tempCurves[i];
                    }
                }
            }
        }
        envConsIsSorted = true;
    }

    double Exception::calcSimi(EnvUnit *e) {
        if (e->EnvValues.size() != envConditionSize)
            throw invalid_argument("Error: inconsistant rule number and layer number");
        if (!envConsIsSorted) sortEnvCons(e->LayerNames);
        double tmpOptimity;
        double minOptimity = 1;
        for (int i = 0; i < envConditionSize; ++i) {
            tmpOptimity = envConditions[i].getOptimality(e->EnvValues[i]);
            if (tmpOptimity < minOptimity) minOptimity = tmpOptimity;
        }
        return minOptimity;
    }


    double Exception::calcOccurrExcluSim(vector<Prototype> *instances, EnvUnit *e, string soilPropertyName, double soilTypeTag, EnvDataset *eds, double zFactor) {
        if (exceptionType != OCCURRENCE && exceptionType != EXCLUSION) return -1;
        if (fabs(getProperty(soilPropertyName) - soilTypeTag) > VERY_SMALL) return -1;
        EnvLayer *dem = eds->getDEM();
        double dist = topoDistanceXY(e->Loc->X, e->Loc->Y, dem, zFactor);
        if (dist > impactRadius) return -1;
        double occurrenceSimi = calcSimi(e);
        // distance Decay enabled
        if (distanceDecayEnabled) {
            double dX = e->Loc->X;
            double dY = e->Loc->Y;
            if (fabs(dY - centralY) > VERY_SMALL || fabs(dX - centralX) > VERY_SMALL) { //the coordinates are identical
                double dInterval = dem->CellSize;
                double dK = 1;
                double dXStart = dX;
                double dYStart = dY;
                double xx = dX;
                double yx = dY;
                double dBoundDist = 0;
                double dBoundSim = 0;
                double searchDist = impactRadius * 2;

                if (fabs(centralX - dX)>VERY_SMALL)
                    dK = (dY - centralY) / (dX - centralX);
                double distToCenter = dist;
                if (fabs(distToCenter - searchDist) > dInterval) {
                    do {
                        if (fabs(dX - centralX)>VERY_SMALL) {
                            if (centralX<dX)
                                xx = dXStart + dInterval / sqrt(pow(dK, 2) + 1);
                            else
                                xx = dXStart - dInterval / sqrt(pow(dK, 2) + 1);
                            if (centralY<dY)
                                yx = dYStart + fabs(dK*dInterval / sqrt(pow(dK, 2) + 1));
                            else
                                yx = dYStart - fabs(dK*dInterval / sqrt(pow(dK, 2) + 1));
                        }
                        else {
                            xx = dXStart;
                            if (centralY<dY)
                                yx = dYStart + dInterval;
                            else
                                yx = dYStart - dInterval;
                        } if (dem != nullptr)
                            distToCenter = topoDistanceXY(xx, yx, dem, zFactor);
                        else
                            distToCenter = sqrt(pow((xx - centralX), 2) + pow((yx - centralY), 2));
                        if (distToCenter == -1) {//out of boundary
                            distToCenter = topoDistanceXY(xx, yx, dem, zFactor);
                            distToCenter = topoDistanceXY(dXStart, dYStart, dem, zFactor);
                            xx = dXStart;
                            yx = dYStart;
                            break;
                        }
                        else if (distToCenter == -2) {
                            distToCenter = topoDistanceXY(dXStart, dYStart, dem, zFactor);
                            xx = dXStart;
                            yx = dYStart;
                            break;
                        }
                        else if (fabs(distToCenter - searchDist) <= dInterval)
                            break;
                        dXStart = xx;
                        dYStart = yx;
                    } while (true);
                }
                EnvUnit *boundaryEnvUnit = eds->GetEnvUnit(xx, yx);
                dBoundSim = Prototype::getOptimality(instances, boundaryEnvUnit, soilPropertyName, soilTypeTag);
                dBoundDist = distToCenter - dist;

                if (dist > 0 && dBoundDist > 0)
                    occurrenceSimi = distSimilarity(occurrenceSimi, dist, dBoundSim, dBoundDist, distanceDecayFactor);
                else if (dBoundDist == 0)
                    occurrenceSimi = dBoundSim;
            }
        }
        return occurrenceSimi;
    }

    double Exception::getOccurrenceOpt(vector<Exception> *occurences, vector<Prototype> *instances, EnvUnit *e, string soilPropertyName, double soilTypeTag, EnvDataset *eds, double zFactor) {
        double occurrenceSimi = -1;
        double tmpSimi;
        for (auto it = occurences->begin(); it != occurences->end(); ++it) {
            tmpSimi = (*it).calcOccurrExcluSim(instances, e, soilPropertyName, soilTypeTag, eds, zFactor);
            if (fabs(tmpSimi + 1) > VERY_SMALL) {
                // find the biggest occurrence optimity value
                if (occurrenceSimi < tmpSimi)  occurrenceSimi = tmpSimi;
            }
        }
        return occurrenceSimi;
    }


    double Exception::getExclusionOpt(vector<Exception> *exlusions, vector<Prototype> *instances, EnvUnit *e, string soilPropertyName, double soilTypeTag, EnvDataset *eds, double zFactor) {
        double exclusionSimi = 1;
        double tmpSimi;
        for (auto it = exlusions->begin(); it != exlusions->end(); ++it) {
            tmpSimi = (*it).calcOccurrExcluSim(instances, e, soilPropertyName, soilTypeTag, eds, zFactor);
            if (fabs(tmpSimi - 1) > VERY_SMALL) {
                // find the biggest occurrence optimity value
                if (exclusionSimi < tmpSimi)  exclusionSimi = tmpSimi;
            }
        }
        return exclusionSimi;
    }


    double  Exception::topoDistanceXY(double dX, double dY, EnvLayer *layer_DEM, double dZFactor)
    {
        double dDist = 0;
        float fNoData = layer_DEM->NoDataValue;
        int iR1, iC1, iR2, iC2;
        layer_DEM->baseRef->geoToGlobalXY(centralX, centralY, iC1, iR1);
        layer_DEM->baseRef->geoToGlobalXY(dX, dY, iC2, iR2);

        if (iR1<0 || iR1 + 1>layer_DEM->baseRef->getYSize() || iR2<0 || iR2 + 1>layer_DEM->baseRef->getYSize()
            || iC1<0 || iC1 + 1>layer_DEM->baseRef->getXSize() || iC2<0 || iC2 + 1>layer_DEM->baseRef->getXSize())
        {
            return -1;
        }
        double dH1 = layer_DEM->baseRef->getValue(iC1, iR1);
        double dH2 = layer_DEM->baseRef->getValue(iC2, iR2);

        double dInterval = layer_DEM->CellSize;
        int iStep = sqrt(pow((centralX - dX), 2) + pow((centralY - dY), 2)) / dInterval;
        double dK = 1;

        double dXStart = centralX;
        double dYStart = centralY;
        double dHStart = dH1;
        double xx;
        double yx;

        //if (sqrt(pow((dX1-dX2),2)+pow((dY1-dY2),2))>fThreshold)
        //	return -1;
        if (fabs(dH1 - fNoData)<VERY_SMALL || fabs(dH2 - fNoData)<VERY_SMALL)//?"="
            return -2;



        if (fabs(dX - centralX)>VERY_SMALL)
            dK = (dY - centralY) / (dX - centralX);
        for (int i = 0; i<iStep; ++i)
        {
            if (fabs(dX - centralX)>VERY_SMALL)
            {
                if (centralX<dX)
                    xx = dXStart + dInterval / sqrt(pow(dK, 2) + 1);
                else
                    xx = dXStart - dInterval / sqrt(pow(dK, 2) + 1);
                if (centralY<dY)
                    yx = dYStart + fabs(dK*dInterval / sqrt(pow(dK, 2) + 1));
                else
                    yx = dYStart - fabs(dK*dInterval / sqrt(pow(dK, 2) + 1));
            }
            else
            {
                xx = dXStart;
                if (centralY<dY)
                    yx = dYStart + dInterval;
                else
                    yx = dYStart - dInterval;
            }

            int row, col;
            layer_DEM->baseRef->geoToGlobalXY(yx, xx, col, row);
            double h = layer_DEM->baseRef->getValue(col, row);

            //dDist+=sqrt(pow((dXStart-dX2),2),pow((dYStart-dY2),2)+pow((dHstart-dH2)*fZFactor,2));
            dDist += sqrt(pow((dXStart - xx), 2) + pow((dYStart - yx), 2) + pow((dHStart - h)*dZFactor, 2));
            dXStart = xx;
            dYStart = yx;
            dHStart = h;
        }
        return dDist;
    }

    double Exception::distSimilarity(double dSim, double dDist, double dBoundSim, double dBoundDist, double dDistDecayFactor)
    {
        float w1 = pow(dDist, -dDistDecayFactor);
        float w2 = pow(dBoundDist, -dDistDecayFactor);
        if (exceptionType == OCCURRENCE)
            return (dSim * w1 + dBoundSim * w2) / (w1 + w2);
        else //if (exceptionType == EXCLUSION)
            return ((1 - dSim) * w1 + dBoundSim * w2) / (w1 + w2);
    }

    double Exception::getOptimality(vector<Exception> *occurrences, vector<Exception> *exclusions, vector<Prototype> *instances, EnvUnit *e, string soilPropertyName, double soilTypeTag, EnvDataset *eds, double zFactor) {
        double instanceSimi = Prototype::getOptimality(instances, e, soilPropertyName, soilTypeTag);
        if (fabs(instanceSimi + 1) < VERY_SMALL) return -1;

        double occurrenceSimi = getOccurrenceOpt(occurrences, instances, e, soilPropertyName, soilTypeTag, eds, zFactor);
        double exclusionSimi = getOccurrenceOpt(exclusions, instances, e, soilPropertyName, soilTypeTag, eds, zFactor);

        if (fabs(occurrenceSimi + 1) < VERY_SMALL&&fabs(exclusionSimi - 1) < VERY_SMALL)
            return instanceSimi;
        if (fabs(occurrenceSimi + 1) > VERY_SMALL&&fabs(exclusionSimi - 1) < VERY_SMALL)
            return occurrenceSimi;
        if (fabs(occurrenceSimi + 1) < VERY_SMALL&&fabs(exclusionSimi - 1) > VERY_SMALL)
            return exclusionSimi;
        if (fabs(occurrenceSimi + 1) > VERY_SMALL&&fabs(exclusionSimi - 1) > VERY_SMALL)
            if (occurrenceSimi < exclusionSimi) return occurrenceSimi;
            else return exclusionSimi;
    }

    void Inference::inferMap(EnvDataset *eds, vector<Prototype>* prototypes, string targetVName, double threshold, string outSoilFile, string outUncerFile, QProgressBar *progressBar) {
        // check the consistency of prototype rules and envdataset
        for (auto it = prototypes->begin(); it != prototypes->end(); ++it) {
            if (!(*it).checkEnvConsIsSorted(eds)) {
                throw invalid_argument("Prototype inconsistent with layers");
                return;
            }
        }
        int Xstart, Ystart;
        int nx, ny;
        double xa, ya;
        int block_size = eds->Layers.at(0)->BlockSize;
        nx = eds->BlockSizeX;
        ny = eds->BlockSizeY;
        float *uncertaintyValue, *predictedValue;
        uncertaintyValue = new float[nx*ny];
        predictedValue = new float[nx*ny];
        BaseIO *outSoilMap = new BaseIO(*(eds->LayerRef));
        outSoilMap->setFileName(outSoilFile);
        outSoilMap->setNodataValue(NODATA);
        BaseIO *outUncerMap = new BaseIO(*(eds->LayerRef));
        outUncerMap->setFileName(outUncerFile);
        outUncerMap->setNodataValue(NODATA);
        double *envValues = new double[MAXLN_LAYERS];
        double *nodata = new double[MAXLN_LAYERS];
        for (int k = 0; k < eds->Layers.size(); k++) {
            nodata[k] = eds->Layers.at(k)->NoDataValue;
        }
        //EnvUnit *e;
        progressBar->setMinimum(0);
        progressBar->setMaximum(block_size*100);

        for (int i = 0; i < block_size; ++i) {
            // for each block, this circle is to ensure every block is processed

            if (i == (block_size - 1)) {
                ny = eds->TotalY - i * eds->BlockSizeY;
            }
            progressBar->setValue(i*100);
            // read the data into all the env layers
            for (int k = 0; k < eds->Layers.size(); ++k) {
                eds->Layers.at(k)->ReadByBlock(i);
            }
#pragma omp parallel for schedule(dynamic)
            for (int n = 0; n < nx*ny; ++n) {
                // for each unit in the block, calculate their predicted value and uncertainty
                bool validEnvUnitFlag = TRUE;

                long long int pixelCount = nx*ny;
                double progressPara = 100.0/pixelCount;
                if (n % int(pixelCount*0.01)==0 && n > 0) {
                    qInfo()<<predictedValue[n-1] << uncertaintyValue[n-1];
                    if(omp_get_thread_num()==0)
                        progressBar->setValue(n*progressPara+i*100);
                    //cout <<n<<" "<<nx*ny<<" "<<omp_get_thread_num()<<endl;
                }
                //e = new EnvUnit();
                for (int k = 0; k < eds->Layers.size(); ++k) {
                    // get the values at all layers for the unit
                    float value = eds->Layers.at(k)->EnvData[n];
                    if (fabs(value - nodata[k]) < VERY_SMALL || value<nodata[k]) {
                        validEnvUnitFlag = FALSE;
                        break;
                    }
                    envValues[k] = value;
                    //e->AddEnvValue(value);
                }
                if (!validEnvUnitFlag) {
                    uncertaintyValue[n] = NODATA;
                    predictedValue[n] = NODATA;
                    //delete e;
                    continue;
                }
                double valueSum = 0;
                double weightSum = 0;
                double maxSimi = 0;
                // calculate predicted value
                for (vector<Prototype>::iterator it = prototypes->begin(); it != prototypes->end(); ++it) {
                    // calculate similarity to prototype
                    double tmpOptimity;
                    double minOptimity = (*it).envConditions[0].getOptimality(envValues[0]);
                    for (int i = 1; i < eds->Layers.size(); ++i) {
                        tmpOptimity = (*it).envConditions[i].getOptimality(envValues[i]);
                        if (tmpOptimity < minOptimity) minOptimity = tmpOptimity;
                    }
                    double simi = minOptimity;
                    //double simi = (*it).calcSimi_preChecked(e);
                    if (simi > threshold) {
                        valueSum += simi*(*it).getProperty(targetVName);
                        weightSum += simi;
                        if (simi > maxSimi)
                            maxSimi = simi;
                    }
                }
                if (fabs(weightSum) < VERY_SMALL) {
                    uncertaintyValue[n] = NODATA;
                    predictedValue[n] = NODATA;
                }
                else {
                    predictedValue[n] = valueSum / weightSum;
                    uncertaintyValue[n] = 1 - maxSimi;
                }
            }

            eds->LayerRef->localToGlobal(i, 0, 0, Xstart, Ystart);
            outSoilMap->write(Xstart, Ystart, ny, nx, predictedValue);
            outUncerMap->write(Xstart, Ystart, ny, nx, uncertaintyValue);//

        }
        delete []envValues;
        delete []nodata;
        delete predictedValue;
        delete uncertaintyValue;
    }

    void Inference::iPSMInferSoil(vector<string> filenames, vector<string> datatypes, vector<string> layernames,
        double threshold, string sampleFilename, string targetVName, string idName,
        string outSoilFile, string outUncerFile, double ramEfficient,QProgressBar *progressBar) {
        EnvDataset *eds = new EnvDataset(filenames, datatypes, layernames, ramEfficient);
        vector<Prototype> *prototypes = Prototype::getPrototypesFromSample(sampleFilename, eds);
        inferMap(eds, prototypes, targetVName, threshold, outSoilFile, outUncerFile,progressBar);
    }
}
