#include "EnvDataset.h"

namespace solim {
	EnvDataset::EnvDataset()
		: LayerRef(nullptr), CellSize(-9999.), CellSizeY(-9999.), XMin(-9999.), XMax(-9999.),
		YMin(-9999.), YMax(-9999.), XStart(0), YStart(0), TotalX(0), TotalY(0), CalcArea(0) {
	}

	EnvDataset::EnvDataset(vector<string>& envLayerFilenames, vector<string>& datatypes, vector<string>& layernames, double ramEfficent)
		: LayerRef(nullptr), CellSize(-9999.), CellSizeY(-9999.), XMin(-9999.), XMax(-9999.),
		YMin(-9999.), YMax(-9999.), XStart(0), YStart(0), TotalX(0), TotalY(0), CalcArea(0) {
		if (layernames.size() == 0) {
			for (size_t i = 0; i < envLayerFilenames.size(); i++) {
				string layername = "";
				string filename = envLayerFilenames[i];
				if (!filename.empty()) {
					std::size_t first = filename.find_last_of('/');
					if (first == std::string::npos) {
						first = filename.find_last_of('\\');
					}
					std::size_t end = filename.find_last_of('.');
					if (end == std::string::npos) end = filename.size();
					layername = filename.substr(first + 1, end - first - 1).c_str();
				}
				layernames.push_back(layername);
			}
			this->LayerNames = layernames;
		}
		ReadinLayers(envLayerFilenames, datatypes, ramEfficent);
	}

	EnvDataset::~EnvDataset() {
	}


	void EnvDataset::RemoveAllLayers() {
		Layers.clear();
	}

	void EnvDataset::ReadinLayers(vector<string>& envLayerFilenames, const vector<string>& datatypes, double ramEfficent) {
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
		LayerRef = new BaseIO(envLayerFilenames[0]);
		TotalX = LayerRef->getXSize();
		TotalY = LayerRef->getYSize();
		CellSize = LayerRef->getDxA();
		CellSizeY = LayerRef->getDyA(); // Assuming dx==dy
		NoDataValue = LayerRef->getNoDataValue();

		// Read tiff data into partitions and blocks

		if (ramEfficent > 0.9999)
			LayerRef->blockNull();
		else
			LayerRef->blockInit(ramEfficent / double(layerNum));
		// Get the size of current block;
		XSize = LayerRef->getBlockX();
		YSize = LayerRef->getBlockY();
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
				newLayer = new EnvLayer(i, LayerNames[i], envLayerFilenames[i].c_str(), CATEGORICAL, LayerRef);
			}
			else if (datatype == "CONTINUOUS") {
				newLayer = new EnvLayer(i, LayerNames[i], envLayerFilenames[i].c_str(), CONTINUOUS, LayerRef);
			}
			else {
				newLayer = new EnvLayer(i, LayerNames[i], envLayerFilenames[i].c_str(), CONTINUOUS, LayerRef);
			}
			if (i == 0) {
				AddLayer(newLayer);
			}
			else {
				if (!LayerRef->compareIO(newLayer->baseRef)) {
					cout << "File size do not match: " << envLayerFilenames[i] << endl;
					return;
				}
				else {
					AddLayer(newLayer);
				}
			}
		}
	}

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
	void EnvDataset::Writeout(string filename, float* EnvData, int blockRank) {
		int localx = 0;
		int localy = 0;
		int globalx, globaly;
		LayerRef->localToGlobal(blockRank, localx, localy, globalx, globaly);
		int nx = LayerRef->getBlockX();
		int ny = LayerRef->getBlockY();
		if (blockRank == (LayerRef->getBlockSize() - 1)) {
			ny = LayerRef->getYSize() - blockRank * LayerRef->getBlockY();
		}
		if (blockRank == 0) LayerRef->writeInit();
		if (EnvData != nullptr)
			LayerRef->write(globalx, globaly, ny, nx, EnvData, filename);
	}

}
