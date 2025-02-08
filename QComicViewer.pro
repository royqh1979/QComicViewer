QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    imagewidget.cpp \
    main.cpp \
    mainwindow.cpp \
    pagesnavigator.cpp \
    quazip\qioapi.cpp \
    quazip\quaadler32.cpp \
    quazip\quachecksum32.cpp \
    quazip\quacrc32.cpp \
    quazip\quagzipfile.cpp \
    quazip\quaziodevice.cpp \
    quazip\quazip.cpp \
    quazip\quazipdir.cpp \
    quazip\quazipfile.cpp \
    quazip\quazipfileinfo.cpp \
    quazip\quazipnewinfo.cpp \
    quazip\unzip.c \
    quazip\zip.c \
    quazip\zlib\adler32.c \
    quazip\zlib\compress.c \
    quazip\zlib\crc32.c \
    quazip\zlib\deflate.c \
    quazip\zlib\gzclose.c \
    quazip\zlib\gzlib.c \
    quazip\zlib\gzread.c \
    quazip\zlib\gzwrite.c \
    quazip\zlib\inflate.c \
    quazip\zlib\infback.c \
    quazip\zlib\inftrees.c \
    quazip\zlib\inffast.c \
    quazip\zlib\trees.c \
    quazip\zlib\uncompr.c \
    quazip\zlib\zutil.c

HEADERS += \
    imagewidget.h \
    mainwindow.h \
    pagesnavigator.h \
    quazip\ioapi.h \
    quazip\minizip_crypt.h \
    quazip\quaadler32.h \
    quazip\quachecksum32.h \
    quazip\quacrc32.h \
    quazip\quagzipfile.h \
    quazip\quaziodevice.h \
    quazip\quazip.h \
    quazip\quazip_global.h \
    quazip\quazip_qt_compat.h \
    quazip\quazipdir.h \
    quazip\quazipfile.h \
    quazip\quazipfileinfo.h \
    quazip\quazipnewinfo.h \
    quazip\unzip.h \
    quazip\zip.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    QComicViewer_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
