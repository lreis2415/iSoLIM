#ifndef PROJECT_H
#define PROJECT_H
#include "solim-lib-forqt.h"
#include <QMessageBox>
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
    vector<string> results;
    vector<string> noFileLayers;
    vector<string> noFileDatatypes;
public:
    SoLIMProject(){
        projName = "";
        projFilename = "";
        studyArea="";
        filenames.clear();
        layernames.clear();
        layertypes.clear();
        prototypes.clear();
        exceptions.clear();
        prototypeBaseNames.clear();
        noFileLayers.clear();
        noFileDatatypes.clear();
    }
    bool addLayer(string layername, string datatype, string filename=""){
        for(int i = 0;i<noFileLayers.size();i++){
            if(noFileLayers[i]==layername)
                return false;
        }
        for(int i = 0;i<layernames.size();i++){
            if(layernames[i]==layername)
                return false;
        }
        if(filename==""){
            noFileLayers.push_back(layername);
            noFileDatatypes.push_back(datatype);
        } else {
            layernames.push_back(layername);
            layertypes.push_back(datatype);
            filenames.push_back(filename);
        }
        return true;
    }
};
#endif // PROJECT_H
