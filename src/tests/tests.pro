QT += core widgets concurrent testlib
CONFIG += c++17 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_qcardio

INCLUDEPATH += $$PWD/.. $$PWD/../ThirdParty

SOURCES += \
    tst_qcardio.cpp \
    ../Models/beatkpi.cpp \
    ../Models/csv.cpp \
    ../Models/directoryvalidator.cpp \
    ../Models/resultmatrix.cpp \
    ../Models/sheetanalyser.cpp \
    ../Models/uiconfigs.cpp \
    ../Services/analyseservice.cpp \
    ../Services/exportservice.cpp \
    ../Services/logservice.cpp

HEADERS += \
    ../Models/beatkpi.h \
    ../Models/csv.h \
    ../Models/define.h \
    ../Models/directoryvalidator.h \
    ../Models/global_qcardio.h \
    ../Models/resultmatrix.h \
    ../Models/sample.h \
    ../Models/sheetanalyser.h \
    ../Models/uiconfigs.h \
    ../Services/analyseservice.h \
    ../Services/exportservice.h \
    ../Services/logservice.h
