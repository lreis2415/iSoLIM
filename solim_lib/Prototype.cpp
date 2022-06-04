#include "Prototype.h"
#include <QTextCodec>
namespace solim {
	Prototype::Prototype() {
        envConditions.clear();
        properties.clear();
        envConsIsSorted = false;
        envConditionSize = 0;
        uncertainty = 0;
        source = UNKNOWN;
        prototypeBaseName="";
	}

    vector<Prototype> *Prototype::getPrototypesFromSample(string filename, EnvDataset* eds, string prototypeName, string xfield, string yfield,vector<string> categoricalProps) {
        vector<Prototype> *prototypes = new vector<Prototype>;
        /*ifstream file(filename); // declare file stream:
        if(!file.is_open()) return nullptr;
        string line;
        getline(file, line);*/
        QTextCodec *code = QTextCodec::codecForName("UTF-8");
        QString filename1 = QString::fromStdString(code->fromUnicode(QString(filename.c_str())).data());
        QFile file(filename1);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  return nullptr;
        QTextStream txtInput(&file);
        string line= txtInput.readLine().toStdString();
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

        int num=0;
        //while (getline(file, line)) {
        while (!txtInput.atEnd()){
            line = txtInput.readLine().toStdString();
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
                if (fabs(e->EnvValues.at(i) - eds->Layers.at(i)->NoDataValue) < VERY_SMALL||e->EnvValues.at(i)<NODATA) {
                    nullSample = true;
					break;
				}
			}
            if (!nullSample) {
				Prototype pt;
				pt.source=SAMPLE;
                for (size_t i = 0; i < eds->Layers.size(); ++i) {
					EnvLayer *layer = eds->Layers[i];     
                    try {
                        Curve *condition = new Curve(layer->LayerName, x, y, layer);
                        pt.envConditions.push_back(*condition);
                        ++(pt.envConditionSize);
                    } catch (invalid_argument msg){
                        cout << "Exception in create membership function";
                    }
				}
                if(categoricalProps.size()==0){
                    for (int i = 0; i < values.size(); ++i) {
                        if (i == pos_X || i == pos_Y || i == pos_idName) continue;
                        pt.addProperties(names[i], atof(values[i].c_str()));
                    }
                } else {
                    for (int i = 0; i < values.size(); ++i) {
                        if (i == pos_X || i == pos_Y || i == pos_idName) continue;
                        if(std::find(categoricalProps.begin(),categoricalProps.end(),names[i])!=categoricalProps.end()){
                            pt.addProperties(names[i], int(atof(values[i].c_str())),CATEGORICAL);
                        } else{
                            pt.addProperties(names[i], atof(values[i].c_str()));
                        }
                    }
                }
                pt.prototypeBaseName = prototypeName;
                if(id_found)
                    pt.prototypeID = values[pos_idName];
                else {
                    pt.prototypeID = std::to_string(num);
                    num++;
                }
				pt.uncertainty = 0;
				prototypes->push_back(pt);
			}
		}
		file.close();
		return prototypes;
	}

    Prototype::Prototype(EnvDataset* eds, int iSoilIDField, string prototypeBasename, OGRFeature* poFeature, int fid, string soilIDName){
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
        int nx = eds->XSize;
        int ny = eds->YSize;
        for (int i = 0; i < block_size; ++i) {
            for (int k = 0; k < eds->Layers.size(); ++k) {
                eds->Layers.at(k)->ReadByBlock(i);
            }
            // check if this block is within the extent of the feature
            if (i == (block_size - 1)) {
                ny = eds->TotalY - i * eds->YSize;
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
                Curve c=Curve(eds->Layers.at(i)->LayerName, values);
                if(c.getKnotNum()>0) addConditions(c);
            }
            else {
                Curve c=Curve(eds->Layers.at(i)->LayerName, freq[i]);
                if(c.getKnotNum()>0) addConditions(c);
            }
        }
        // add soil id to prototypeID and soil properties
        OGRFieldDefn *poFieldDefn = poFeature->GetFieldDefnRef(iSoilIDField);
        for (int iField = 0; iField < poFeature->GetFieldCount(); iField++)
        {
            if (iField == iSoilIDField) {
                prototypeID = poFeature->GetFieldAsString(iField);
                int id = -1;
                try {
                    id = stoi(prototypeID);
                    addProperties(soilIDName,id,CATEGORICAL);
                }  catch (invalid_argument) {
                    addProperties(soilIDName,fid,CATEGORICAL);
                }
                continue;
            }
        }
        if(iSoilIDField<0){
            prototypeID = prototypeBasename + to_string(fid);
            addProperties("ID",fid,CATEGORICAL);
        }
        //GDALClose(poDS);
    }

    Prototype::Prototype(EnvDataset* eds, int iSoilIDField, string prototypeBasename, OGRLayer* poLayer, vector<int> fids,string soilIDName){
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
            int nx = eds->XSize;
            int ny = eds->YSize;
            for (int i = 0; i < block_size; ++i) {
                for (int k = 0; k < eds->Layers.size(); ++k) {
                    eds->Layers.at(k)->ReadByBlock(i);
                }
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
                OGRFieldDefn *poFieldDefn = poFeature->GetFieldDefnRef(iSoilIDField);
                for (int iField = 0; iField < poFeature->GetFieldCount(); iField++)
                {
                    if (iField == iSoilIDField) {
                        prototypeID = poFeature->GetFieldAsString(iField);
                        int id = -1;
                        try {
                            id = stoi(prototypeID);
                            addProperties(soilIDName,id,CATEGORICAL);
                        }  catch (invalid_argument) {
                            addProperties(soilIDName,fids[iFid],CATEGORICAL);
                        }
                        continue;
                    }
                }
                if(iSoilIDField<0){
                    prototypeID = prototypeBasename + to_string(fids[iFid]);
                    addProperties("ID",fids[iFid],CATEGORICAL);
                }
            }
        }
        // if polygon is smaller than 4 cells, does not count as a prototype
        if (freq[0]->size() < 4) return;
        for (size_t i = 0; i < eds->Layers.size(); i++) {
            if (eds->Layers.at(i)->DataType == CATEGORICAL) {
                vector<int>*values = new vector<int>(freq[i]->begin(), freq[i]->end());
                Curve c=Curve(eds->Layers.at(i)->LayerName, values);
                if(c.getKnotNum()>0) addConditions(c);
            }
            else {
                Curve c=Curve(eds->Layers.at(i)->LayerName, freq[i]);
                if(c.getKnotNum()>0) addConditions(c);
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
            if (fieldname == soilIDFieldName/*"soilID"*/) {
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
                    Prototype p(eds,iIdField,prototypeBasename,poLayer,polyIDs,soilIDFieldName);
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
                Prototype p(eds,iIdField,prototypeBasename,poLayer->GetFeature(feature_num),feature_num,soilIDFieldName);
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
            Prototype p(eds,iIdField,prototypeBasename,poLayer->GetFeature(feature_num),feature_num, soilIDFieldName);

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
        int soilID_num = 0;
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
                    for (size_t iProto = 0; iProto < tmp_protos.size(); ++iProto) {
                        curves->push_back(tmp_protos[iProto].envConditions[iCon]);
                    }
                    p.addConditions(Curve(covname, curves));
                    p.envConditions.at(iCon).range=ranges[iCon];
                }
                int id = -1;
                try {
                    id = std::stoi(*it);
                    p.addProperties(soilIDFieldName, id, CATEGORICAL);
                } catch(invalid_argument){
                    id = soilID_num;
                    p.addProperties(soilIDFieldName, id, CATEGORICAL);
                }
                soiltypes_proto->push_back(p);
                soilID_num++;
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
        for(size_t i=0;i<properties.size();i++){
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
	}
	void Prototype::writeRules(string fileName) {
		char *cFileName = new char[fileName.length() + 1];
		strcpy(cFileName, fileName.c_str());
        string ext = fileName.substr(fileName.find_last_of(".") + 1);
        if (ext!="xml" && ext != "XML") {
			strcat(cFileName, ".xml");
		}

		TiXmlDocument *doc = new TiXmlDocument();
		TiXmlDeclaration *pDeclaration = new TiXmlDeclaration("1.0", "UTF-8", "");
		doc->LinkEndChild(pDeclaration);
		TiXmlElement *root_node = new TiXmlElement("CurveLib");
		doc->LinkEndChild(root_node);

        for (vector<Curve>::iterator it = envConditions.begin(); it != envConditions.end(); ++it) {
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
        string ext = fileName.substr(fileName.find_last_of(".") + 1);
        if (ext!="xml" && ext != "XML") {
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

