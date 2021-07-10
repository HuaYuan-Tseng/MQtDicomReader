QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14 \
    sdk_no_version_check

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dcmio.cpp \
    main.cpp \
    mainwindow.cpp \
    studybrowser.cpp \
    tableoperate.cpp

HEADERS += \
    dcmio.h \
    dcmlayer.h \
    globalstate.h \
    mainwindow.h \
    studybrowser.h \
    tableoperate.h

FORMS += \
    mainwindow.ui \
    studybrowser.ui

INCLUDEPATH += /usr/local/Cellar/dcmtk/3.6.6/include/\

LIBS += -L/usr/local/Cellar/dcmtk/3.6.6/lib\
        -lcmr.16.3.6.6\
        -ldcmjpls.16.3.6.6 \
        -ldcmtls.16.3.6.6 \
        -ldcmdata.16.3.6.6 \
        -ldcmnet.16.3.6.6 \
        -ldcmtract.16.3.6.6 \
        -ldcmdsig.16.3.6.6 \
        -ldcmpmap.16.3.6.6 \
        -ldcmwlm.16.3.6.6 \
        -ldcmect.16.3.6.6 \
        -ldcmpstat.16.3.6.6 \
        -li2d.16.3.6.6 \
        -ldcmfg.16.3.6.6 \
        -ldcmqrdb.16.3.6.6 \
        -lijg12.16.3.6.6 \
        -ldcmimage.16.3.6.6 \
        -ldcmrt.16.3.6.6 \
        -lijg16.16.3.6.6 \
        -ldcmimgle.16.3.6.6 \
        -ldcmseg.16.3.6.6 \
        -lijg8.16.3.6.6 \
        -ldcmiod.16.3.6.6 \
        -ldcmsr.16.3.6.6 \
        -loflog.16.3.6.6 \
        -ldcmjpeg.16.3.6.6 \
        -ldcmtkcharls.16.3.6.6 \
        -lofstd.16.3.6.6 \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
