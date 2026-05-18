cmake_minimum_required(VERSION 3.16)
project(QComicViewer VERSION 1.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 QUIET COMPONENTS Core Gui Svg Widgets LinguistTools)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Svg Widgets LinguistTools)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

set(SOURCES
    aboutdialog.cpp
    archivereader.cpp
    bookpagesmodel.cpp
    folderarchivereader.cpp
    imagewidget.cpp
    main.cpp
    mainwindow.cpp
    thumbnailview.cpp
    qtrar/qtrar.cpp
    qtrar/qtrarfile.cpp
    qtrar/qtrarfileinfo.cpp
    qtrar/unrar/rar.cpp
    qtrar/unrar/strlist.cpp
    qtrar/unrar/strfn.cpp
    qtrar/unrar/pathfn.cpp
    qtrar/unrar/smallfn.cpp
    qtrar/unrar/global.cpp
    qtrar/unrar/file.cpp
    qtrar/unrar/filefn.cpp
    qtrar/unrar/filcreat.cpp
    qtrar/unrar/archive.cpp
    qtrar/unrar/arcread.cpp
    qtrar/unrar/unicode.cpp
    qtrar/unrar/system.cpp
    qtrar/unrar/isnt.cpp
    qtrar/unrar/crypt.cpp
    qtrar/unrar/crc.cpp
    qtrar/unrar/rawread.cpp
    qtrar/unrar/encname.cpp
    qtrar/unrar/resource.cpp
    qtrar/unrar/match.cpp
    qtrar/unrar/timefn.cpp
    qtrar/unrar/rdwrfn.cpp
    qtrar/unrar/consio.cpp
    qtrar/unrar/options.cpp
    qtrar/unrar/errhnd.cpp
    qtrar/unrar/rarvm.cpp
    qtrar/unrar/secpassword.cpp
    qtrar/unrar/rijndael.cpp
    qtrar/unrar/getbits.cpp
    qtrar/unrar/sha1.cpp
    qtrar/unrar/sha256.cpp
    qtrar/unrar/blake2s.cpp
    qtrar/unrar/hash.cpp
    qtrar/unrar/extinfo.cpp
    qtrar/unrar/extract.cpp
    qtrar/unrar/volume.cpp
    qtrar/unrar/list.cpp
    qtrar/unrar/find.cpp
    qtrar/unrar/unpack.cpp
    qtrar/unrar/headers.cpp
    qtrar/unrar/threadpool.cpp
    qtrar/unrar/rs16.cpp
    qtrar/unrar/cmddata.cpp
    qtrar/unrar/ui.cpp
    qtrar/unrar/filestr.cpp
    qtrar/unrar/scantree.cpp
    qtrar/unrar/dll.cpp
    qtrar/unrar/qopen.cpp
    quazip/qioapi.cpp
    quazip/quaadler32.cpp
    quazip/quachecksum32.cpp
    quazip/quacrc32.cpp
    quazip/quagzipfile.cpp
    quazip/quaziodevice.cpp
    quazip/quazip.cpp
    quazip/quazipdir.cpp
    quazip/quazipfile.cpp
    quazip/quazipfileinfo.cpp
    quazip/quazipnewinfo.cpp
    quazip/unzip.c
    quazip/zip.c
    quazip/zlib/adler32.c
    quazip/zlib/compress.c
    quazip/zlib/crc32.c
    quazip/zlib/deflate.c
    quazip/zlib/gzclose.c
    quazip/zlib/gzlib.c
    quazip/zlib/gzread.c
    quazip/zlib/gzwrite.c
    quazip/zlib/inflate.c
    quazip/zlib/infback.c
    quazip/zlib/inftrees.c
    quazip/zlib/inffast.c
    quazip/zlib/trees.c
    quazip/zlib/uncompr.c
    quazip/zlib/zutil.c
    settings.cpp
    settingsdialog/appearancesettingswidget.cpp
    settingsdialog/viewsettingswidget.cpp
    settingsdialog/settingsdialog.cpp
    settingsdialog/settingswidget.cpp
    rararchivereader.cpp
    ziparchivereader.cpp
)

set(HEADERS
    aboutdialog.h
    archivereader.h
    bookpagesmodel.h
    folderarchivereader.h
    imagewidget.h
    mainwindow.h
    thumbnailview.h
    qtrar/qtrar.h
    qtrar/qtrar_global.h
    qtrar/qtrarfile.h
    qtrar/qtrarfileinfo.h
    qtrar/unrar/rar.hpp
    quazip/ioapi.h
    quazip/minizip_crypt.h
    quazip/quaadler32.h
    quazip/quachecksum32.h
    quazip/quacrc32.h
    quazip/quagzipfile.h
    quazip/quaziodevice.h
    quazip/quazip.h
    quazip/quazip_global.h
    quazip/quazip_qt_compat.h
    quazip/quazipdir.h
    quazip/quazipfile.h
    quazip/quazipfileinfo.h
    quazip/quazipnewinfo.h
    quazip/unzip.h
    quazip/zip.h
    settings.h
    settingsdialog/appearancesettingswidget.h
    settingsdialog/viewsettingswidget.h
    settingsdialog/settingsdialog.h
    settingsdialog/settingswidget.h
    rararchivereader.h
    ziparchivereader.h
)

set(FORMS
    aboutdialog.ui
    mainwindow.ui
    settingsdialog/appearancesettingswidget.ui
    settingsdialog/viewsettingswidget.ui
    settingsdialog/settingsdialog.ui
)

set(RESOURCES
    icons.qrc
)

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})
else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    RARDLL
    "APP_VERSION=\"${PROJECT_VERSION}\""
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Svg
    Qt::Widgets
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:MinSizeRel>:-s -static>
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE powrprof)

    set(APP_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/comic-book.ico")
    if(EXISTS ${APP_ICON_PATH})
        set(APP_RC "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
        file(WRITE ${APP_RC} "IDI_ICON1 ICON \"${APP_ICON_PATH}\"\n")
        target_sources(${PROJECT_NAME} PRIVATE ${APP_RC})
    endif()
endif()

set(TS_FILES
    QComicViewer_zh_CN.ts
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_translations(${PROJECT_NAME} TS_FILES ${TS_FILES})
else()
    find_package(Qt5LinguistTools REQUIRED)
    set(QM_FILES)
    qt5_add_translation(QM_FILES ${TS_FILES})

    set(QM_QRC "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc")
    file(WRITE ${QM_QRC} "<RCC>\n    <qresource prefix=\"/i18n\">\n")
    foreach(QM_FILE ${QM_FILES})
        get_filename_component(QM_NAME ${QM_FILE} NAME)
        file(APPEND ${QM_QRC} "        <file>${QM_NAME}</file>\n")
    endforeach()
    file(APPEND ${QM_QRC} "    </qresource>\n</RCC>\n")
    target_sources(${PROJECT_NAME} PRIVATE ${QM_QRC})
endif()

if(QNX)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /tmp/${PROJECT_NAME}/bin)
elseif(UNIX AND NOT ANDROID)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /opt/${PROJECT_NAME}/bin)
endif()
