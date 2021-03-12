#ifndef PROJECT_H
#define PROJECT_H
#include "solim_lib/Prototype.h"
#include "solim_lib/preprocess.h"
#include <QMessageBox>
#include <QFileInfo>
class SoLIMProject{
public:
    string projName;
    string projFilename;
    string studyArea;
    vector<string> filenames;
    vector<string> layernames;
    vector<string> layertypes;
    vector<solim::Prototype> prototypes;
    //vector<solim::Exception> exceptions;
    vector<string> prototypeBaseNames;
    vector<string> results;
    QString workingDir;
public:
    SoLIMProject(){
        projName = "";
        projFilename = "";
        studyArea="";
        filenames.clear();
        layernames.clear();
        layertypes.clear();
        prototypes.clear();
        //exceptions.clear();
        prototypeBaseNames.clear();
    }
    bool addLayer(string layername, string datatype, string filename=""){
        for(int i = 0;i<layernames.size();i++){
            if(layernames[i]==layername)
                return false;
        }
        if(filename!="")
            if(!QFileInfo(filename.c_str()).exists())
                filename="";
        layernames.push_back(layername);
        layertypes.push_back(datatype);
        filenames.push_back(filename);
        return true;
    }
};
#endif // PROJECT_H
