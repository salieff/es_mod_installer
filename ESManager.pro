TEMPLATE = app

QT += qml quick widgets

!android {
QT += webengine
}

SOURCES += main.cpp \
    esmodelement.cpp \
    esmodmodel.cpp \
    minizip/ioapi.c \
    minizip/unzip.c \
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
    asyncfilewriter.cpp

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
    asyncfilewriter.h

DEFINES += _LARGEFILE64_SOURCE=1 HAVE_HIDDEN _FILE_OFFSET_BITS=64 IOAPI_NO_64

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
