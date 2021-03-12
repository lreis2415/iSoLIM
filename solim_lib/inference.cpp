#include "inference.h"

namespace solim {
void Inference::inferMap(EnvDataset *eds, vector<Prototype>* prototypes, string targetVName, double threshold, string outSoilFile, string outUncerFile, QProgressBar *progressBar) {
    // check the consistency of prototype rules and envdataset
    for (auto it=prototypes->begin(); it!=prototypes->end(); ++it){
        if (!(*it).checkEnvConsIsSorted(eds)) {
            throw invalid_argument("Prototype inconsistent with layers");
            return;
        }
    }
    int Xstart, Ystart;
    int nx, ny;
    double xa, ya;
    int block_size = eds->Layers.at(0)->BlockSize;
    nx = eds->XSize;
    ny = eds->YSize;
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
            ny = eds->TotalY - i * eds->YSize;
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
    EnvDataset *eds = new EnvDataset(filenames, datatypes,layernames, ramEfficient);
    vector<Prototype> *prototypes = Prototype::getPrototypesFromSample(sampleFilename, eds);
    inferMap(eds, prototypes, targetVName, threshold, outSoilFile, outUncerFile,progressBar);
}

}
