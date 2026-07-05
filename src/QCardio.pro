QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
SOURCES += \
    Controller/cexporter.cpp \
    Controller/csettings.cpp \
    Controller/csignalview.cpp \
    Controller/cwfdb.cpp \
    Models/annotationreader.cpp \
    Models/directoryvalidator.cpp \
    Models/uiconfigs.cpp \
    Views/signalviewwidget.cpp \
    controller.cpp \
    main.cpp \
    Views/mainwindow.cpp \
    wfdb/annot.c \
    wfdb/calib.c \
    wfdb/signal.c \
    wfdb/wfdbinit.c \
    wfdb/wfdbio.c

HEADERS += \
    Config/define.h \
    Config/structer.h \
    Controller/cexporter.h \
    Controller/csettings.h \
    Controller/csignalview.h \
    Controller/cwfdb.h \
    Models/annotationreader.h \
    Models/directoryvalidator.h \
    Models/sample.h \
    Models/uiconfigs.h \
    Views/signalviewwidget.h \
    controller.h \
    Models/global_qcardio.h \
    Views/mainwindow.h \
    wfdb/ecgcodes.h \
    wfdb/ecgmap.h \
    wfdb/wfdb.h \
    wfdb/wfdblib.h

FORMS += \
    Views/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += C:/curl-8.20.0_5/include
LIBS += -LC:/curl-8.20.0_5/lib -lcurl


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resource.qrc
