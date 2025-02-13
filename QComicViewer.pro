QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

APP_VERSION = 0.9

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += RARDLL
DEFINES += APP_VERSION=$${APP_VERSION}
#DEFINES += _FILE_OFFSET_BITS=64
#DEFINES += _LARGEFILE_SOURCE

win32 {
    LIBS += -lpowrprof
}

SOURCES += \
    aboutdialog.cpp \
    archivereader.cpp \
    folderarchivereader.cpp \
    imagewidget.cpp \
    main.cpp \
    mainwindow.cpp \
    pagesnavigator.cpp \
    qtrar\qtrar.cpp \
    qtrar\qtrarfile.cpp \
    qtrar\qtrarfileinfo.cpp \
    qtrar\unrar\rar.cpp \
    qtrar\unrar\strlist.cpp \
    qtrar\unrar\strfn.cpp \
    qtrar\unrar\pathfn.cpp \
    qtrar\unrar\smallfn.cpp \
    qtrar\unrar\global.cpp \
    qtrar\unrar\file.cpp \
    qtrar\unrar\filefn.cpp \
    qtrar\unrar\filcreat.cpp \
    qtrar\unrar\archive.cpp \
    qtrar\unrar\arcread.cpp \
    qtrar\unrar\unicode.cpp \
    qtrar\unrar\system.cpp \
    qtrar\unrar\isnt.cpp \
    qtrar\unrar\crypt.cpp \
    qtrar\unrar\crc.cpp \
    qtrar\unrar\rawread.cpp \
    qtrar\unrar\encname.cpp \
    qtrar\unrar\resource.cpp \
    qtrar\unrar\match.cpp \
    qtrar\unrar\timefn.cpp \
    qtrar\unrar\rdwrfn.cpp \
    qtrar\unrar\consio.cpp \
    qtrar\unrar\options.cpp \
    qtrar\unrar\errhnd.cpp \
    qtrar\unrar\rarvm.cpp \
    qtrar\unrar\secpassword.cpp \
    qtrar\unrar\rijndael.cpp \
    qtrar\unrar\getbits.cpp \
    qtrar\unrar\sha1.cpp \
    qtrar\unrar\sha256.cpp \
    qtrar\unrar\blake2s.cpp \
    qtrar\unrar\hash.cpp \
    qtrar\unrar\extinfo.cpp \
    qtrar\unrar\extract.cpp \
    qtrar\unrar\volume.cpp \
    qtrar\unrar\list.cpp \
    qtrar\unrar\find.cpp \
    qtrar\unrar\unpack.cpp \
    qtrar\unrar\headers.cpp \
    qtrar\unrar\threadpool.cpp \
    qtrar\unrar\rs16.cpp \
    qtrar\unrar\cmddata.cpp \
    qtrar\unrar\ui.cpp \
    qtrar\unrar\filestr.cpp \
    qtrar\unrar\scantree.cpp \
    qtrar\unrar\dll.cpp \
    qtrar\unrar\qopen.cpp \
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
    quazip\zlib\zutil.c \
    settings.cpp \
    settingsdialog/appearancesettingswidget.cpp \
    settingsdialog/viewsettingswidget.cpp \
    settingsdialog\settingsdialog.cpp \
    settingsdialog\settingswidget.cpp \
    rararchivereader.cpp \
    ziparchivereader.cpp

HEADERS += \
    aboutdialog.h \
    archivereader.h \
    folderarchivereader.h \
    imagewidget.h \
    mainwindow.h \
    pagesnavigator.h \
    qtrar\qtrar.h \
    qtrar\qtrar_global.h \
    qtrar\qtrarfile.h \
    qtrar\qtrarfileinfo.h \
    qtrar\unrar\rar.hpp \
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
    quazip\zip.h \
    settings.h \
    settingsdialog/appearancesettingswidget.h \
    settingsdialog/viewsettingswidget.h \
    settingsdialog\settingsdialog.h \
    settingsdialog\settingswidget.h \
    rararchivereader.h \
    ziparchivereader.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    settingsdialog/appearancesettingswidget.ui \
    settingsdialog/viewsettingswidget.ui \
    settingsdialog\settingsdialog.ui

TRANSLATIONS += \
    QComicViewer_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

RC_ICONS = comic-book.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
