QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14 \
    sdk_no_version_check

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dcmdataset.cpp \
    dcmdatasetthread.cpp \
    dcmio.cpp \
    dcmlistthread.cpp \
    imageviewer1.cpp \
    main.cpp \
    mainwindow.cpp \
    studybrowser.cpp \
    tableoperate.cpp

HEADERS += \
    dcmdataset.h \
    dcmdatasetthread.h \
    dcmio.h \
    dcmlayer.h \
    dcmlistthread.h \
    globalstate.h \
    imageviewer1.h \
    mainwindow.h \
    studybrowser.h \
    tableoperate.h

FORMS += \
    imageviewer1.ui \
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

INCLUDEPATH += /usr/local/Cellar/opencv@3/3.4.14_3/include\

LIBS += -L/usr/local/Cellar/opencv@3/3.4.14_3/lib\
    -lopencv_aruco.3.4.14\
    -lopencv_flann.3.4.14\
    -lopencv_phase_unwrapping.3.4.14\
    -lopencv_surface_matching.3.4.14\
    -lopencv_bgsegm.3.4.14\
    -lopencv_freetype.3.4.14\
    -lopencv_photo.3.4.14\
    -lopencv_tracking.3.4.14\
    -lopencv_bioinspired.3.4.14\
    -lopencv_fuzzy.3.4.14\
    -lopencv_plot.3.4.14\
    -lopencv_video.3.4.14\
    -lopencv_calib3d.3.4.14\
    -lopencv_hfs.3.4.14\
    -lopencv_reg.3.4.14\
    -lopencv_videoio.3.4.14\
    -lopencv_ccalib.3.4.14\
    -lopencv_highgui.3.4.14\
    -lopencv_rgbd.3.4.14\
    -lopencv_videostab.3.4.14\
    -lopencv_core.3.4.14\
    -lopencv_img_hash.3.4.14\
    -lopencv_saliency.3.4.14\
    -lopencv_xfeatures2d.3.4.14\
    -lopencv_datasets.3.4.14\
    -lopencv_imgcodecs.3.4.14\
    -lopencv_sfm.3.4.14\
    -lopencv_ximgproc.3.4.14\
    -lopencv_dnn.3.4.14\
    -lopencv_imgproc.3.4.14\
    -lopencv_shape.3.4.14\
    -lopencv_xobjdetect.3.4.14\
    -lopencv_dnn_objdetect.3.4.14\
    -lopencv_line_descriptor.3.4.14\
    -lopencv_stereo.3.4.14\
    -lopencv_xphoto.3.4.14\
    -lopencv_dpm.3.4.14\
    -lopencv_ml.3.4.14\
    -lopencv_stitching.3.4.14\
    -lopencv_face.3.4.14\
    -lopencv_objdetect.3.4.14\
    -lopencv_structured_light.3.4.14\
    -lopencv_features2d.3.4.14\
    -lopencv_optflow.3.4.14\
    -lopencv_superres.3.4.14\

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Note
