#include "io.h"

#include <string>
#include <vector>
#include <fstream>

#include "preprocess.h"
#include "EnvUnit.h"
#include "EnvDataset.h"
#include "EnvLayer.h"

// using namespace std; // Avoid this usage, instead of specific functions. 2019/08/06 ZHULJ
using std::ofstream;
using std::ifstream;

namespace solim {

bool WriteoutRaster(EnvLayer* envLayer, string filename, string type, GDALDataset* srcDs) {
    envLayer->Writeout(filename);
    return true;
}

vector<EnvUnit *> ReadTable(string filename,
                            EnvDataset* envDataset,
                            string targetVName/* = "None" */,
                            string xName,
                            string yName,
                            string idName/* = "None" */) {
    vector<EnvUnit *> envUnits;
    ifstream file(filename); // declare file stream:

    string line;
    getline(file, line);
    vector<string> names;
    int pos_X = 0;
    int pos_Y = 1;
    int pos_targetVName = -1;
    int pos_idName = 0;
    ParseStr(line, ',', names);
    for (int i = 0; i < names.size(); i++) {
        if (names[i] == "X" || names[i] == "x") {
            pos_X = i;
            break;
        }
    }
    for (int i = 0; i < names.size(); i++) {
        if (names[i] == "Y" || names[i] == "y") {
            pos_Y = i;
            break;
        }
    }
    if (targetVName != "None") {
        for (int i = 0; i < names.size(); i++) {
            if (names[i] == targetVName) {
                pos_targetVName = i;
                break;
            }
        }
    }
    if (idName != "None") {
        for (int i = 0; i < names.size(); i++) {
            if (names[i] == idName) {
                pos_idName = i;
                break;
            }
        }
    }

    while (getline(file, line)) {
        vector<string> values;
        ParseStr(line, ',', values);
        const char* xstr = values[pos_X].c_str();
        const char* ystr = values[pos_Y].c_str();
        double x = atof(xstr);
        double y = atof(ystr);

        double targetV = 0.0;
        if (targetVName != "None") {
            if (pos_targetVName == -1) {
                throw "Target name does not exist in the file.";
                return envUnits;
            }
            const char* targetVstr = values[pos_targetVName].c_str();
            targetV = atof(targetVstr);
        } else {
            pos_targetVName = 2;
            const char* targetVstr = values[pos_targetVName].c_str();
            targetV = atof(targetVstr);
        }
        string id = "";
        if (idName != "None") {
            id = values[pos_idName];
        }
        EnvUnit* e = envDataset->GetEnvUnit(x, y);
        if (e != NULL) {
            if (targetVName != "None") { e->SoilVariable = targetV; }
            if (idName != "None") { e->SampleID = id; }
            envUnits.push_back(e);
        }
    }
    file.close();
    return envUnits;
}

bool WriteTable(string filename, vector<EnvUnit *> envUnit) {
    if (envUnit.empty()) {
        return false;
    }
    ofstream file(filename);
    string firstLine = "X,Y\n";
    file << firstLine;

    double cellSize = envUnit[0]->CellSize;
    for (int i = 0; i < envUnit.size(); i++) {
        double x = envUnit[i]->Loc->X + cellSize / 2;
        double y = envUnit[i]->Loc->Y - cellSize / 2;
        string xstr = ConvertToString(x);
        string ystr = ConvertToString(y);
        string line = xstr + "," + ystr + "\n";
        file << line;
    }
    file.flush();
    file.close();
    return true;
}

bool WriteCSV_SampleCredibility(string filename, vector<EnvUnit *> samples) {
    if (samples.empty()) {
        return false;
    }
    ofstream file(filename);
    string firstLine = "ID,X,Y,Credibility,Number_Support,Number_Contradict\n";
    file << firstLine;

    double cellSize = samples[0]->CellSize;
    for (int i = 0; i < samples.size(); i++) {
        EnvUnit* e = samples[i];
        double x = e->Loc->X + cellSize / 2;
        double y = e->Loc->Y - cellSize / 2;
        string str_id = e->SampleID;
        string str_x = ConvertToString(x);
        string str_y = ConvertToString(y);
        string str_credibility = ConvertToString(e->Credibility);
        string str_number_support = ConvertToString(e->Number_Support);
        string str_number_contradict = ConvertToString(e->Number_Contradict);
        string line = str_id + "," + str_x + "," + str_y + "," + str_credibility + "," + str_number_support + "," +
                str_number_contradict + "\n";
        file << line;
    }
    file.flush();
    file.close();
    return true;
}
}
