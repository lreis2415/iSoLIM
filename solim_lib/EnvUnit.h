/*!
 * @brief Environmtan unit which stores multi-environmetal variables.
 * @revision  17-11-21 zhanglei - initial version
 *            17-11-27 lj       - code revision and format
 */
#ifndef ENVUNIT_HPP_
#define ENVUNIT_HPP_
#include <vector>
#include <string>

#include "DataTypeEnum.h"
#include "Location.h"

// using namespace std; // Avoid this usage, instead of specific functions. 2019/08/06 ZHULJ
using std::vector;
using std::string;

namespace solim {
class EnvUnit {
public:
    bool IsCal;                      // involed in calculation or not
	vector<string> LayerNames;
    vector<float> EnvValues;        // all environment values of the current cell
    vector<float> MembershipValues; // all fuzzy membership values of the current cell
    vector<DataTypeEnum> DataTypes;  // types of all environment variables
    vector<EnvUnit *> SimiEnvUnits;  // other cells that similar to the current one
    double SoilType;                 // soil type
    double SoilVariable;             // soil property
    string SampleID;                 // record sample ID
    Location* Loc;                   // location information
    double Uncertainty;              // uncertainty
    double Uncertainty_tmp;          // temporary uncertainty value
    double MaxSimi;                  // maximum similarity of the current cell to samples
    double CellSize;                 // cell size
    bool isCanPredict;               // can the current cell be predicted

    double PredictSoilVarible; // prediction value
    double PredictUncertainty; // uncertainty of prediction
    double PredictCredibility; // credibility of prediction

    double Credibility;    // For sample point only! credibility of sample point
    int Number_Support;    // samples number
    int Number_Contradict; // number of samples with contradiction

    int Density;  // density of points that have similarity values greater than threshold
    double DSimi; // similarity of the point that has higher density and highest similarity than the current cell

public:
    EnvUnit() {
        IsCal = true;
        Loc = new Location();
        Uncertainty = 1.0;
        Uncertainty_tmp = 1.0;
        MaxSimi = 0;
        CellSize = 10;
        isCanPredict = false;
        Credibility = 0;
        Number_Contradict = 0;
        Number_Support = 0;
        PredictCredibility = -1;
        PredictSoilVarible = -1;
        PredictUncertainty = -1;
        SoilType = 0.0;
        SoilVariable = 0.0;
        SampleID = "id-none";

        Density = 0;
        DSimi = 0;
    }

    ~EnvUnit() {
        delete Loc;
    }

	void AddEnvValue(string layername, double envValue, DataTypeEnum type) {
		LayerNames.push_back(layername);
		EnvValues.push_back(envValue);
		DataTypes.push_back(type);
	}

    void AddEnvValue(const double envValue) {
        EnvValues.push_back(envValue);
    }

    void AddMembershipValue(const double membershipValue) {
        MembershipValues.push_back(membershipValue);
    }
};
}

#endif
