QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += $$PWD $$PWD/ThirdParty

SOURCES += \
    Controllers/appcontroller.cpp \
    Controllers/signalviewcontroller.cpp \
    Models/beatkpi.cpp \
    Models/csv.cpp \
    Models/directoryvalidator.cpp \
    Models/resultmatrix.cpp \
    Models/sheetanalyser.cpp \
    Models/uiconfigs.cpp \
    Services/analyseservice.cpp \
    Services/annotationservice.cpp \
    Services/exportservice.cpp \
    Services/logservice.cpp \
    Services/settingsservice.cpp \
    Services/wfdbservice.cpp \
    Views/signalviewwidget.cpp \
    Views/mainwindow.cpp \
    main.cpp \
    ThirdParty/wfdb/annot.c \
    ThirdParty/wfdb/calib.c \
    ThirdParty/wfdb/signal.c \
    ThirdParty/wfdb/wfdbinit.c \
    ThirdParty/wfdb/wfdbio.c

HEADERS += \
    Controllers/appcontroller.h \
    Controllers/signalviewcontroller.h \
    Models/beatkpi.h \
    Models/csv.h \
    Models/define.h \
    Models/directoryvalidator.h \
    Models/global_qcardio.h \
    Models/resultmatrix.h \
    Models/sample.h \
    Models/sheetanalyser.h \
    Models/uiconfigs.h \
    Services/analyseservice.h \
    Services/annotationservice.h \
    Services/exportservice.h \
    Services/logservice.h \
    Services/settingsservice.h \
    Services/wfdbservice.h \
    Views/signalviewwidget.h \
    Views/mainwindow.h \
    ThirdParty/wfdb/ecgcodes.h \
    ThirdParty/wfdb/ecgmap.h \
    ThirdParty/wfdb/wfdb.h \
    ThirdParty/wfdb/wfdblib.h

FORMS += \
    Views/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libcurl
}

win32 {
    INCLUDEPATH += C:/curl-8.20.0_5/include
    LIBS += -LC:/curl-8.20.0_5/lib -lcurl
}

RESOURCES += \
    Resource.qrc

RC_ICONS = Res/image/icon.ico

DISTFILES += \
    Res/help/Physionet Help.pdf
