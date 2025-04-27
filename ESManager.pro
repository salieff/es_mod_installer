ESM_VERSION_MAJOR = 1
ESM_VERSION_MINOR = 13
ESM_VERSION_BUILD = 1
ESM_VERSION_CODE = 29

TEMPLATE = app

QT += core core-private qml quick widgets network webview quickcontrols2

SOURCES += main.cpp \
    esmodelement.cpp \
    esmodmodel.cpp \
    minizip/ioapi.c \
    minizip/unzip.c \
    safaccessdialog.cpp \
    zlib/adler32.c \
    zlib/compress.c \
    zlib/crc32.c \
    zlib/deflate.c \
    zlib/gzclose.c \
    zlib/gzlib.c \
    zlib/gzread.c \
    zlib/gzwrite.c \
    zlib/infback.c \
    zlib/inffast.c \
    zlib/inflate.c \
    zlib/inftrees.c \
    zlib/trees.c \
    zlib/uncompr.c \
    zlib/zutil.c \
    asyncunzipper.cpp \
    asyncdownloader.cpp \
    asyncdeleter.cpp \
    asyncjsonwriter.cpp \
    statisticsmanager.cpp \
    esinstalledmodmodel.cpp \
    safadapter.cpp \
    safioapi.cpp \
    admcontroller.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    esmodelement.h \
    esmodmodel.h \
    minizip/crypt.h \
    minizip/ioapi.h \
    minizip/unzip.h \
    safaccessdialog.h \
    zlib/crc32.h \
    zlib/deflate.h \
    zlib/gzguts.h \
    zlib/inffast.h \
    zlib/inffixed.h \
    zlib/inflate.h \
    zlib/inftrees.h \
    zlib/trees.h \
    zlib/zconf.h \
    zlib/zlib.h \
    zlib/zutil.h \
    asyncunzipper.h \
    asyncdownloader.h \
    asyncdeleter.h \
    asyncjsonwriter.h \
    statisticsmanager.h \
    esinstalledmodmodel.h \
    safadapter.h \
    admcontroller.h

DEFINES += ESM_VERSION_MAJOR=$${ESM_VERSION_MAJOR} \
           ESM_VERSION_MINOR=$${ESM_VERSION_MINOR} \
           ESM_VERSION_BUILD=$${ESM_VERSION_BUILD} \
           ESM_VERSION_CODE=$${ESM_VERSION_CODE}

DEFINES += _LARGEFILE64_SOURCE=1 HAVE_HIDDEN _FILE_OFFSET_BITS=64 USE_FILE32API

DISTFILES += \
    android/AndroidManifest.xml \
    android/src/org/salieff/SafAdapter.java \
    android/src/org/salieff/ADMController.java \
    android/logo.xml \
    android/res/values/colors.xml \
    android/res/mipmap-nodpi-v26/icon.xml \
    android/res/mipmap-nodpi-v26/icon_round.xml \
    android/res/mipmap-nodpi/icon_foreground.png

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

ANDROID_PERMISSIONS += android.permission.READ_EXTERNAL_STORAGE android.permission.WRITE_EXTERNAL_STORAGE

ANDROID_API_VERSION = 35
ANDROID_TARGET_SDK_VERSION = 34

ANDROID_VERSION_CODE = $$ESM_VERSION_CODE
ANDROID_VERSION_NAME = $${ESM_VERSION_MAJOR}.$${ESM_VERSION_MINOR}

FORMS += \
    safaccessdialog.ui
