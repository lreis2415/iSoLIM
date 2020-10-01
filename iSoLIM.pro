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
    addgisdatadialog.cpp \
    changecovname.cpp \
    inference.cpp \
    main.cpp \
    mainwindow.cpp \
    mygraphicsview.cpp \
    newprojectdialog.cpp \
    prototypefromsamples.cpp \
    solim-lib-forqt.cpp \
    third_party/tinystr.cpp \
    third_party/tinyxml.cpp \
    third_party/tinyxmlerror.cpp \
    third_party/tinyxmlparser.cpp

HEADERS += \
    addgisdatadialog.h \
    changecovname.h \
    inference.h \
    mainwindow.h \
    mygraphicsview.h \
    newprojectdialog.h \
    project.h \
    prototypefromsamples.h \
    solim-lib-forQt_global.h \
    solim-lib-forqt.h \
    third_party/tinystr.h \
    third_party/tinyxml.h

FORMS += \
    addgisdatadialog.ui \
    changecovname.ui \
    inference.ui \
    mainwindow.ui \
    newprojectdialog.ui \
    prototypefromsamples.ui

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
