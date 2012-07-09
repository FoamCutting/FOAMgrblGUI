#-------------------------------------------------
#
# Project created by QtCreator 2011-05-21T21:58:57
#
#-------------------------------------------------


QT       += core gui
CONFIG += serialport

TARGET = FoamGrbl
TEMPLATE = app

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../FoamGrbl-Debug
    MOC_DIR = $$PWD/../FoamGrbl-Debug/tmp
    OBJECTS_DIR = $$PWD/../FoamGrbl-Debug/tmp
    UI_DIR = $$PWD/../FoamGrbl-Debug/tmp
} else {
    DESTDIR = $$PWD/../FoamGrbl-Release
    MOC_DIR = $$PWD/../FoamGrbl-Release/tmp
    OBJECTS_DIR = $$PWD/../FoamGrbl-Release/tmp
    UI_DIR = $$PWD/../FoamGrbl-Release/tmp
}

SOURCES += main.cpp\
    gcodeplot.cpp \
    gcodetext.cpp \
    errorhandler.cpp \
    offsetdialog.cpp \
    scaledialog.cpp \
    grblcage.cpp \
    eventfilterizer.cpp \
    settings.cpp \
    arduinoio.cpp \
    Toolbox.cpp \
    machineselect.cpp \
    newmachinedialog.cpp

HEADERS  += \
    gcodeplot.h \
    gcodetext.h \
    errorhandler.h \
    offsetdialog.h \
    scaledialog.h \
    scaledialog.h \
    grblcage.h \
    eventfilterizer.h \
    settings.h \
    arduinoio.h \
    Toolbox.h \
    machineselect.h \
    newmachinedialog.h

FORMS    += grblcage.ui \
    errorhandler.ui \
    offsetdialog.ui \
    scaledialog.ui \
    settings.ui

OTHER_FILES +=

RESOURCES +=


#QMAKE_POST_LINK = ../FoamGrbl-Release/FoamGrbl
