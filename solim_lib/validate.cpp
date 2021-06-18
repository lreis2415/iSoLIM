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
    samples = ReadTable(sampleFile,eds,targetField,"None",xField,yField);
    sampleSize = samples.size();
    validSampleCount = 0;
    for(int i = 0; i<sampleSize;i++){
        EnvUnit *e = samples[i];
        double predictedValue = mappingResult->GetValue(e->Loc->Col,e->Loc->Row);
        double measuredValue = e->SoilVariable;
        if(fabs(predictedValue-mappingResult->NoDataValue) < VERY_SMALL
                || (predictedValue - NODATA) < VERY_SMALL
                || (measuredValue - NODATA) < VERY_SMALL)
            continue;
        predictedValues.push_back(predictedValue);
        measuredValues.push_back(measuredValue);
        validSampleCount++;
    }
}
double Validate::getRMSE(){
    if(mappingResult->DataType==CATEGORICAL||validSampleCount<1)
        return -1;
    double sum = 0;
    for(int i = 0;i < validSampleCount; i++){
        sum += pow(predictedValues[i]-measuredValues[i],2);
    }
    return sqrt(sum/sampleSize);
}
double Validate::getMAE(){
    if(mappingResult->DataType==CATEGORICAL||validSampleCount<1)
        return -1;
    double sum = 0;
    for(int i = 0;i < validSampleCount; i++){
        sum += fabs(predictedValues[i]-measuredValues[i]);
    }
    return sum/sampleSize;
}
double Validate::getPrecision(){
    if(mappingResult->DataType==CONTINUOUS || validSampleCount < 1)
        return -1;
    else {
        //todo
        return -1;
    }
}
double Validate::getRecall(){
    if(mappingResult->DataType==CONTINUOUS || validSampleCount < 1)
        return -1;
    else {
        //todo
        return -1;
    }
}
double Validate::getOverallAccuracy(){
    if(mappingResult->DataType==CONTINUOUS || validSampleCount < 1)
        return -1;
    else {
        //todo
        return -1;
    }
}
}
