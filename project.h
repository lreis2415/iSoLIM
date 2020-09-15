#ifndef PROJECT_H
#define PROJECT_H
#include "solim-lib-forqt.h"
class SoLIMProject{
public:
    string projName;
    string projFilename;
    string studyArea;
    vector<string> filenames;
    vector<string> layernames;
    vector<string> layertypes;
    vector<solim::Prototype> prototypes;
    vector<solim::Exception> exceptions;
    vector<string> prototypeBaseNames;
    vector<string> propertyNames;
    vector<string> results;
public:
    SoLIMProject(){
        projName = "";
        projFilename = "";
        filenames.clear();
        layernames.clear();
        layertypes.clear();
        prototypes.clear();
        exceptions.clear();
        prototypeBaseNames.clear();
        propertyNames.clear();
    }
};
#endif // PROJECT_H
