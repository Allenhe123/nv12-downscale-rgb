TEMPLATE = app
CONFIG += console c++11
#CONFIG -= app_bundle
#CONFIG -= qt

LIBS += -lpthread -lglog -lgflags

INCLUDEPATH += /usr/local/include/opencv2


INCLUDEPATH += /home/allen/libyuv/include/
INCLUDEPATH += /home/allen/libyuv/include/libyuv

LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann
#LIBS += -lopencv_nonfree

SOURCES += \
        main.cpp \
        /home/allen/libyuv/source/* \
    nv12_handle.cpp


#HEADERS +=

HEADERS += \
    nv12_handle.h
