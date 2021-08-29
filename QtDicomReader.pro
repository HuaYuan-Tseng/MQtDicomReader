QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14
CONFIG += sdk_no_version_check
CONFIG += debug
CONFIG += release

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cvtest.cpp \
    dcmdataset.cpp \
    dcmdatasetthread.cpp \
    dcmio.cpp \
    dcmlistthread.cpp \
    imageviewer1.cpp \
    main.cpp \
    mainwindow.cpp \
    nodule.cpp \
    roi.cpp \
    studybrowser.cpp \
    tableoperate.cpp \
    viewer.cpp \
    viewerinteractor.cpp \
    viewertext.cpp

HEADERS += \
    cvtest.h \
    dcmdataset.h \
    dcmdatasetthread.h \
    dcmio.h \
    dcmlayer.h \
    dcmlistthread.h \
    globalstate.h \
    imageviewer1.h \
    mainwindow.h \
    nodule.h \
    roi.h \
    studybrowser.h \
    tableoperate.h \
    viewer.h \
    viewerinteractor.h \
    viewertext.h

FORMS += \
    imageviewer1.ui \
    mainwindow.ui \
    studybrowser.ui

INCLUDEPATH += /usr/local/Cellar/dcmtk/3.6.6/include/

LIBS += -L/usr/local/Cellar/dcmtk/3.6.6/lib \
        -lcmr.16.3.6.6 \
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

INCLUDEPATH += /usr/local/Cellar/opencv@3/3.4.14_3/include/

LIBS += -L/usr/local/Cellar/opencv@3/3.4.14_3/lib \
    -lopencv_aruco.3.4.14 \
    -lopencv_flann.3.4.14 \
    -lopencv_phase_unwrapping.3.4.14 \
    -lopencv_surface_matching.3.4.14 \
    -lopencv_bgsegm.3.4.14 \
    -lopencv_freetype.3.4.14 \
    -lopencv_photo.3.4.14 \
    -lopencv_tracking.3.4.14 \
    -lopencv_bioinspired.3.4.14 \
    -lopencv_fuzzy.3.4.14 \
    -lopencv_plot.3.4.14 \
    -lopencv_video.3.4.14 \
    -lopencv_calib3d.3.4.14 \
    -lopencv_hfs.3.4.14 \
    -lopencv_reg.3.4.14 \
    -lopencv_videoio.3.4.14 \
    -lopencv_ccalib.3.4.14 \
    -lopencv_highgui.3.4.14 \
    -lopencv_rgbd.3.4.14 \
    -lopencv_videostab.3.4.14 \
    -lopencv_core.3.4.14 \
    -lopencv_img_hash.3.4.14 \
    -lopencv_saliency.3.4.14 \
    -lopencv_xfeatures2d.3.4.14 \
    -lopencv_datasets.3.4.14 \
    -lopencv_imgcodecs.3.4.14 \
    -lopencv_sfm.3.4.14 \
    -lopencv_ximgproc.3.4.14 \
    -lopencv_dnn.3.4.14 \
    -lopencv_imgproc.3.4.14 \
    -lopencv_shape.3.4.14 \
    -lopencv_xobjdetect.3.4.14 \
    -lopencv_dnn_objdetect.3.4.14 \
    -lopencv_line_descriptor.3.4.14 \
    -lopencv_stereo.3.4.14 \
    -lopencv_xphoto.3.4.14 \
    -lopencv_dpm.3.4.14 \
    -lopencv_ml.3.4.14 \
    -lopencv_stitching.3.4.14 \
    -lopencv_face.3.4.14 \
    -lopencv_objdetect.3.4.14 \
    -lopencv_structured_light.3.4.14 \
    -lopencv_features2d.3.4.14 \
    -lopencv_optflow.3.4.14 \
    -lopencv_superres.3.4.14 \

INCLUDEPATH += /usr/local/Cellar/vtk@8.2/8.2.0_7/include/vtk-8.2/

LIBS += -L/usr/local/Cellar/vtk@8.2/8.2.0_7/lib \
    -lvtkChartsCore-8.2.1 \
    -lvtkCommonColor-8.2.1 \
    -lvtkCommonComputationalGeometry-8.2.1 \
    -lvtkCommonCore-8.2.1 \
    -lvtkCommonDataModel-8.2.1 \
    -lvtkCommonExecutionModel-8.2.1 \
    -lvtkCommonMath-8.2.1 \
    -lvtkCommonMisc-8.2.1 \
    -lvtkCommonSystem-8.2.1 \
    -lvtkCommonTransforms-8.2.1 \
    -lvtkDICOMParser-8.2.1 \
    -lvtkDomainsChemistry-8.2.1 \
    -lvtkDomainsChemistryOpenGL2-8.2.1 \
    -lvtkFiltersAMR-8.2.1 \
    -lvtkFiltersCore-8.2.1 \
    -lvtkFiltersExtraction-8.2.1 \
    -lvtkFiltersFlowPaths-8.2.1 \
    -lvtkFiltersGeneral-8.2.1 \
    -lvtkFiltersGeneric-8.2.1 \
    -lvtkFiltersGeometry-8.2.1 \
    -lvtkFiltersHybrid-8.2.1 \
    -lvtkFiltersHyperTree-8.2.1 \
    -lvtkFiltersImaging-8.2.1 \
    -lvtkFiltersModeling-8.2.1 \
    -lvtkFiltersParallel-8.2.1 \
    -lvtkFiltersParallelImaging-8.2.1 \
    -lvtkFiltersPoints-8.2.1 \
    -lvtkFiltersProgrammable-8.2.1 \
    -lvtkFiltersSMP-8.2.1 \
    -lvtkFiltersSelection-8.2.1 \
    -lvtkFiltersSources-8.2.1 \
    -lvtkFiltersStatistics-8.2.1 \
    -lvtkFiltersTexture-8.2.1 \
    -lvtkFiltersTopology-8.2.1 \
    -lvtkFiltersVerdict-8.2.1 \
    -lvtkGUISupportQt-8.2.1 \
    -lvtkGUISupportQtSQL-8.2.1 \
    -lvtkGeovisCore-8.2.1 \
    -lvtkIOAMR-8.2.1 \
    -lvtkIOAsynchronous-8.2.1 \
    -lvtkIOCityGML-8.2.1 \
    -lvtkIOCore-8.2.1 \
    -lvtkIOEnSight-8.2.1 \
    -lvtkIOExodus-8.2.1 \
    -lvtkIOExport-8.2.1 \
    -lvtkIOExportOpenGL2-8.2.1 \
    -lvtkIOExportPDF-8.2.1 \
    -lvtkIOGeometry-8.2.1 \
    -lvtkIOImage-8.2.1 \
    -lvtkIOImport-8.2.1 \
    -lvtkIOInfovis-8.2.1 \
    -lvtkIOLSDyna-8.2.1 \
    -lvtkIOLegacy-8.2.1 \
    -lvtkIOMINC-8.2.1 \
    -lvtkIOMovie-8.2.1 \
    -lvtkIONetCDF-8.2.1 \
    -lvtkIOPLY-8.2.1 \
    -lvtkIOParallel-8.2.1 \
    -lvtkIOParallelXML-8.2.1 \
    -lvtkIOSQL-8.2.1 \
    -lvtkIOSegY-8.2.1 \
    -lvtkIOTecplotTable-8.2.1 \
    -lvtkIOVeraOut-8.2.1 \
    -lvtkIOVideo-8.2.1 \
    -lvtkIOXML-8.2.1 \
    -lvtkIOXMLParser-8.2.1 \
    -lvtkImagingColor-8.2.1 \
    -lvtkImagingCore-8.2.1 \
    -lvtkImagingFourier-8.2.1 \
    -lvtkImagingGeneral-8.2.1 \
    -lvtkImagingHybrid-8.2.1 \
    -lvtkImagingMath-8.2.1 \
    -lvtkImagingMorphological-8.2.1 \
    -lvtkImagingSources-8.2.1 \
    -lvtkImagingStatistics-8.2.1 \
    -lvtkImagingStencil-8.2.1 \
    -lvtkInfovisBoostGraphAlgorithms-8.2.1 \
    -lvtkInfovisCore-8.2.1 \
    -lvtkInfovisLayout-8.2.1 \
    -lvtkInteractionImage-8.2.1 \
    -lvtkInteractionStyle-8.2.1 \
    -lvtkInteractionWidgets-8.2.1 \
    -lvtkParallelCore-8.2.1 \
    -lvtkRenderingAnnotation-8.2.1 \
    -lvtkRenderingContext2D-8.2.1 \
    -lvtkRenderingContextOpenGL2-8.2.1 \
    -lvtkRenderingCore-8.2.1 \
    -lvtkRenderingFreeType-8.2.1 \
    -lvtkRenderingFreeTypeFontConfig-8.2.1 \
    -lvtkRenderingGL2PSOpenGL2-8.2.1 \
    -lvtkRenderingImage-8.2.1 \
    -lvtkRenderingLOD-8.2.1 \
    -lvtkRenderingLabel-8.2.1 \
    -lvtkRenderingOpenGL2-8.2.1 \
    -lvtkRenderingQt-8.2.1 \
    -lvtkRenderingVolume-8.2.1 \
    -lvtkRenderingVolumeOpenGL2-8.2.1 \
    -lvtkViewsContext2D-8.2.1 \
    -lvtkViewsCore-8.2.1 \
    -lvtkViewsInfovis-8.2.1 \
    -lvtkViewsQt-8.2.1 \
    -lvtkdoubleconversion-8.2.1 \
    -lvtkexodusII-8.2.1 \
    -lvtkfreetype-8.2.1 \
    -lvtkgl2ps-8.2.1 \
    -lvtkglew-8.2.1 \
    -lvtkjsoncpp-8.2.1 \
    -lvtklibharu-8.2.1 \
    -lvtklz4-8.2.1 \
    -lvtklzma-8.2.1 \
    -lvtkmetaio-8.2.1 \
    -lvtkogg-8.2.1 \
    -lvtkproj-8.2.1 \
    -lvtkpugixml-8.2.1 \
    -lvtksqlite-8.2.1 \
    -lvtksys-8.2.1 \
    -lvtktheora-8.2.1 \
    -lvtkverdict-8.2.1 \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Note
