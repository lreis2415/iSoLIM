#ifndef VALIDATE_H
#define VALIDATE_H

#include "EnvDataset.h"
#include "io.h"
namespace solim{
class Validate
{
public:
    EnvLayer *mappingResult;
    vector<EnvUnit*> samples;
    vector<double> measuredValues;
    vector<double> predictedValues;
    int sampleSize;
    int validSampleCount;
public:
    Validate(string sampleFile, string resultFile,
             string resultDatatype, string xFiled, string yField, string targetField);
    double getRMSE();
    double getMAE();
    double getPrecision();
    double getRecall();
    double getOverallAccuracy();
    int getValidSampleCoutn() { return validSampleCount; }
};
}

#endif // VALIDATE_H
