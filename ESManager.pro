TEMPLATE = app

QT += qml quick widgets network webview quickcontrols2

android {
    QT += androidextras
}

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
    asyncfilewriter.cpp \
    statisticsmanager.cpp \
    esinstalledmodmodel.cpp \
    safadapter.cpp \
    safioapi.cpp

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
    asyncfilewriter.h \
    ios_helpers.h \
    statisticsmanager.h \
    esinstalledmodmodel.h \
    version.h \
    modpaths.h \
    safadapter.h

DEFINES += _LARGEFILE64_SOURCE=1 HAVE_HIDDEN _FILE_OFFSET_BITS=64 USE_FILE32API

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/assets/help/index.html \
    android/gradle.properties

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

ANDROID_TARGET_SDK_VERSION = 30

ios {
    QMAKE_INFO_PLIST = $$PWD/ESInfo.plist
    ios_icon.files = $$PWD/ios/icon.png $$files($$PWD/ios/AppIcon*.png)
    QMAKE_BUNDLE_DATA += ios_icon
    app_launch_images.files = $$PWD/ios/ESLaunch.xib $$files($$PWD/ios/LaunchImage*.png)
    QMAKE_BUNDLE_DATA += app_launch_images
    app_help_topic.files = $$files($$PWD/ios/help/*)
    QMAKE_BUNDLE_DATA += app_help_topic

    OBJECTIVE_HEADERS += ios_helpers.h
    OBJECTIVE_SOURCES += ios_helpers.mm

    LIBS += -framework Foundation -framework CoreFoundation -framework UIKit
}

FORMS += \
    safaccessdialog.ui
