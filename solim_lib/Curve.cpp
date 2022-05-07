#include "Curve.h"

namespace solim {
	Curve::Curve() {
		covariateName = "";
		dataType = CONTINUOUS;	// default data type is continuous
		vecKnotX.clear();
		vecKnotY.clear();
		vecDY.clear();
		vecDDY.clear();
		vecS.clear();
		iKnotNum = 0;
        range = 0;
		typicalValue = NODATA;
	}

	Curve::Curve(string covName, DataTypeEnum type, vector<double> *x, vector<double> *y) {
		// knowledge from experts: freehand rule
		covariateName = covName;
		dataType = type;
		vecKnotX = *x;
		vecKnotY = *y;
		iKnotNum = vecKnotX.size();
		typicalValue = NODATA;
        range=0;
		if (iKnotNum != vecKnotY.size()) {
			throw invalid_argument( "Error in knot coordinates");
		}
		for (int i = 0; i<iKnotNum; i++) {
			if (fabs(vecKnotY[i] - 1)<VERY_SMALL) {
				typicalValue = vecKnotX[i];
				break;
			}
        }
        bubbleSort();
        range=fabs(vecKnotX[0])>fabs(vecKnotX[iKnotNum-1])?fabs(vecKnotX[0]):fabs(vecKnotX[iKnotNum-1]);
		if (dataType == CONTINUOUS) {
            calcSpline();
        }
	}

	Curve::Curve(string covName, DataTypeEnum type) {
		covariateName = covName;
		dataType = type;	
		vecKnotX.clear();
		vecKnotY.clear();
		vecDY.clear();
		vecDDY.clear();
		vecS.clear();
		iKnotNum = 0;
		typicalValue = NODATA;
        range=0;
	}

    Curve::Curve(string covName, DataTypeEnum type, int knotNum, string coords, double valueRange) {
		// knowledge from experts: word rule
		covariateName = covName;
		dataType = type;
		vecKnotX.clear();
		vecKnotY.clear();
		vecDY.clear();
		vecDDY.clear();
		vecS.clear();
		iKnotNum = knotNum;
        range = valueRange;

		vector<string> xycoord;
		ParseStr(coords, ',', xycoord);
		if (xycoord.size() != iKnotNum) iKnotNum = xycoord.size();
		vecKnotX.reserve(iKnotNum);
		vecKnotY.reserve(iKnotNum);

		for (int i = 0; i < iKnotNum; ++i) {
			vector<string> coord;
			ParseStr(xycoord[i], ' ', coord);
			if (coord.size() != 2)
				throw invalid_argument("invalid coordinate description.");
			vecKnotX.push_back(atof(coord[0].c_str()));
			vecKnotY.push_back(atof(coord[1].c_str()));
			if (fabs(atof(coord[1].c_str()) - 1)<VERY_SMALL)
				typicalValue = atof(coord[0].c_str());
		}
		if (type == CATEGORICAL || knotNum < 1) return;
		bubbleSort();
		calcSpline();			
	}

    Curve::Curve(string covName, double lowUnity, double highUnity, double lowCross, double highCross, CurveTypeEnum curveType) {
		// knowledge from experts: range rule
		covariateName = covName;
		dataType = CONTINUOUS;
		vecKnotX.clear();
		vecKnotY.clear();
		vecDY.clear();
		vecDDY.clear();
		vecS.clear();
		iKnotNum = 0;
        double rangePar=2.5775678826705466;  //sqrt(ln(0.01)/ln(0.5))
        double rangePar_75=0.64423404076379;    //sqrt(ln(0.75)/ln(0.5))
        double rangePar_25 = 1.41421356237309;
        double lowRange=lowUnity-(lowUnity - lowCross)*rangePar;
        double low_75=lowUnity-(lowUnity - lowCross)*rangePar_75;
        double highRange=highUnity+(highCross-highUnity)*rangePar;
        double high_75=highUnity+(highCross-highUnity)*rangePar_75;
        range=fabs(lowRange)>fabs(highRange)?fabs(lowRange):fabs(highRange);

		// add 6 points to generate the spline curve
		// two unity points, two range points, and two cross points
		if (curveType == BELL_SHAPED) {
            addKnot(lowRange, 0);
            addKnot(lowUnity-(lowUnity - lowCross)*rangePar_25,0.25);
            addKnot(lowCross, 0.5);
            addKnot(low_75,0.75);
            addKnot(lowUnity, 1);
            addKnot(highUnity, 1);
            addKnot(high_75,0.75);
            addKnot(highCross, 0.5);
            addKnot(highUnity+(highCross-highUnity)*rangePar_25,0.25);
            addKnot(highRange, 0);
            typicalValue = (highUnity+lowUnity)*0.5;
		}
		else if (curveType == S_SHAPED) {
            range=fabs(range)>fabs(lowUnity+1)?fabs(range):fabs(lowUnity+1);
            addKnot(lowRange, 0);
            addKnot(lowUnity-(lowUnity - lowCross)*rangePar_25,0.25);
            addKnot(lowCross, 0.5);
            addKnot(low_75,0.75);
            addKnot(lowUnity, 1);
            typicalValue = lowUnity;
		}
		else if (curveType == Z_SHAPED) {
            range=fabs(range)>fabs(highUnity-1)?fabs(range):fabs(highUnity-1);
            addKnot(highUnity, 1);
            addKnot(high_75,0.75);
            addKnot(highCross, 0.5);
            addKnot(highUnity+(highCross-highUnity)*rangePar_25,0.25);
            addKnot(highRange, 0);
            typicalValue = lowUnity;
		}
		calcSpline();
	}

	Curve::Curve(string covName, double x, double y, EnvLayer *layer) {
		// knowledge from sample
		// knowledge from experts: point rule
		covariateName = covName;
		dataType = layer->DataType;
		iKnotNum = 0;
		vecKnotX.clear();
		vecKnotY.clear();
		vecDY.clear();
		vecDDY.clear();
		vecS.clear();
		int row, col;
        double max = layer->Data_Max;
        double min = layer->Data_Min;
        range = (fabs(max)>fabs(min))?fabs(max):fabs(min);
		//int halfKnotNum = 3;
		layer->baseRef->geoToGlobalXY(x, y, col, row);
		typicalValue = layer->baseRef->getValue(col, row);
		if (dataType == CATEGORICAL) {
			addKnot(typicalValue, 1);
			return;
		}
		int cellNum = 0;
		double sum = 0;
		double squareSum = 0;
		double SDjSquareSum = 0;
		for (int k = 0; k < layer->BlockSize; ++k) {
			layer->ReadByBlock(k);
			int n = layer->XSize*layer->GetYSizeByBlock(k);
			for (int i = 0; i < n; ++i) {
				double value = layer->EnvData[i];
				if (fabs(value - layer->NoDataValue) < VERY_SMALL||value<NODATA) continue;
				sum += value;
				squareSum += value*value;
				SDjSquareSum += pow(value - typicalValue, 2);
				++cellNum;
			}
		}
		double mean = sum / (double)cellNum;
		double SDSquare = squareSum / (double)cellNum - mean * mean;
		double SDjSquare = SDjSquareSum / (double)cellNum;

        double zeroPar = SDSquare/sqrt(SDjSquare)*3.03485425877029270172;    // 3.034=sqrt(-2*ln0.01);
        double _1stQuaPar = SDSquare/sqrt(SDjSquare)*1.66510922231539551270; // 1.665=sqrt(-2*ln0.25);
        double halfPar = SDSquare/sqrt(SDjSquare)*1.17741002251547469101;    // 1.117=sqrt(-2*ln0.5);
        double _3rdQuaPar = SDSquare/sqrt(SDjSquare)*0.75852761644093213257; // 1.665=sqrt(-2*ln0.75);
        addKnot(typicalValue - zeroPar,0);
        addKnot(typicalValue - _1stQuaPar, 0.25);
        addKnot(typicalValue - halfPar,0.5);
        addKnot(typicalValue - _3rdQuaPar, 0.75);
        addKnot(typicalValue,1);
        addKnot(typicalValue + _3rdQuaPar, 0.75);
        addKnot(typicalValue + halfPar, 0.5);
        addKnot(typicalValue + _1stQuaPar, 0.25);
        addKnot(typicalValue + zeroPar, 0);
		calcSpline();
	}

	Curve::Curve(string covName, vector<float> *values) {	// add rules from data mining-continuous
		covariateName = covName;
		dataType = CONTINUOUS;
		iKnotNum = 0;
		vecKnotX.clear();
		vecKnotY.clear();
		vecDY.clear();
		vecDDY.clear();
		vecS.clear();
		int n = values->size();
		if (n < 4) {
			throw invalid_argument("Knot number less than 3, unable to generate spline");
			return;
		}
		double mean = std::accumulate(values->begin(), values->end(), 0.0) / n;
		double sq_sum = std::inner_product(values->begin(), values->end(), values->begin(), 0.0);
		double stdev = std::sqrt(sq_sum / n - mean * mean);
		int const Q1 = n / 4;
		int const Q3 = 3 * Q1;
		std::nth_element(values->begin(), values->begin() + Q1, values->end());
		std::nth_element(values->begin() + Q1 + 1, values->begin() + Q3, values->end());
		double quartile = values->at(Q3)-values->at(Q1);
		double p = (stdev < quartile) ? stdev : quartile;
        double h = 1.06*p*pow(n, -0.2);	// 1.06min(std,quartile range)n^(-0.2)
		float xmin = *std::min_element(values->begin(),values->end());	//minimum value
		float xrange = *std::max_element(values->begin(), values->end())-xmin;
        if(xrange<VERY_SMALL){
            addKnot(xmin, 1);
            return;
        }
        range = (fabs(xrange+xmin)>fabs(xmin))?fabs(xrange+xmin):fabs(xmin);
		double x_pre = xmin;
		double y_pre = KernelEst(x_pre, n, h, values);
		addKnot(x_pre, y_pre);
		double ymax = y_pre, ymin = y_pre;
		int interval_num = xrange / h * 10;
        if(interval_num>100) interval_num = 100;
		double interval= xrange/interval_num;	//10*h interval
		double x = xmin + interval;
		double y = KernelEst(x, n, h, values);
		double x_next,y_next;
		for (int i = 1; i < interval_num; i++) {
			x_next = x + interval;	//x_min+interval*(i+1)
			y_next= KernelEst(x_next, n, h, values);
			double flag = (y - y_pre)*(y - y_next);
            if(!(flag < 0)){
                addKnot(x_pre, y_pre);
                addKnot(x, y);
                addKnot(x_next, y_next);
                if (y_pre > ymax) { ymax = y_pre; typicalValue = x_pre;}
                if (y_pre < ymin) ymin = y_pre;
            }
			x_pre = x;
			y_pre = y;
			x = x_next;
			y = y_next;
		}
		if (iKnotNum == 1) {
			x = xmin + xrange*0.5;
			y = KernelEst(xmin + xrange*0.5, n, h, values);
			addKnot(x, y);
			if (y > ymax) { ymax = y; typicalValue = x; }
			if (y < ymin) ymin = y;
		}
		addKnot(x_next, y_next);
		if (y_next > ymax) { ymax = y_next; typicalValue = x_next; }
		if (y_next < ymin) ymin = y_next;
		// strech y to 0-1
		if (ymax > ymin && !(fabs(ymax - 1)<VERY_SMALL && fabs(ymin)<VERY_SMALL)) {
			double strechRatio = 1.0 / (ymax - ymin);
			for (int i = 0; i < iKnotNum; i++) vecKnotY[i] = (vecKnotY[i] - ymin) * strechRatio;
		}
		bubbleSort();
        if(1 - vecKnotY[0]>VERY_SMALL) vecKnotY[0] = 0;
        if(1-vecKnotY[iKnotNum-1]>VERY_SMALL) vecKnotY[iKnotNum-1]=0;
		calcSpline();
	}
	Curve::Curve(string covName, vector<Curve> *curves) {
        covariateName = covName;
        dataType = curves->at(0).dataType;
        iKnotNum = 0;
        vecKnotX.clear();
        vecKnotY.clear();
        vecDY.clear();
        vecDDY.clear();
        vecS.clear();
        typicalValue = curves->at(0).typicalValue;
        // if curves are categorical
        if(dataType==CATEGORICAL){
            for (size_t i = 0; i < curves->size(); ++i) {
                for(size_t j = 0; j<curves->at(i).iKnotNum;j++){
                    addKnot(curves->at(i).vecKnotX[j],curves->at(i).vecKnotY[j]);
                }
            }
            bubbleSort();
            return;
        }
        // if the curves are continuous;
        int knotNumSum = 0;
        vector<float> vecXCollect;
        for (int i = 0; i < curves->size(); ++i) {
            knotNumSum += curves->at(i).iKnotNum;
            vecXCollect.insert(vecXCollect.end(), curves->at(i).vecKnotX.begin(), curves->at(i).vecKnotX.end());
        }
        std::sort(vecXCollect.begin(), vecXCollect.end());
        float x_pre = vecXCollect[0];
        float y_pre = curves->at(0).getOptimality(x_pre);
        float x, y, x_next, y_next;
        for (int n = 1; n < curves->size(); ++n) {
            float y_tmp = curves->at(n).getOptimality(x_pre);
            if (y_tmp > y_pre) y_pre = y_tmp;
        }
        addKnot(x_pre, y_pre);
        x = x_pre + (vecXCollect[1] - x_pre)*0.1;
        y = curves->at(0).getOptimality(x);
        for (int n = 1; n < curves->size(); ++n) {
            float y_tmp = curves->at(n).getOptimality(x);
            if (y_tmp > y) y = y_tmp;
        }
        for (int i = 1; i < vecXCollect.size(); ++i) {
            float x_interval = (vecXCollect[i] - vecXCollect[i-1])*0.1;
            int k = 0;
            if (i == 1) k = 1;
            for (; k < 10; k++) {
                x_next = x + x_interval;
                y_next = curves->at(0).getOptimality(x_next);
                for (int n = 1; n < curves->size(); ++n) {
                    float y_tmp = curves->at(n).getOptimality(x_next);
                    if (y_tmp > y_next) y_next = y_tmp;
                }
                if ((y - y_pre)*(y - y_next) < 0);
                else if (fabs(y - y_pre) < VERY_SMALL&&fabs(y - y_next) < VERY_SMALL);
                else {
                    addKnot(x_pre, y_pre);
                    addKnot(x, y);
                    addKnot(x_next, y_next);
                }
                x_pre = x;
                y_pre = y;
                x = x_next;
                y = y_next;
            }
        }
        addKnot(x_next, y_next);
        bubbleSort();
        if(1 - vecKnotY[0]>VERY_SMALL) vecKnotY[0] = 0;
        if(1-vecKnotY[iKnotNum-1]>VERY_SMALL) vecKnotY[iKnotNum-1]=0;
        if(iKnotNum > 3)  calcSpline();
	}

	double Curve::KernelEst(double x,int n,double h, vector<float> *values){
		double sum = 0;
		for (int i = 0; i < values->size(); i++) {
			sum += exp(-pow(x - values->at(i), 2) / h*0.5);
		}
		return sum / (n*h*sqrt(2 * PI));
	}

	Curve::Curve(string covName, vector<int> *values) {	// add rules from data mining-categorical
        int count = values->size();
        sort(values->begin(),values->end());
        std::vector<int>::iterator unique_it;
        unique_it = std::unique(values->begin(), values->end());
        vector<float> freq;
        float max_freq = 0;
        for (std::vector<int>::iterator it = values->begin(); it!=unique_it; ++it)
		{
            int tmp_mode = *it;
            int tmp_count = std::count(values->begin(), values->end(), tmp_mode);
            float tmp_freq = float(tmp_count)/count;
            freq.push_back(tmp_freq);
            if(tmp_freq>max_freq) {
                typicalValue = tmp_mode;
                max_freq = tmp_freq;
            }
        }
        // select values whose frequency is more than 0.5*max_freq as modes
        /*float freq_threshold = 0.5;
        int i = 0;
        for (std::vector<float>::iterator it = freq.begin(); it!=freq.end(); ++it)
        {
            if(*it>freq_threshold*max_freq){
                vecKnotX.push_back(values->at(i));
                vecKnotY.push_back(1);
            }
            i++;
        }*/

		covariateName = covName;
		dataType = CATEGORICAL;
		iKnotNum = 1;
		vecKnotX.clear();
		vecKnotY.clear();
		vecDY.clear();
		vecDDY.clear();
		vecS.clear();
        // add only typical value as only mode
        vecKnotX.push_back(typicalValue);
        vecKnotY.push_back(1);
	}

	void Curve::addKnot(double x, double y) {
		for (int i = 0; i < iKnotNum; ++i) {
			if (fabs(vecKnotX[i] - x) < VERY_SMALL)
                return;
		}
        vecKnotX.push_back(x);
        vecKnotY.push_back(y);
        ++iKnotNum;
	}

	void Curve::updateCurve() {
		iKnotNum = vecKnotX.size();
		if (vecKnotY.size() != iKnotNum) {
			throw invalid_argument ("Error in knot coordinates");
		}
		bubbleSort();
		calcSpline();
	}

    void Curve::changeCurve(Curve*c) {
        dataType=c->dataType;
        vecKnotX.clear();
        vecKnotX.shrink_to_fit();
        vecKnotY.clear();
        vecKnotY.shrink_to_fit();
        vecDY.clear();
        vecDY.shrink_to_fit();
        vecDDY.clear();
        vecDDY.shrink_to_fit();
        vecS.clear();
        vecS.shrink_to_fit();
        vecKnotX.insert(vecKnotX.end(), c->vecKnotX.begin(),c->vecKnotX.end());
        vecKnotY.insert(vecKnotY.end(), c->vecKnotY.begin(),c->vecKnotY.end());
        vecDY.insert(vecDY.end(), c->vecDY.begin(),c->vecDY.end());
        vecDDY.insert(vecDDY.end(), c->vecDDY.begin(),c->vecDDY.end());
        vecS.insert(vecS.end(), c->vecS.begin(),c->vecS.end());
        iKnotNum = c->iKnotNum;
        typicalValue = c->typicalValue;
    }
	double Curve::getOptimality(double envValue) {
		// for categorical value
		if (dataType == CATEGORICAL) {
			for (int i = 0; i < iKnotNum; ++i)
                if (fabs(envValue - vecKnotX[i]) < VERY_SMALL)
					return vecKnotY[i];
			return 0;
		}
		// for continuous value
        else{
            if (iKnotNum < 3){
                for (int i = 0; i < iKnotNum; ++i)
                    if (fabs(envValue - vecKnotX[i]) < VERY_SMALL)
                        return vecKnotY[i];
                return 0;
            }
            double result;
            if (envValue < vecKnotX[0]){
                if(vecKnotY[0]<VERY_SMALL || fabs(vecKnotY[0]-1) < VERY_SMALL) return vecKnotY[0];
                result = vecDY[0] * (envValue - vecKnotX[0]) + vecKnotY[0];
            }
            else if (envValue > vecKnotX[iKnotNum - 1]){
                if(vecKnotY[iKnotNum - 1]<VERY_SMALL || fabs(vecKnotY[iKnotNum - 1]-1) < VERY_SMALL) return vecKnotY[iKnotNum - 1];
                result = vecDY[iKnotNum - 1] * (envValue - vecKnotX[iKnotNum - 1]) + vecKnotY[iKnotNum - 1];
            }
            else {
                int i = 0;
                while (envValue>vecKnotX[i + 1])
                    i = i + 1;
                double h0, h1;
                h1 = (vecKnotX[i + 1] - envValue) / vecS[i];
                h0 = h1*h1;
                result = (3.0*h0 - 2.0*h0*h1)*vecKnotY[i];
                result += vecS[i] * (h0 - h0*h1)*vecDY[i];
                h1 = (envValue - vecKnotX[i]) / vecS[i];
                h0 = h1*h1;
                result += (3.0*h0 - 2.0*h0*h1)*vecKnotY[i + 1];
                result -= vecS[i] * (h0 - h0*h1)*vecDY[i + 1];
            }
            if (result>1)
                result = 1;
            else if (result<0)
                result = 0;
            return result;
        }
	}

	int Curve::getKnotNum() {
		iKnotNum = vecKnotX.size();
		if (iKnotNum == vecKnotY.size()) return iKnotNum;
		else throw invalid_argument("Error: inconsistent knotX number and knotY number.");
	}
	string Curve::getCoords() {
		iKnotNum = vecKnotX.size();
		if (iKnotNum != vecKnotY.size())
			throw invalid_argument("Error: inconsistent knotX number and knotY number.");
		string coords = "";
		for (int i = 0; i < iKnotNum; ++i) {
			string tmp;
			if (i == iKnotNum - 1)
				tmp = to_string(vecKnotX[i]) + " " + to_string(vecKnotY[i]);
			else
				tmp = to_string(vecKnotX[i]) + " " + to_string(vecKnotY[i]) + ",";
			coords += tmp;
		}
		return coords;
	}


	void Curve::bubbleSort() {
		double tempx, tempy;
		for (int i = 0; i<iKnotNum; ++i) {
			for (int j = 0; j<iKnotNum - 1; ++j) {
				if (vecKnotX[j]>vecKnotX[j + 1]) {
					tempx = vecKnotX[j + 1];
					vecKnotX[j + 1] = vecKnotX[j];
					vecKnotX[j] = tempx;
					tempy = vecKnotY[j + 1];
					vecKnotY[j + 1] = vecKnotY[j];
					vecKnotY[j] = tempy;
				}
			}
		}
	}

	void Curve::calcSpline() {
		if (iKnotNum < 3) {
            cout<<"Knot number less than 3, unable to generate spline";
			return;
		}
		vecDY.reserve(iKnotNum);
		vecDY.resize(iKnotNum);
		vecDDY.reserve(iKnotNum);
		vecDDY.resize(iKnotNum);
		vecS.reserve(iKnotNum);
		vecS.resize(iKnotNum);

		int i, j;
		double h0, h1, alpha, beta;

		vecDDY[0] = 0;
		vecDDY[iKnotNum - 1] = 0;
		vecDY[0] = -0.5;
		h0 = vecKnotX[1] - vecKnotX[0];
		vecS[0] = 3.0 * (vecKnotY[1] - vecKnotY[0]) / (2.0 * h0) - vecDDY[0] * h0 / 4.0;
		for (j = 1; j <= iKnotNum - 2; ++j)
		{
			h1 = vecKnotX[j + 1] - vecKnotX[j];
			alpha = h0 / (h0 + h1);
			beta = ((1.0 - alpha)*(vecKnotY[j] - vecKnotY[j - 1])) / h0;
			beta = 3.0*beta + 3.0*alpha*(vecKnotY[j + 1] - vecKnotY[j]) / h1;
			vecDY[j] = -alpha / (2.0 + ((1.0 - alpha)*vecDY[j - 1]));
			vecS[j] = beta - (1.0 - alpha)*vecS[j - 1];
			vecS[j] = vecS[j] / (2.0 + ((1.0 - alpha)*vecDY[j - 1]));
			h0 = h1;
		}
		vecDY[iKnotNum - 1] = (3.0*(vecKnotY[iKnotNum - 1] - vecKnotY[iKnotNum - 2]) / h1 + vecDDY[iKnotNum - 1] * h1 / 2 - vecS[iKnotNum - 2]) / (2.0 + vecDY[iKnotNum - 2]);
		for (j = iKnotNum - 2; j >= 0; j--)
			vecDY[j] = vecDY[j] * vecDY[j + 1] + vecS[j];

		for (j = 0; j <= iKnotNum - 2; ++j)
			vecS[j] = vecKnotX[j + 1] - vecKnotX[j];

		for (j = 0; j <= iKnotNum - 2; ++j)
		{
			h1 = vecS[j] * vecS[j];
			vecDDY[j] = 6.0*(vecKnotY[j + 1] - vecKnotY[j]) / h1 - 2.0*(2.0*vecDY[j] + vecDY[j + 1]) / vecS[j];
		}
		h1 = vecS[iKnotNum - 2] * vecS[iKnotNum - 2];
		vecDDY[iKnotNum - 1] = 6 * (vecKnotY[iKnotNum - 2] - vecKnotY[iKnotNum - 1]) / h1 + 2 * (2 * vecDY[iKnotNum - 1] + vecDY[iKnotNum - 2]) / vecS[iKnotNum - 2];
	}

	int Curve::bsearch(int low, int high, double envValue)
	{
		if (low > high || low == high) return -1;
		int mid = (low + high) / 2;

		if (vecKnotX[mid] < envValue || fabs(vecKnotX[mid] - envValue) < VERY_SMALL) {
			if (vecKnotX[mid + 1] > envValue || fabs(vecKnotX[mid + 1] - envValue) < VERY_SMALL) {
				return mid;
			}
			return bsearch(mid + 1, high, envValue);
		}
		else if (vecKnotX[mid]>envValue) {
			if (mid < 1)	return -1;
			if (vecKnotX[mid - 1] < envValue) {
				return mid - 1;
			}
			return bsearch(low, mid - 1, envValue);
		}
	}

	/*double Curve::rangeFunction(double value, double unity, double cross) {
		return exp(pow(fabs(value - unity) / (unity - cross), 2)*log(0.5));
	}*/


}
