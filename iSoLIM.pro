QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addprototype_expert.cpp \
    addprototypebase.cpp \
    changecovname.cpp \
    itemselectionwindow.cpp \
    legendwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    mapinference.cpp \
    mygraphicsview.cpp \
    newprojectdialog.cpp \
    simpledialog.cpp \
    solim_lib/BaseIO.cpp \
    solim_lib/Curve.cpp \
    solim_lib/EnvDataset.cpp \
    solim_lib/EnvLayer.cpp \
    solim_lib/Prototype.cpp \
    solim_lib/inference.cpp \
    solim_lib/io.cpp \
    solim_lib/preprocess.cpp \
    solim_lib/validate.cpp \
    third_party/tinystr.cpp \
    third_party/tinyxml.cpp \
    third_party/tinyxmlerror.cpp \
    third_party/tinyxmlparser.cpp \
    validation.cpp

HEADERS += \
    addprototype_expert.h \
    addprototypebase.h \
    changecovname.h \
    itemselectionwindow.h \
    legendwindow.h \
    mainwindow.h \
    mapinference.h \
    mygraphicsview.h \
    newprojectdialog.h \
    project.h \
    simpledialog.h \
    solim_lib/BaseIO.h \
    solim_lib/BasicSetting.h \
    solim_lib/Curve.h \
    solim_lib/DataTypeEnum.h \
    solim_lib/EnvDataset.h \
    solim_lib/EnvLayer.h \
    solim_lib/EnvUnit.h \
    solim_lib/Location.h \
    solim_lib/Prototype.h \
    solim_lib/inference.h \
    solim_lib/io.h \
    solim_lib/preprocess.h \
    solim_lib/validate.h \
    third_party/tinystr.h \
    third_party/tinyxml.h \
    validation.h

FORMS += \
    addprototype_expert.ui \
    addprototypebase.ui \
    changecovname.ui \
    itemselectionwindow.ui \
    legendwindow.ui \
    mainwindow.ui \
    mapinference.ui \
    newprojectdialog.ui \
    simpledialog.ui \
    validation.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# include GDAL
win32:CONFIG(release, debug|release): LIBS += -LC:/gdal304/lib/ -lgdal_i
else:win32:CONFIG(debug, debug|release): LIBS += -LC:/gdal304/lib/ -lgdal_i
else:unix: LIBS += -LC:/gdal304/lib/ -lgdal_i

INCLUDEPATH += C:/gdal304/include
DEPENDPATH += C:/gdal304/include

# enable openmp
QMAKE_CXXFLAGS+= -openmp

RC_ICONS = solim.ico
