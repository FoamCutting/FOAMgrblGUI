#-------------------------------------------------
#
# Project created by QtCreator 2011-05-21T21:58:57
#
#-------------------------------------------------

message($$_PRO_FILE_PWD_)

QT       += core gui

TARGET = GrblCage
TEMPLATE = app

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

LIBS += -L/home/christian/personal/Projects/CNC/GrblCageProject/qserialdevice/src/build/release/ -lqserialdevice
#LIBS +=	-L $$PWD/../qserialdevice/src-build-desktop/build/debug/ -lqserialdevice


INCLUDEPATH += $$PWD/../qserialdevice/src/qserialdevice
INCLUDEPATH += $$PWD/../qserialdevice/src/qserialdeviceenumerator

DEPENDPATH += $$PWD/../qserialdevice/src/qserialdevice
DEPENDPATH +=  $$PWD/../qserialdevice/src/qserialdeviceenumerator

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../qserialdevice/src/build/release -lqserialdevice
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../qserialdevice/src/build/debug -lqserialdevice
else:unix: LIBS += -L$$PWD/../qserialdevice/src/build/release/ -lqserialdevice

INCLUDEPATH += $$PWD/../qserialdevice/src/qserialdevice
DEPENDPATH += $$PWD/../qserialdevice/src/qserialdevice
