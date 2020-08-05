#ifndef PROJECT_H
#define PROJECT_H
#include "solim-lib-forqt.h"
class SoLIMProject{
public:
    string projName;
    string workingDiret;
    vector<string> filenames;
    vector<string> layernames;
    vector<solim::Prototype> prototypes;
    vector<solim::Exception> exceptions;
};
#endif // PROJECT_H
