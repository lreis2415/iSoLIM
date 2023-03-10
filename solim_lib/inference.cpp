#include "inference.h"
#include "QTime"

namespace solim {
Inference::Inference(EnvDataset *eds, vector<Prototype>* prototypes, double threshold,
                     string outSoilFile, string outUncerFile,IntegrationMethod integrate):
EDS(eds),Prototypes(prototypes),Threshold(threshold),outSoilFilename(outSoilFile),
  outUncerFilename(outUncerFile),Integrate(integrate),outSoilMap(nullptr),outUncerMap(nullptr){}

void Inference::Mapping(string targetVName,QProgressBar *progressBar){
    // check the consistency of prototype rules and envdataset
    for (auto it=Prototypes->begin(); it!=Prototypes->end(); ++it){
        if (!(*it).checkEnvConsIsSorted(EDS)) {
            throw invalid_argument("Prototype inconsistent with layers");
            return;
        }
    }
    int Xstart, Ystart;
    int nx, ny;
    int block_size = EDS->Layers.at(0)->BlockSize;
    nx = EDS->XSize;
    ny = EDS->YSize;
    float *uncertaintyValue, *predictedValue;
    uncertaintyValue = new float[nx*ny];
    predictedValue = new float[nx*ny];
    outSoilMap = new BaseIO(EDS->LayerRef);
    outSoilMap->setFileName(outSoilFilename);
    outSoilMap->setNodataValue(NODATA);
    outUncerMap = new BaseIO(EDS->LayerRef);
    outUncerMap->setFileName(outUncerFilename);
    outUncerMap->setNodataValue(-1);
    double *envValues = new double[MAXLN_LAYERS];
    double *nodata = new double[MAXLN_LAYERS];
    for (int k = 0; k < EDS->Layers.size(); k++) {
        nodata[k] = EDS->Layers.at(k)->NoDataValue;
    }
    //EnvUnit *e;
    progressBar->setMinimum(0);
    progressBar->setMaximum(block_size*100);
#ifdef EXPERIMENT
    compute_time = 0;
#endif
    for (int i = 0; i < block_size; ++i) {
        // for each block, this circle is to ensure every block is processed

        if (i == (block_size - 1)) {
            ny = EDS->TotalY - i * EDS->YSize;
        }
        progressBar->setValue(i*100);
        // read the data into all the env layers
        for (int k = 0; k < EDS->Layers.size(); ++k) {
            EDS->Layers.at(k)->ReadByBlock(i);
        }
#ifdef EXPERIMENT
        QTime start = QTime::currentTime();
#endif
        long long int pixelCount = nx*ny;
        int numcores = omp_get_num_procs();
#ifdef EXPERIMENT
        cout<<i<<" number of processors: "<<numcores<<endl;
#endif
#pragma omp parallel for schedule(dynamic) num_threads(numcores)
        for (int n = 0; n < nx*ny; ++n) {
            // for each unit in the block, calculate their predicted value and uncertainty
            bool validEnvUnitFlag = TRUE;

            double progressPara = 100.0/pixelCount;
            if (n % int(pixelCount*0.01)==0 && n > 0) {
                if(omp_get_thread_num()==0)
                    progressBar->setValue(n*progressPara+i*100);
            }
            for (int k = 0; k < EDS->Layers.size(); ++k) {
                // get the values at all layers for the unit
                float value = EDS->Layers.at(k)->EnvData[n];
                if (fabs(value - nodata[k]) < VERY_SMALL || value<NODATA) {
                    validEnvUnitFlag = FALSE;
                    break;
                }
                envValues[k] = value;
            }
            if (!validEnvUnitFlag) {
                uncertaintyValue[n] = -1;
                predictedValue[n] = NODATA;
                //delete e;
                continue;
            }
            double valueSum = 0;
            double weightSum = 0;
            double maxSimi = 0;
            // adaptive threshold
            /*double *simi_collect = new double[prototypes->size()];
            double *value_collect = new double[prototypes->size()];
            int k = 0;
            for (vector<Prototype>::iterator it = prototypes->begin(); it != prototypes->end(); ++it) {
                // calculate similarity to prototype
                double tmpOptimity;
                double minOptimity = (*it).envConditions[0].getOptimality(envValues[0]);
                for (int i = 1; i < eds->Layers.size(); ++i) {
                    tmpOptimity = (*it).envConditions[i].getOptimality(envValues[i]);
                    if (tmpOptimity < minOptimity) minOptimity = tmpOptimity;
                }
                simi_collect[k] = minOptimity;
                value_collect[k] = (*it).getProperty(targetVName);
                k++;
            }
            std::sort(simi_collect, simi_collect + prototypes->size());
            int threshold_loc = prototypes->size()-int(prototypes->size()/10+0.5)-1;
            if(threshold_loc<0) threshold_loc = 0;
            double threshold = simi_collect[threshold_loc];
            for(int i = 0; i < prototypes->size(); i++){
                if(simi_collect[i]>threshold){
                    valueSum += simi_collect[i]*value_collect[i];
                    weightSum += simi_collect[i];
                    if (simi_collect[i] > maxSimi)
                        maxSimi = simi_collect[i];
                }
            }*/
            // calculate predicted value
            for (vector<Prototype>::iterator it = Prototypes->begin(); it != Prototypes->end(); ++it) {
                // calculate similarity to prototype
                double tmpOptimity;
                double minOptimity = (*it).envConditions[0].getOptimality(envValues[0]);
                for (int i = 1; i < EDS->Layers.size(); ++i) {
                    tmpOptimity = (*it).envConditions[i].getOptimality(envValues[i]);
                    if (tmpOptimity < minOptimity) minOptimity = tmpOptimity;
                }
                double simi = minOptimity;
                //double simi = (*it).calcSimi_preChecked(e);
                if (simi > Threshold) {
                    valueSum += simi*(*it).getProperty(targetVName);
                    weightSum += simi;
                    if (simi > maxSimi)
                        maxSimi = simi;
                }
            }
            if (fabs(weightSum) < VERY_SMALL) {
                uncertaintyValue[n] = -1;
                predictedValue[n] = NODATA;
            }
            else {
                predictedValue[n] = valueSum / weightSum;
                uncertaintyValue[n] = 1 - maxSimi;
            }
        }
#ifdef EXPERIMENT
        QTime end = QTime::currentTime();
        compute_time += start.msecsTo(end)/1000.0;
#endif
        EDS->LayerRef->localToGlobal(i, 0, 0, Xstart, Ystart);
        outSoilMap->write(Xstart, Ystart, ny, nx, predictedValue);
        outUncerMap->write(Xstart, Ystart, ny, nx, uncertaintyValue);//

    }
#ifdef EXPERIMENT
    cout<<"compute time:"<<compute_time<<endl;
#endif
    outSoilMap->computeStatistics();
    delete []envValues;
    delete []nodata;
    delete predictedValue;
    delete uncertaintyValue;
}

void Inference::MappingCategorical(string targetVName,string membershipFolder,QProgressBar *progressBar){
    vector<int> category_nums;
    for (vector<Prototype>::iterator it=Prototypes->begin(); it!=Prototypes->end(); ++it){
        if (!(*it).checkEnvConsIsSorted(EDS)) {
            throw invalid_argument("Prototype inconsistent with layers");
            return;
        }
        category_nums.push_back((*it).getProperty(targetVName));
    }
    // check if each prototype represent one unique category
    std::sort(category_nums.begin(), category_nums.end());
    vector<int>::iterator last = std::unique(category_nums.begin(), category_nums.end());
    category_nums.erase(last, category_nums.end());
    category_nums.shrink_to_fit();
    if(category_nums.size()<Prototypes->size()){
        vector<Prototype>* prototyeps_merge = new vector<Prototype>;
        for(int i = 0; i<category_nums.size();i++){
            vector<Prototype> tmp_protos;
            vector<Prototype>::iterator it = Prototypes->begin();
            while(it!=Prototypes->end()){
                if(int((*it).getProperty(targetVName))==category_nums[i]){
                    tmp_protos.push_back(Prototype(*it));
                    it = Prototypes->erase(it);
                }
                else ++it;
            }
            if (tmp_protos.size() == 1) prototyeps_merge->push_back(tmp_protos[0]);
            else if (tmp_protos.size() > 1) {
                Prototype p;
                p.source = MAP;
                p.prototypeBaseName=tmp_protos[0].prototypeBaseName;
                p.prototypeID = to_string(category_nums[i]);
                for (int iCon = 0; iCon < tmp_protos[0].envConditionSize; ++iCon) {
                    string covname = tmp_protos[0].envConditions[iCon].covariateName;
                    vector<Curve>* curves = new vector<Curve>;
                    for (size_t iProto = 0; iProto < tmp_protos.size(); ++iProto) {
                        curves->push_back(tmp_protos[iProto].envConditions[iCon]);
                    }
                    p.addConditions(Curve(covname, curves));
                }
                p.addProperties(targetVName, category_nums[i], CATEGORICAL);
                prototyeps_merge->push_back(p);
            }
        }
        Prototypes->clear();
        Prototypes->insert(Prototypes->end(),prototyeps_merge->begin(),prototyeps_merge->end());
        Prototypes->shrink_to_fit();
    }
    // re-sort category_nums based on the sequence of prototypes
    category_nums.clear();
    for (vector<Prototype>::iterator it=Prototypes->begin(); it!=Prototypes->end(); ++it){
        if (!(*it).checkEnvConsIsSorted(EDS)) {
            throw invalid_argument("Prototype inconsistent with layers");
            return;
        }
        category_nums.push_back((*it).getProperty(targetVName));
    }
    category_nums.shrink_to_fit();
    // calculate membership
    int Xstart, Ystart;
    int nx, ny;
    double xa, ya;
    int block_size = EDS->Layers.at(0)->BlockSize;
    nx = EDS->XSize;
    ny = EDS->YSize;
    float *uncertaintyValue, *predictedValue;
    uncertaintyValue = new float[nx*ny];
    predictedValue = new float[nx*ny];
    outSoilMap = new BaseIO(EDS->LayerRef);
    outSoilMap->setFileName(outSoilFilename);
    outSoilMap->setNodataValue(NODATA);
    outUncerMap = new BaseIO(EDS->LayerRef);
    outUncerMap->setFileName(outUncerFilename);
    outUncerMap->setNodataValue(-1);
    double *envValues = new double[MAXLN_LAYERS];
    double *nodata = new double[MAXLN_LAYERS];
    for (int k = 0; k < EDS->Layers.size(); k++) {
        nodata[k] = EDS->Layers.at(k)->NoDataValue;
    }
    vector<BaseIO> *membershipMaps = new vector<BaseIO>;
    vector<float*> membershipData;
    if(membershipFolder!=""){
        string refname = EDS->LayerRef->getFilename();
        string ext = refname.substr(refname.find_last_of("."));
        string slash = "/";
        if(membershipFolder.find("\\")!=string::npos) slash = "\\";
        long pixelCount = EDS->XSize * EDS->YSize;
        for(int i = 0; i<category_nums.size();i++){
            BaseIO tmpMap = BaseIO(EDS->LayerRef);
            tmpMap.setFileName(membershipFolder + slash + to_string(category_nums[i]) + ext);
            tmpMap.setNodataValue(-1);
            membershipMaps->push_back(tmpMap);
            membershipData.push_back(new float[pixelCount]);
        }
    }
    //EnvUnit *e;
    progressBar->setMinimum(0);
    progressBar->setMaximum(block_size*100);
    for (int i = 0; i < block_size; ++i) {
        // for each block, this circle is to ensure every block is processed

        if (i == (block_size - 1)) {
            ny = EDS->TotalY - i * EDS->YSize;
        }
        progressBar->setValue(i*100);
        // read the data into all the env layers
        for (int k = 0; k < EDS->Layers.size(); ++k) {
            EDS->Layers.at(k)->ReadByBlock(i);
        }
        long long int pixelCount = nx*ny;
        int numcores = omp_get_num_procs();
#pragma omp parallel for schedule(dynamic) num_threads(numcores)
        for (int n = 0; n < nx*ny; ++n) {
            // for each unit in the block, calculate their predicted value and uncertainty
            bool validEnvUnitFlag = TRUE;

            double progressPara = 100.0/pixelCount;
            if (n % int(pixelCount*0.01)==0 && n > 0) {
                if(omp_get_thread_num()==0)
                    progressBar->setValue(n*progressPara+i*100);
            }
            for (int k = 0; k < EDS->Layers.size(); ++k) {
                // get the values at all layers for the unit
                float value = EDS->Layers.at(k)->EnvData[n];
                if (fabs(value - nodata[k]) < VERY_SMALL || value<NODATA) {
                    validEnvUnitFlag = FALSE;
                    break;
                }
                envValues[k] = value;
            }
            if (!validEnvUnitFlag) {
                uncertaintyValue[n] = -1;
                predictedValue[n] = NODATA;
                if(membershipFolder!=""){
                    for(int proto_num = 0; proto_num<category_nums.size();proto_num++){
                          membershipData[proto_num][n]=-1;
                    }
                }
                continue;
            }
            double maxSimi = 0;
            int hardenedClass = NODATA;
            int proto_num = 0;
            // calculate predicted value
            for (vector<Prototype>::iterator it = Prototypes->begin(); it != Prototypes->end(); ++it) {
                // calculate similarity to prototype
                double tmpOptimity;
                double minOptimity = (*it).envConditions[0].getOptimality(envValues[0]);
                for (int i = 1; i < EDS->Layers.size(); ++i) {
                    tmpOptimity = (*it).envConditions[i].getOptimality(envValues[i]);
                    if (tmpOptimity < minOptimity) minOptimity = tmpOptimity;
                }
                double simi = minOptimity;
                //double simi = (*it).calcSimi_preChecked(e);
                if (simi > Threshold) {
                    if (simi > maxSimi) {
                        maxSimi = simi;
                        hardenedClass = (*it).getProperty(targetVName);
                    }
                }
                if(membershipFolder!=""){
                    membershipData[proto_num][n]=simi;
                }
                proto_num++;
            }
            predictedValue[n] = hardenedClass;
            uncertaintyValue[n] = 1 - maxSimi;
        }
        EDS->LayerRef->localToGlobal(i, 0, 0, Xstart, Ystart);
        outSoilMap->setNodataValue(NODATA);
        outSoilMap->write(Xstart, Ystart, ny, nx, predictedValue);
        outUncerMap->setNodataValue(-1);
        outUncerMap->write(Xstart, Ystart, ny, nx, uncertaintyValue);//
        if(membershipFolder!=""){
            for(int proto_num = 0; proto_num<category_nums.size();proto_num++){
                membershipMaps->at(proto_num).write(Xstart, Ystart, ny, nx,membershipData[proto_num]);
            }
        }

    }
    outSoilMap->computeStatistics();
    delete []envValues;
    delete []nodata;
    delete predictedValue;
    delete uncertaintyValue;
}

void Inference::inferMap(EnvDataset *eds, vector<Prototype>* prototypes, string targetVName, double threshold, string outSoilFile, string outUncerFile, QProgressBar *progressBar,IntegrationMethod integrate) {
    // check the consistency of prototype rules and envdataset
    for (auto it=prototypes->begin(); it!=prototypes->end(); ++it){
        if (!(*it).checkEnvConsIsSorted(eds)) {
            throw invalid_argument("Prototype inconsistent with layers");
            return;
        }
    }
    int Xstart, Ystart;
    int nx, ny;
    int block_size = eds->Layers.at(0)->BlockSize;
    nx = eds->XSize;
    ny = eds->YSize;
    float *uncertaintyValue, *predictedValue;
    uncertaintyValue = new float[nx*ny];
    predictedValue = new float[nx*ny];
    BaseIO *outSoilMap = new BaseIO(eds->LayerRef);
    outSoilMap->setFileName(outSoilFile);
    outSoilMap->setNodataValue(NODATA);
    BaseIO *outUncerMap = new BaseIO(eds->LayerRef);
    outUncerMap->setFileName(outUncerFile);
    outUncerMap->setNodataValue(-1);
    double *envValues = new double[MAXLN_LAYERS];
    double *nodata = new double[MAXLN_LAYERS];
    for (int k = 0; k < eds->Layers.size(); k++) {
        nodata[k] = eds->Layers.at(k)->NoDataValue;
    }
    //EnvUnit *e;
    progressBar->setMinimum(0);
    progressBar->setMaximum(block_size*100);
    double compute_time = 0;
    for (int i = 0; i < block_size; ++i) {
        // for each block, this circle is to ensure every block is processed

        if (i == (block_size - 1)) {
            ny = eds->TotalY - i * eds->YSize;
        }
        progressBar->setValue(i*100);
        // read the data into all the env layers
        for (int k = 0; k < eds->Layers.size(); ++k) {
            eds->Layers.at(k)->ReadByBlock(i);
        }
        //QTime start = QTime::currentTime();
        long long int pixelCount = nx*ny;
        int numcores = omp_get_num_procs();
        #ifdef EXPERIMENT
        cout<<i<<" number of processors: "<<numcores<<endl;
        #endif
#pragma omp parallel for schedule(dynamic) num_threads(numcores)
        for (int n = 0; n < nx*ny; ++n) {
            // for each unit in the block, calculate their predicted value and uncertainty
            bool validEnvUnitFlag = TRUE;

            double progressPara = 100.0/pixelCount;
            if (n % int(pixelCount*0.01)==0 && n > 0) {
                if(omp_get_thread_num()==0)
                    progressBar->setValue(n*progressPara+i*100);
            }
            for (int k = 0; k < eds->Layers.size(); ++k) {
                // get the values at all layers for the unit
                float value = eds->Layers.at(k)->EnvData[n];
                if (fabs(value - nodata[k]) < VERY_SMALL || value<NODATA) {
                    validEnvUnitFlag = FALSE;
                    break;
                }
                envValues[k] = value;
            }
            if (!validEnvUnitFlag) {
                uncertaintyValue[n] = -1;
                predictedValue[n] = NODATA;
                //delete e;
                continue;
            }
            double valueSum = 0;
            double weightSum = 0;
            double maxSimi = 0;
            // adaptive threshold
            /*double *simi_collect = new double[prototypes->size()];
            double *value_collect = new double[prototypes->size()];
            int k = 0;
            for (vector<Prototype>::iterator it = prototypes->begin(); it != prototypes->end(); ++it) {
                // calculate similarity to prototype
                double tmpOptimity;
                double minOptimity = (*it).envConditions[0].getOptimality(envValues[0]);
                for (int i = 1; i < eds->Layers.size(); ++i) {
                    tmpOptimity = (*it).envConditions[i].getOptimality(envValues[i]);
                    if (tmpOptimity < minOptimity) minOptimity = tmpOptimity;
                }
                simi_collect[k] = minOptimity;
                value_collect[k] = (*it).getProperty(targetVName);
                k++;
            }
            std::sort(simi_collect, simi_collect + prototypes->size());
            int threshold_loc = prototypes->size()-int(prototypes->size()/10+0.5)-1;
            if(threshold_loc<0) threshold_loc = 0;
            double threshold = simi_collect[threshold_loc];
            for(int i = 0; i < prototypes->size(); i++){
                if(simi_collect[i]>threshold){
                    valueSum += simi_collect[i]*value_collect[i];
                    weightSum += simi_collect[i];
                    if (simi_collect[i] > maxSimi)
                        maxSimi = simi_collect[i];
                }
            }*/
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
                uncertaintyValue[n] = -1;
                predictedValue[n] = NODATA;
            }
            else {
                predictedValue[n] = valueSum / weightSum;
                uncertaintyValue[n] = 1 - maxSimi;
            }
        }
        //QTime end = QTime::currentTime();
        //compute_time += start.msecsTo(end)/1000.0;
        eds->LayerRef->localToGlobal(i, 0, 0, Xstart, Ystart);
        outSoilMap->write(Xstart, Ystart, ny, nx, predictedValue);
        outUncerMap->write(Xstart, Ystart, ny, nx, uncertaintyValue);//

    }
    outSoilMap->computeStatistics();
    delete []envValues;
    delete []nodata;
    delete predictedValue;
    delete uncertaintyValue;
    //eds->CellSizeY = compute_time;
}

void Inference::inferCategoricalMap(EnvDataset *eds, vector<Prototype>* prototypes, string targetVName, double threshold,
                                    string outSoilFile, string outUncerFile, string membershipFolder, QProgressBar *progressBar) {
    // check the consistency of prototype rules and envdataset
    vector<int> category_nums;
    for (vector<Prototype>::iterator it=prototypes->begin(); it!=prototypes->end(); ++it){
        if (!(*it).checkEnvConsIsSorted(eds)) {
            throw invalid_argument("Prototype inconsistent with layers");
            return;
        }
        category_nums.push_back((*it).getProperty(targetVName));
    }
    // check if each prototype represent one unique category
    std::sort(category_nums.begin(), category_nums.end());
    vector<int>::iterator last = std::unique(category_nums.begin(), category_nums.end());
    category_nums.erase(last, category_nums.end());
    category_nums.shrink_to_fit();
    if(category_nums.size()<prototypes->size()){
        vector<Prototype>* prototyeps_merge = new vector<Prototype>;
        for(int i = 0; i<category_nums.size();i++){
            vector<Prototype> tmp_protos;
            vector<Prototype>::iterator it = prototypes->begin();
            while(it!=prototypes->end()){
                if(int((*it).getProperty(targetVName))==category_nums[i]){
                    tmp_protos.push_back(Prototype(*it));
                    it = prototypes->erase(it);
                }
                else ++it;
            }
            if (tmp_protos.size() == 1) prototyeps_merge->push_back(tmp_protos[0]);
            else if (tmp_protos.size() > 1) {
                Prototype p;
                p.source = MAP;
                p.prototypeBaseName=tmp_protos[0].prototypeBaseName;
                p.prototypeID = to_string(category_nums[i]);
                for (int iCon = 0; iCon < tmp_protos[0].envConditionSize; ++iCon) {
                    string covname = tmp_protos[0].envConditions[iCon].covariateName;
                    vector<Curve>* curves = new vector<Curve>;
                    for (size_t iProto = 0; iProto < tmp_protos.size(); ++iProto) {
                        curves->push_back(tmp_protos[iProto].envConditions[iCon]);
                    }
                    p.addConditions(Curve(covname, curves));
                }
                p.addProperties(targetVName, category_nums[i], CATEGORICAL);
                prototyeps_merge->push_back(p);
            }
        }
        prototypes->clear();
        prototypes->insert(prototypes->end(),prototyeps_merge->begin(),prototyeps_merge->end());
        prototypes->shrink_to_fit();
    }
    // re-sort category_nums based on the sequence of prototypes
    category_nums.clear();
    for (vector<Prototype>::iterator it=prototypes->begin(); it!=prototypes->end(); ++it){
        if (!(*it).checkEnvConsIsSorted(eds)) {
            throw invalid_argument("Prototype inconsistent with layers");
            return;
        }
        category_nums.push_back((*it).getProperty(targetVName));
    }
    category_nums.shrink_to_fit();
    // calculate membership
    int Xstart, Ystart;
    int nx, ny;
    double xa, ya;
    int block_size = eds->Layers.at(0)->BlockSize;
    nx = eds->XSize;
    ny = eds->YSize;
    float *uncertaintyValue, *predictedValue;
    uncertaintyValue = new float[nx*ny];
    predictedValue = new float[nx*ny];
    BaseIO *outSoilMap = new BaseIO(eds->LayerRef);
    outSoilMap->setFileName(outSoilFile);
    outSoilMap->setNodataValue(NODATA);
    BaseIO *outUncerMap = new BaseIO(eds->LayerRef);
    outUncerMap->setFileName(outUncerFile);
    outUncerMap->setNodataValue(-1);
    double *envValues = new double[MAXLN_LAYERS];
    double *nodata = new double[MAXLN_LAYERS];
    for (int k = 0; k < eds->Layers.size(); k++) {
        nodata[k] = eds->Layers.at(k)->NoDataValue;
    }
    vector<BaseIO> *membershipMaps = new vector<BaseIO>;
    vector<float*> membershipData;
    if(membershipFolder!=""){
        string refname = eds->LayerRef->getFilename();
        string ext = refname.substr(refname.find_last_of("."));
        string slash = "/";
        if(membershipFolder.find("\\")!=string::npos) slash = "\\";
        long pixelCount = eds->XSize * eds->YSize;
        for(int i = 0; i<category_nums.size();i++){
            BaseIO tmpMap = BaseIO(eds->LayerRef);
            tmpMap.setFileName(membershipFolder + slash + to_string(category_nums[i]) + ext);
            tmpMap.setNodataValue(-1);
            membershipMaps->push_back(tmpMap);
            membershipData.push_back(new float[pixelCount]);
        }
    }
    //EnvUnit *e;
    progressBar->setMinimum(0);
    progressBar->setMaximum(block_size*100);
    for (int i = 0; i < block_size; ++i) {
        // for each block, this circle is to ensure every block is processed

        if (i == (block_size - 1)) {
            ny = eds->TotalY - i * eds->YSize;
        }
        progressBar->setValue(i*100);
        // read the data into all the env layers
        for (int k = 0; k < eds->Layers.size(); ++k) {
            eds->Layers.at(k)->ReadByBlock(i);
        }
        long long int pixelCount = nx*ny;
        int numcores = omp_get_num_procs();
#pragma omp parallel for schedule(dynamic) num_threads(numcores)
        for (int n = 0; n < nx*ny; ++n) {
            // for each unit in the block, calculate their predicted value and uncertainty
            bool validEnvUnitFlag = TRUE;

            double progressPara = 100.0/pixelCount;
            if (n % int(pixelCount*0.01)==0 && n > 0) {
                if(omp_get_thread_num()==0)
                    progressBar->setValue(n*progressPara+i*100);
            }
            for (int k = 0; k < eds->Layers.size(); ++k) {
                // get the values at all layers for the unit
                float value = eds->Layers.at(k)->EnvData[n];
                if (fabs(value - nodata[k]) < VERY_SMALL || value<NODATA) {
                    validEnvUnitFlag = FALSE;
                    break;
                }
                envValues[k] = value;
            }
            if (!validEnvUnitFlag) {
                uncertaintyValue[n] = -1;
                predictedValue[n] = NODATA;
                if(membershipFolder!=""){
                    for(int proto_num = 0; proto_num<category_nums.size();proto_num++){
                          membershipData[proto_num][n]=-1;
                    }
                }
                continue;
            }
            double maxSimi = 0;
            int hardenedClass = NODATA;
            int proto_num = 0;
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
                    if (simi > maxSimi) {
                        maxSimi = simi;
                        hardenedClass = (*it).getProperty(targetVName);
                    }
                }
                if(membershipFolder!=""){
                    membershipData[proto_num][n]=simi;
                }
                proto_num++;
            }
            predictedValue[n] = hardenedClass;
            uncertaintyValue[n] = 1 - maxSimi;
        }
        eds->LayerRef->localToGlobal(i, 0, 0, Xstart, Ystart);
        outSoilMap->setNodataValue(NODATA);
        outSoilMap->write(Xstart, Ystart, ny, nx, predictedValue);
        outUncerMap->setNodataValue(-1);
        outUncerMap->write(Xstart, Ystart, ny, nx, uncertaintyValue);//
        if(membershipFolder!=""){
            for(int proto_num = 0; proto_num<category_nums.size();proto_num++){
                membershipMaps->at(proto_num).write(Xstart, Ystart, ny, nx,membershipData[proto_num]);
            }
        }

    }
    delete []envValues;
    delete []nodata;
    delete predictedValue;
    delete uncertaintyValue;
}

void Inference::propertyInference(vector<string> filenames, vector<string> datatypes, vector<string> layernames,
    double threshold, string sampleFilename, string targetVName,
    string outSoilFile, string outUncerFile, double ramEfficient,QProgressBar *progressBar) {
    EnvDataset *eds = new EnvDataset(filenames, datatypes,layernames, ramEfficient);
    vector<Prototype> *prototypes = Prototype::getPrototypesFromSample(sampleFilename, eds);
    inferMap(eds, prototypes, targetVName, threshold, outSoilFile, outUncerFile,progressBar);
}

void Inference::typeInference(vector<string> filenames, vector<string> datatypes, vector<string> layernames,
    double threshold, string sampleFilename, string targetVName,
    string outSoilFile, string outUncerFile, double ramEfficient, string membershipFolder, QProgressBar *progressBar) {
    EnvDataset *eds = new EnvDataset(filenames, datatypes,layernames, ramEfficient);
    vector<Prototype> *prototypes = Prototype::getPrototypesFromSample(sampleFilename, eds);
    inferCategoricalMap(eds, prototypes, targetVName, threshold, outSoilFile, outUncerFile, membershipFolder, progressBar);
}

}
