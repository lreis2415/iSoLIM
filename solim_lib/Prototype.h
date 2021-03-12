#ifndef PROTOTYPE_H
#define PROTOTYPE_H

#pragma once
#include <vector>
#include <fstream>
#include <stdexcept>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <QProgressBar>
#include <omp.h>
#include <QDebug>
#include "EnvDataset.h"
#include "EnvUnit.h"
#include "Curve.h"
using std::ifstream;
using std::ofstream;

namespace solim {
	class Prototype {
	public:
        string prototypeBaseName;
		string prototypeID;
		vector<Curve> envConditions;
		vector<SoilProperty> properties;
		double uncertainty;
		int envConditionSize;
		PrototypeSource source;
	private:
		bool envConsIsSorted;

	public:
		Prototype();
        Prototype(EnvDataset* eds, int iSoilIDField, string prototypeBasename, OGRFeature* poFeature, int fid);
        Prototype(EnvDataset* eds, int iSoilIDField, string prototypeBasename, OGRLayer* poLayer, vector<int> fids);
        static vector<Prototype> *getPrototypesFromSample(string filename, EnvDataset* eds, string prototypeBaseName="", string xfield="x", string yfield="y");
        static vector<Prototype> *getPrototypesFromMining_soilType(string filename, EnvDataset* eds, string soilIDFieldName, string prototypeBasename, QProgressBar *progressBar);
        static vector<Prototype> *getPrototypesFromMining_polygon(string filename, EnvDataset *eds, string soilIDFieldName, string prototypeBasename, QProgressBar *progressBar);
        void addConditions(string filename);
		void addConditions(Curve con) { envConditions.push_back(con); envConditionSize++; }
        void addProperties(string propertyName, double propertyValue, DataTypeEnum type=CONTINUOUS);
		double getProperty(string propName);
		void writeRules(string fileName);
		void writePrototype(string fileName);
        TiXmlElement* writePrototypeXmlElement();
		void readPrototype(string filename);
		double calcSimi(EnvUnit *e);
		double calcSimi_preChecked(EnvUnit *e);
		bool checkEnvConsIsSorted(EnvDataset *eds);
		void sortEnvCons(vector<string> layernames);
		static double getOptimality(vector<Prototype> *prototypes, EnvUnit *e, string soilPropertyName, double soilTypeTag);

	};
}

#endif // !MEMBERSHIPRULE_H
