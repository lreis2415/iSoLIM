#ifndef PROTOTYPE_H
#define PROTOTYPE_H

#pragma once
#include <vector>
#include <fstream>
#include <stdexcept>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include "EnvDataset.h"
#include "EnvUnit.h"
#include "Curve.h"
using std::ifstream;
using std::ofstream;

namespace solim {
	class Prototype {
	public:
		string prototypeID;
		vector<Curve> envConditions;
		vector<SoilProperty> properties;
	public:
		double uncertainty;
		int envConditionSize;
		PrototypeSource source;
	private:
		bool envConsIsSorted;

	public:
		Prototype();
		static vector<Prototype> *getPrototypesFromSample(string filename, EnvDataset* eds);
		static vector<Prototype> *getPrototypesFromMining(string filename, EnvDataset* eds);
		static vector<Prototype> *getPrototypesFromMining(string filename, EnvDataset *eds, string soilIDFieldName);
		void addConditions(string filename);
		void addConditions(Curve con) { envConditions.push_back(con); envConditionSize++; }
		void addProperties(string propertyName, double propertyValue/*, DataTypeEnum type*/);
		double getProperty(string propName);
		void writeRules(string fileName);
		void writePrototype(string fileName);
		void readPrototype(string filename);
		double calcSimi(EnvUnit *e);
		double calcSimi_preChecked(EnvUnit *e);
		bool checkEnvConsIsSorted(EnvDataset *eds);
		void sortEnvCons(vector<string> layernames);
		static double getOptimality(vector<Prototype> *prototypes, EnvUnit *e, string soilPropertyName, double soilTypeTag);
	};
}

#endif // !MEMBERSHIPRULE_H
