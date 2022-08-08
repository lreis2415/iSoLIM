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
    string currentBaseName;
    string currentResultName;
    vector<string> filenames;
    vector<string> layernames;
    vector<string> layertypes;
    vector<float> layerDataMax;
    vector<float> layerDataMin;
    vector<solim::Prototype> prototypes;
    //vector<solim::Exception> exceptions;
    vector<string> prototypeBaseNames;
    vector<string> prototypeBaseTypes;
    vector<string> results;
    vector<float> resultDataMax;
    vector<float> resultDataMin;
    QString workingDir;
public:
    SoLIMProject(){
        projName = "";
        projFilename = "";
        studyArea = "";
        currentBaseName = "";
        currentResultName = "";
        filenames.clear();
        layernames.clear();
        layertypes.clear();
        prototypes.clear();
        //exceptions.clear();
        prototypeBaseNames.clear();
        layerDataMax.clear();
        layerDataMin.clear();
        resultDataMax.clear();
        resultDataMin.clear();
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
        layerDataMax.push_back(NODATA);
        layerDataMin.push_back(NODATA);
        return true;
    }
    bool addResult(string filename, float dataMax=NODATA, float dataMin=NODATA){
        bool fileExist = false;
        for(size_t i = 0; i< results.size(); i++){
            if(results[i]==filename){
                fileExist = true;
                string tmpImg = filename+".png";
                QFile tmpImgFile(tmpImg.c_str());
                if(tmpImgFile.exists()){
                    tmpImgFile.remove();
                }
                return false;
            }
        }
        if(!fileExist){
            results.push_back(filename);
            resultDataMax.push_back(dataMax);
            resultDataMin.push_back(dataMin);
            return true;
        }
    }
};
#endif // PROJECT_H
