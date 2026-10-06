QT += core widgets concurrent testlib
CONFIG += c++17 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = tst_qcardio

INCLUDEPATH += $$PWD/.. $$PWD/../ThirdParty

SOURCES += \
    tst_qcardio.cpp \
    ../Models/aamiclass.cpp \
    ../Models/beattestprofile.cpp \
    ../Models/beatkpi.cpp \
    ../Models/beatmatcher.cpp \
    ../Models/csv.cpp \
    ../Models/ec57metrics.cpp \
    ../Models/performancereport.cpp \
    ../Models/directoryvalidator.cpp \
    ../Models/resultmatrix.cpp \
    ../Models/sheetanalyser.cpp \
    ../Models/uiconfigs.cpp \
    ../Services/analyseservice.cpp \
    ../Services/exportservice.cpp \
    ../Services/logservice.cpp

HEADERS += \
    ../Models/aamiclass.h \
    ../Models/beattestprofile.h \
    ../Models/beatkpi.h \
    ../Models/beatmatcher.h \
    ../Models/csv.h \
    ../Models/ec57metrics.h \
    ../Models/performancereport.h \
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
