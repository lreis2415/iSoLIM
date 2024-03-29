#pragma once
#ifndef INFERENCE_HPP_
#define INFERENCE_HPP_

#include "EnvDataset.h"
#include "Prototype.h"
#include <vector>
#include <omp.h>
#include <QProgressBar>
#include <QDebug>

#define NODATA -9999
#define MAXLN 128
#define MAXLN_LAYERS 128
//#define EXPERIMENT
namespace solim {
    class Inference {
    public:
        EnvDataset *EDS;
        vector<Prototype>* Prototypes;
        string outSoilFilename;
        BaseIO *outSoilMap;
        string outUncerFilename;
        BaseIO *outUncerMap;
        double Threshold;
        IntegrationMethod Integrate;
#ifdef EXPERIMENT
        double compute_time;
#endif
    public:
        Inference(EnvDataset *eds, vector<Prototype>* prototypes, double threshold,
                  string outSoilFile, string outUncerFile,IntegrationMethod integrate = MINIMUM);
        void Mapping(string targetVName,QProgressBar *progressBar);
        void MappingCategorical(string targetVName,string membershipFolder,QProgressBar *progressBar);
        static void inferMap(EnvDataset *eds, vector<Prototype>* prototypes, string targetVName, double threshold,
                             string outSoilFile, string outUncerFile, QProgressBar *progressBar, IntegrationMethod integrate = MINIMUM);
        static void inferCategoricalMap(EnvDataset *eds, vector<Prototype>* prototypes, string targetVName, double threshold,
                                        string outSoilFile, string outUncerFile, string membershipFolder, QProgressBar *progressBar);
        static void propertyInference(vector<string> filenames, vector<string> datatypes, vector<string> layernames,
                                      double threshold, string sampleFilename, string targetVName, string outSoil,
                                      string outUncer, double ramEfficient, QProgressBar *progressBar);
        static void typeInference(vector<string> filenames, vector<string> datatypes, vector<string> layernames,
                                  double threshold, string sampleFilename, string targetVName, string outSoilFile,
                                  string outUncerFile, double ramEfficient, string membershipFolder, QProgressBar *progressBar);
    };
}

#endif
