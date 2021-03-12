#include"BaseIO.h"
#define MAXLN 4096
BaseIO::BaseIO(string filename) {
	openSuccess = true;
	if (strcmp(strrchr(filename.c_str(), '.'), ".3dr") == 0) {
		is3dr = true;
		fileName = filename;
		char utilityString[50];
		float utilityFloat = 0;
		int utilityInt = 0;
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
			openSuccess = false;
			return;

		}
		if(getMaxMin>1)
			dataRange = dataMax - dataMin;
		else {}//todo get max and min
		inCurLoc = firstDataByte;
		dataEndLoc = xSize*ySize * sizeof(float) + firstDataByte;
		fclose(threeDRfp);
	}
	else {
		is3dr = false;
		GDALAllRegister();
		fh = GDALOpen(filename.c_str(), GA_ReadOnly);
		if (fh == NULL) {
			cout << "Error opening file " << filename << endl;
			openSuccess = false;
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
		eBDataType = GDALGetRasterDataType(bandh);
		if (eBDataType == GDT_Unknown) eBDataType = GDT_Float32;
		if (eBDataType == GDT_UInt16 || eBDataType == GDT_Int16) fileDataType = SHORT_TYPE;
		if (eBDataType == GDT_UInt32 || eBDataType == GDT_Int32) fileDataType = LONG_TYPE;
		if (eBDataType == GDT_Float32 || eBDataType == GDT_Float64) fileDataType = FLOAT_TYPE;
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
		fseek(threeDRfp, inCurLoc + (xSize*yStart +xStart)* sizeof(float), SEEK_SET);
		fread(dest, sizeof(float), numRows*numCols, threeDRfp);
		fclose(threeDRfp);
	}
	else {
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


void BaseIO::write(long xStart, long yStart, long numRows, long numCols, float *source, string writeFilename) {
	if (writeFilename == "")
		writeFilename = fileName;
	if (is3dr) {
		if (writeFilename.substr(writeFilename.length() - 3) != "3dr")
			writeFilename = writeFilename + ".3dr";
		if (!isFileInititialized) {
			if ((threeDRfp = fopen(writeFilename.c_str(), "w")) == NULL) {
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
			/*if (strlen(FileType)>0) {
				fprintf(fp, "%s %s\n", "FileType: ", FileType);
				NumberOfRecords++;
			}
			if (strlen(GridUnits)>0) {
				fprintf(fp, "%s %s\n", "GridUnits: ", GridUnits);
				NumberOfRecords++;
			}
			if (strlen(DataUnits)>0) {
				fprintf(fp, "%s %s\n", "DataUnits: ", DataUnits);
				NumberOfRecords++;
			}
			*/

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
		threeDRfp = fopen(writeFilename.c_str(), "ab");
		fseek(threeDRfp, firstDataByte + (xSize*yStart + xStart) * sizeof(float), SEEK_SET);
		fwrite(source, sizeof(float), numRows*numCols, threeDRfp);
		fclose(threeDRfp);
	}
	else {
		char *cFileName = new char[writeFilename.length() + 1];
		strcpy(cFileName, writeFilename.c_str());

		fflush(stdout);
		char **papszOptions = NULL;
		const char *extension_list[6] = { ".tif", ".img", ".sdat", ".bil", ".bin", ".tiff" };  // extension list --can add more
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
			else if (index == 1) { // .img files.  Refer to http://www.gdal.org/frmt_hfa.html where COMPRESSED = YES are create options for ERDAS .img files
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
					printf("Setting BIGTIFF, File: %s, Anticipated size (GB):%.2f\n", fileName, fileGB);
				}
			}
			if (eBDataType == GDT_Unknown) eBDataType = GDT_Float32;
			fh = GDALCreate(hDriver, fileName.c_str(), xSize, ySize, 1, eBDataType, NULL);
			GDALSetProjection(fh, GDALGetProjectionRef(fh));
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
		if (eBDataType == GDT_Unknown) eBDataType = GDT_Float32;
		CPLErr result = GDALRasterIO(bandh, GF_Write, xStart, yStart, numCols, numRows,
			source, numCols, numRows, eBDataType, 0, 0);
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
bool BaseIO::globalToLocal(int blockRank, int globalX, int globalY, int &localX, int &localY){
	localX = globalX;
	localY = globalY - blockRank * blockRows;
	//  For the last process ny is greater than the size of the other partitions so rank*ny does not get the row right.
	//  totaly%size was added to ny for the last partition, so the size of partitions above is actually
	//  ny - totaly%size  (the remainder from dividing totaly by size).
	if (blockRank == blockSize - 1 && blockRank > 0) {
		localY = globalY - blockRank * (blockRows - ySize % blockRank);
	}
	return isInPartition(localX, localY);
}
void BaseIO::localToGlobal(int blockRank, int localX, int localY, int &globalX, int &globalY) {
	globalX = localX;
	globalY = blockRows * blockRank + localY;
	//localY = blockRows * blockRank + localY;
	//localToGlobal(localX, localY, globalX, globalY);
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
	} else {
		MEMORYSTATUSEX statusex;
		unsigned long long avl = 0;
		unsigned long long memoryForOneBlock=0;
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
		} else {
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
void BaseIO::blockNull(){
	blockRows = ySize;
	blockSize = 1;
	blockX = xSize;
	blockY = ySize;
	blockIsInitialized = true;
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