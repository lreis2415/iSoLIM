#ifndef SOILINFERENCEFROMSAMPLES_H
#define SOILINFERENCEFROMSAMPLES_H

#include <QDialog>
#include "solim-lib-forqt.h"

namespace Ui {
class soilInferenceFromSamples;
}

class soilInferenceFromSamples : public QDialog
{
    Q_OBJECT

public:
    explicit soilInferenceFromSamples(QWidget *parent = nullptr);
    ~soilInferenceFromSamples();

private slots:
    void on_SampleFileRead_btn_clicked();

    void on_CovariateFileRead_btn_clicked();

    void on_CovariateFileDelete_btn_clicked();

    void on_buttonBox_accepted();

    void on_InferFromSmaples_OK_btn_clicked();

    void on_SoilFileCreate_btn_clicked();

    void on_UncerFileCreate_btn_clicked();

private:
    Ui::soilInferenceFromSamples *ui;
};

#endif // SOILINFERENCEFROMSAMPLES_H
