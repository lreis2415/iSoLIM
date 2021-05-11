#pragma once
#ifndef CURVE_H_
#define CURVE_H_

#include <fstream>
#include <algorithm>
#include <numeric>
#include <third_party/tinyxml.h>
#include "DataTypeEnum.h"
#include "EnvLayer.h"
#include "preprocess.h"

#define VERY_SMALL 0.0001
using std::cout;
using std::endl;
using std::to_string;
using std::invalid_argument;

namespace solim {
	struct SoilProperty {
		string propertyName;
		double propertyValue;
        DataTypeEnum soilPropertyType;
	};

	class Curve {
	public:
		string covariateName;
		DataTypeEnum dataType;
		double typicalValue;
        double range;
        vector <double> vecKnotX;
        vector <double> vecKnotY;
    private:
        vector <double> vecDY;
		vector <double> vecDDY;
		vector <double> vecS;
		int iKnotNum;
	public:
		Curve();
		Curve(string covName, DataTypeEnum type);
        Curve(string covName, DataTypeEnum type, int knotNum, string coords, double valueRange=0);
		Curve(string covName, DataTypeEnum type, vector<double> *x, vector<double> *y);	// freehand rule
        Curve(string covName, double lowUnity, double highUnity, double lowCross, double highCross, CurveTypeEnum curveType);	// range rule
		Curve(string covName, double xcoord, double ycoord, EnvLayer *layer);	// point rule
		Curve(string covName, vector<float> *values);	// data mining continuous
		Curve(string covName, vector<int> *values);		// data mining categorical
		Curve(string covName, vector<Curve> *curves);
		double KernelEst(double x, int n, double h, vector<float> *values);
		void addKnot(double x, double y);
		double getOptimality(double x);
		void updateCurve();
        void changeCurve(Curve *c);
		int getKnotNum();
		string getCoords();

	private:
		void bubbleSort();
		void calcSpline();
		int bsearch(int low, int high, double x);
		//double rangeFunction(double value, double unity, double cross);
	};
	
}

#endif	//CURVE_H_
