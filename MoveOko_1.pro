#-------------------------------------------------
#
# Project created by QtCreator 2017-11-16T22:07:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MoveOko1
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_objdetect -lopencv_videoio -lopencv_video -lopencv_xfeatures2d -lopencv_features2d -lopencv_flann -lopencv_calib3d

SOURCES += \
        main.cpp \
        moveoko.cpp \
    camera.cpp \
    nqlabel.cpp

HEADERS += \
        moveoko.h \
    camera.h \
    nqlabel.h

FORMS += \
    camera.ui

DISTFILES += \
    PVS-Studio.pri
