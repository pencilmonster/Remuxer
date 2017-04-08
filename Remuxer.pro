#-------------------------------------------------
#
# Project created by QtCreator 2017-04-05T19:32:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Remuxer
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    remuxer.cpp

HEADERS  += dialog.h \
    ffmpeg.h \
    remuxer.h

FORMS    += dialog.ui

INCLUDEPATH += D:/ffmpeg-dev/include
QMAKE_LIBDIR+=D:/ffmpeg-dev/lib
    LIBS += avcodec.lib\
            avdevice.lib\
            avfilter.lib\
            avformat.lib\
            avutil.lib\
            swscale.lib\
