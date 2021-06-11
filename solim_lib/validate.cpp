#include "validate.h"
namespace solim {
Validate::Validate(string sampleFile, string resultFile,
                   string resultDatatype, string xField, string yField, string targetField)
{
    vector<string> filenames;
    filenames.push_back(resultFile);
    vector<string> datatypes;
    datatypes.push_back(resultDatatype);
    EnvDataset *eds = new EnvDataset(filenames,datatypes);
    mappingResult = eds->Layers[0];
    samples = ReadTable(sampleFile,eds,targetField,xField,yField);
    sampleSize = samples.size();
    for(int i = 0; i<sampleSize;i++){
        EnvUnit *e = samples[i];
        predictedValues.push_back(mappingResult->GetValue(e->Loc->Col,e->Loc->Row));
        measuredValues.push_back(e->SoilVariable);
    }
}
double Validate::getRMSE(){
    if(mappingResult->DataType==CATEGORICAL)
        return -1;
    double sum = 0;
    for(int i = 0;i < sampleSize;i++){
        sum += pow(predictedValues[i]-measuredValues[i],2);
    }
    return sqrt(sum/sampleSize);
}
double Validate::getMAE(){
    if(mappingResult->DataType==CATEGORICAL)
        return -1;
    double sum = 0;
    for(int i = 0;i < sampleSize;i++){
        sum += fabs(predictedValues[i]-measuredValues[i]);
    }
    return sum/sampleSize;
}
double Validate::getPrecision(){
    if(mappingResult->DataType==CONTINUOUS)
        return -1;
    else {
        //todo
        return -1;
    }
}
double Validate::getRecall(){
    if(mappingResult->DataType==CONTINUOUS)
        return -1;
    else {
        //todo
        return -1;
    }
}
double Validate::getOverallAccuracy(){
    if(mappingResult->DataType==CONTINUOUS)
        return -1;
    else {
        //todo
        return -1;
    }
}
}
