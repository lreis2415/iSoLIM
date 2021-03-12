#include "Prototype.h"

namespace solim {
	Prototype::Prototype() {
		envConditions.clear();
		properties.clear();
		envConsIsSorted = false;
		envConditionSize = 0;
		uncertainty = 0;
		source = UNKNOWN;
	}

	vector<Prototype> *Prototype::getPrototypesFromSample(string filename, EnvDataset* eds) {
		vector<Prototype> *prototypes = new vector<Prototype>;
		ifstream file(filename); // declare file stream:
		string line;
		getline(file, line);
		vector<string> names;
		int pos_X = 0;
		int pos_Y = 1;
		int pos_targetVName = -1;
		int pos_idName = 0;
		bool id_found = false;
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

		for (int i = 0; i < names.size(); ++i) {
			if (names[i] == "ID" || names[i] == "id") {
				pos_idName = i;
				id_found = true;
				break;
			}
		}
		if (!id_found)	pos_idName = -1;

		while (getline(file, line)) {
			vector<string> values;
			ParseStr(line, ',', values);
			const char* xstr = values[pos_X].c_str();
			const char* ystr = values[pos_Y].c_str();
			double x = atof(xstr);
			double y = atof(ystr);
			bool nullSample = false;

			EnvUnit* e = eds->GetEnvUnit(x, y);
			for (int i = 0; i < e->EnvValues.size(); ++i) {
				if (fabs(e->EnvValues.at(i) - eds->Layers.at(i)->NoDataValue) < VERY_SMALL) {
					nullSample = true;
					break;
				}
			}
			if (e != NULL && (!nullSample)) {
				Prototype pt;
				pt.source=SAMPLE;
				for (int i = 0; i < eds->Layers.size(); ++i) {
					EnvLayer *layer = eds->Layers[i];
					Curve *condition = new Curve(layer->LayerName, x, y, layer);
					pt.envConditions.push_back(*condition);
					++(pt.envConditionSize);
				}
				for (int i = 0; i < values.size(); ++i) {
					if (i == pos_X || i == pos_Y || i == pos_idName) continue;
					pt.addProperties(names[i], atof(values[i].c_str()));
				}
				pt.prototypeID = values[pos_idName].c_str();
				pt.uncertainty = 0;
				prototypes->push_back(pt);
			}
		}
		file.close();
		return prototypes;
	}

	vector<Prototype> *Prototype::getPrototypesFromMining(string filename, EnvDataset *eds) {
		vector<Prototype> *prototypes = new vector<Prototype>;
		GDALAllRegister();
		GDALDataset *poDS;
		poDS = (GDALDataset*)GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
		if (poDS == NULL)
		{
			cout<<"Open failed."<<endl;
			exit(1);
		}
		int lyrcnt = poDS->GetLayerCount();
		OGRLayer  *poLayer;
		poLayer = poDS->GetLayer(0);
		poLayer->ResetReading();
		// check if shapefile type is polygon
		OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
		if (poFDefn->GetGeomType() != wkbPolygon) {
			cout << "Feature type is not polygon type. Cannot be used for data mining." << endl;
			return nullptr;
		}
		// check the extent of the layer
		OGREnvelope *extent = new OGREnvelope;
		poLayer->GetExtent(extent);
		double xmax = eds->LayerRef->getXMax();
		double ymin = eds->LayerRef->getYMin();
		if (extent->MinX > eds->LayerRef->getXMax() || extent->MaxX < eds->LayerRef->getXMin() ||
			extent->MinY > eds->LayerRef->getYMax() || extent->MaxY < eds->LayerRef->getYMin()) {
			cout << "Feature extent does not match covariate extent. Cannot be used for data mining." << endl;
			return nullptr;
		}
		int feature_num = 0;
		int feature_count = poLayer->GetFeatureCount();
		for (OGRFeature* poFeature = poLayer->GetNextFeature(); poFeature != NULL; poFeature = poLayer->GetNextFeature()) {
			poFeature->GetGeometryRef()->getEnvelope(extent);
			int globalXMin, globalXMax, globalYMin, globalYMax;
			eds->LayerRef->geoToGlobalXY(extent->MinX, extent->MinY, globalXMin, globalYMax);
			eds->LayerRef->geoToGlobalXY(extent->MaxX, extent->MaxY, globalXMax, globalYMin);
			// iterate over features
			Prototype p;
			p.source = MAP;
			OGRGeometry *poGeometry;
			poGeometry = poFeature->GetGeometryRef();
			if (poGeometry == NULL)	continue;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,3,0)
			OGRPolygon *poPolygon = poGeometry->toPolygon();
#else
			OGRPolygon *poPolygon = (OGRPolygon *)poGeometry;
#endif
			vector<vector<float>*> freq;
			for (int i = 0; i < eds->Layers.size(); i++) {
				freq.push_back(new vector<float>);
			}
			std::size_t first = filename.find_last_of('/');
			if (first == std::string::npos) {
				first = filename.find_last_of('\\');
			}
			std::size_t end = filename.find_last_of('.');
			string basename = filename.substr(first + 1, end - first - 1).c_str();
			// iterate over pixel
			int block_size = eds->Layers.at(0)->BlockSize;
			int nx = eds->XSize;
			int ny = eds->YSize;
			for (int i = 0; i < block_size; ++i) {
				// check if this block is within the extent of the feature
				if (i == (block_size - 1)) {
					ny = eds->TotalY - i * eds->YSize;
				}
				int localymin, localxmin, localymax, localxmax;
				eds->Layers.at(0)->baseRef->globalToLocal(i, globalXMin, globalYMin, localxmin, localymin);
				eds->Layers.at(0)->baseRef->globalToLocal(i, globalXMax, globalYMax, localxmax, localymax);
				if (localymin > ny || localymax < 0 || localxmin > nx || localxmax < 0) continue;
				// read the data into all the env layers
				for (int k = 0; k < eds->Layers.size(); ++k) {
					eds->Layers.at(k)->ReadByBlock(i);
				}
				for (int ncol = localxmin > 0 ? localxmin : 0; ncol < (localxmax < nx ? localxmax : nx); ++ncol) {
					for (int nrow = localymin > 0 ? localymin : 0; nrow < (localymax < ny ? localymax : ny); ++nrow) {
						int iloc = nrow*nx + ncol;
						double geoX, geoY;
						eds->LayerRef->globalXYToGeo(ncol, nrow, geoX, geoY);
						OGRBoolean within = OGRPoint(geoX, geoY).Within(poPolygon);
						if (within != 0) {
							for (int k = 0; k < eds->Layers.size(); ++k) {
								freq[k]->push_back(eds->Layers.at(k)->EnvData[iloc]);
							}
						}
					}
				}
			}
			if (freq[0]->size() < 4) {
				++feature_num;
				continue;
			}
			for (int i = 0; i < eds->Layers.size(); i++) {
				if (eds->Layers.at(i)->DataType == CATEGORICAL) {
					vector<int>*values = new vector<int>(freq[i]->begin(), freq[i]->end());
					p.addConditions(Curve(eds->Layers.at(i)->LayerName, values));
				}
				else {
					freq[i];
					eds->Layers.at(i)->LayerName;
					Curve(eds->Layers.at(i)->LayerName, freq[i]);
					p.addConditions(Curve(eds->Layers.at(i)->LayerName, freq[i]));
				}
			}
			bool foundID = false;
			for (int iField = 0; iField < poFDefn->GetFieldCount(); iField++)
			{
				// iterate over fields
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
				string fieldname = poFieldDefn->GetNameRef();
				if (fieldname == "ID") {
					p.prototypeID = poFeature->GetFieldAsString(iField);
					foundID = true;
					continue;
				}
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
				p.properties.push_back(sp);
			}
			if (!foundID) p.prototypeID = basename + to_string(feature_num);
			++feature_num;
			prototypes->push_back(p);
		}
		return prototypes;
	}

	vector<Prototype> *Prototype::getPrototypesFromMining(string filename, EnvDataset *eds,string soilIDFieldName) {
		vector<Prototype> *prototypes = new vector<Prototype>;
		GDALAllRegister();
		GDALDataset *poDS;
		vector<string> soilIDs;
		poDS = (GDALDataset*)GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
		if (poDS == NULL)
		{
			cout << "Open failed." << endl;
			exit(1);
		}
		int lyrcnt = poDS->GetLayerCount();
		OGRLayer  *poLayer;
		poLayer = poDS->GetLayer(0);
		poLayer->ResetReading();
		// check if shapefile type is polygon
		OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
		if (poFDefn->GetGeomType() != wkbPolygon) {
			cout << "Feature type is not polygon type. Cannot be used for data mining." << endl;
			return nullptr;
		}
		// check the extent of the layer
		OGREnvelope *extent = new OGREnvelope;
		poLayer->GetExtent(extent);
		double xmax = eds->LayerRef->getXMax();
		double ymin = eds->LayerRef->getYMin();
		if (extent->MinX > eds->LayerRef->getXMax() || extent->MaxX < eds->LayerRef->getXMin() ||
			extent->MinY > eds->LayerRef->getYMax() || extent->MaxY < eds->LayerRef->getYMin()) {
			cout << "Feature extent does not match covariate extent. Cannot be used for data mining." << endl;
			return nullptr;
		}
		int feature_num = 0;
		int feature_count = poLayer->GetFeatureCount();
		for (OGRFeature* poFeature = poLayer->GetNextFeature(); poFeature != NULL; poFeature = poLayer->GetNextFeature()) {
			poFeature->GetGeometryRef()->getEnvelope(extent);
			int globalXMin, globalXMax, globalYMin, globalYMax;
			eds->LayerRef->geoToGlobalXY(extent->MinX, extent->MinY, globalXMin, globalYMax);
			eds->LayerRef->geoToGlobalXY(extent->MaxX, extent->MaxY, globalXMax, globalYMin);
			// iterate over features
			Prototype p;
			p.source = MAP;
			OGRGeometry *poGeometry;
			poGeometry = poFeature->GetGeometryRef();
			if (poGeometry == NULL) continue;
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,3,0)
			OGRPolygon *poPolygon = poGeometry->toPolygon();
#else
			OGRPolygon *poPolygon = (OGRPolygon *)poGeometry;
#endif
			vector<vector<float>*> freq;
			for (int i = 0; i < eds->Layers.size(); i++) {
				freq.push_back(new vector<float>);
			}
			std::size_t first = filename.find_last_of('/');
			if (first == std::string::npos) {
				first = filename.find_last_of('\\');
			}
			std::size_t end = filename.find_last_of('.');
			string basename = filename.substr(first + 1, end - first - 1).c_str();
			// iterate over pixel
			int block_size = eds->Layers.at(0)->BlockSize;
			int nx = eds->XSize;
			int ny = eds->YSize;
			for (int i = 0; i < block_size; ++i) {
				// check if this block is within the extent of the feature
				if (i == (block_size - 1)) {
					ny = eds->TotalY - i * eds->YSize;
				}
				int localymin, localxmin, localymax, localxmax;
				eds->Layers.at(0)->baseRef->globalToLocal(i, globalXMin, globalYMin, localxmin, localymin);
				eds->Layers.at(0)->baseRef->globalToLocal(i, globalXMax, globalYMax, localxmax, localymax);
				if (localymin > ny || localymax < 0 || localxmin > nx || localxmax < 0) continue;
				// read the data into all the env layers
				for (int k = 0; k < eds->Layers.size(); ++k) {
					eds->Layers.at(k)->ReadByBlock(i);
				}
				for (int ncol = localxmin > 0 ? localxmin : 0; ncol < (localxmax < nx ? localxmax : nx); ++ncol) {
					for (int nrow = localymin > 0 ? localymin : 0; nrow < (localymax < ny ? localymax : ny); ++nrow) {
						int iloc = nrow*nx + ncol;
						double geoX, geoY;
						eds->LayerRef->globalXYToGeo(ncol, nrow, geoX, geoY);
						OGRBoolean within = OGRPoint(geoX, geoY).Within(poPolygon);
						if (within != 0) {
							for (int k = 0; k < eds->Layers.size(); ++k) {
								freq[k]->push_back(eds->Layers.at(k)->EnvData[iloc]);
							}
						}
					}
				}
				/*int startLoc = (localymin > 0 ? localymin : 0)*nx + localxmin > 0 ? localxmin : 0;
				int endLoc = (localymax < ny ? localymax : ny)*nx + localxmax < nx ? localxmax : nx;
				for (int j = startLoc; j < endLoc; j++) {
					int row = j / nx, col = j % nx;
					double geoX, geoY;
					eds->LayerRef->globalXYToGeo(col, row, geoX, geoY);
					auto within = OGRPoint(geoX, geoY).Within(poPolygon);
					if (within != 0) {
						for (int k = 0; k < eds->Layers.size(); ++k) {
							freq[k]->push_back(eds->Layers.at(k)->EnvData[j]);
						}
					}
				}*/
			}
			if (freq[0]->size() < 4) {
				++feature_num;
				continue;
			}
			for (int i = 0; i < eds->Layers.size(); i++) {
				if (eds->Layers.at(i)->DataType == CATEGORICAL) {
					vector<int>*values = new vector<int>(freq[i]->begin(), freq[i]->end());
					p.addConditions(Curve(eds->Layers.at(i)->LayerName, values));
				}
				else {
					freq[i];
					eds->Layers.at(i)->LayerName;
					Curve(eds->Layers.at(i)->LayerName, freq[i]);
					p.addConditions(Curve(eds->Layers.at(i)->LayerName, freq[i]));
				}
			}
			bool foundID = false;
			for (int iField = 0; iField < poFDefn->GetFieldCount(); iField++)
			{
				// iterate over fields
				OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn(iField);
				string fieldname = poFieldDefn->GetNameRef();
				if (fieldname == soilIDFieldName) {
					soilIDs.push_back(poFeature->GetFieldAsString(iField));
					p.prototypeID= poFeature->GetFieldAsString(iField);
					foundID = true;
					continue;
				}
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
				p.properties.push_back(sp);
			}
			if (!foundID) p.prototypeID = basename + to_string(feature_num);
			++feature_num;
			prototypes->push_back(p);
		}
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
				p.prototypeID = *it;
				for (int iCon = 0; iCon < tmp_protos[0].envConditionSize; ++iCon) {
					string covname = tmp_protos[0].envConditions[iCon].covariateName;
					vector<Curve>* curves = new vector<Curve>;
					for (int iProto = 0; iProto < tmp_protos.size(); ++iProto) {
						curves->push_back(tmp_protos[iProto].envConditions[iCon]);
					}
					p.addConditions(Curve(covname, curves));
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

	void Prototype::addProperties(string propertyName, double propertyValue/*, DataTypeEnum type*/) {
		SoilProperty sp;
		sp.propertyName = propertyName;
		sp.propertyValue = propertyValue;
		//sp.soilPropertyType = type;
		properties.push_back(sp);
	}


	double Prototype::getProperty(string propName) {
		for (auto it = properties.begin(); it != properties.end(); ++it) {
			if ((*it).propertyName == propName) {
				return (*it).propertyValue;
			}
		}
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

	void Prototype::sortEnvCons(vector<string> layernames) {
		if (layernames.size() != envConditionSize)
			throw invalid_argument("Error: inconsistant rule number and layer number");
		envConsIsSorted = true;
		for (int i = 0; i < envConditionSize; ++i) {
			if (envConditions[i].covariateName != layernames[i]) {
				envConsIsSorted = false;
				break;
			}
		}
		int sumi = 0;
		int sumj = 0;
		if (!envConsIsSorted) {
			vector<Curve> tempCurves = envConditions;
			for (int i = 0; i < envConditionSize; ++i) {
				for (int j = 0; j < envConditionSize; ++j) {
					if (tempCurves[i].covariateName == layernames[j]) {
						envConditions[j] = tempCurves[i];
						sumi += i;
						sumj += j;
					}
				}
			}
		}
		if(sumi==sumj)
			envConsIsSorted = true;
		else
			throw invalid_argument("Error: inconsistant rule names and layer names");
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
		if(!envConsIsSorted) sortEnvCons(eds->LayerNames);
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
}