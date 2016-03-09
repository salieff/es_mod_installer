#ifndef ES_MOD_INSTALLER_IOSHELPERS_H
#define ES_MOD_INSTALLER_IOSHELPERS_H

#include <QString>

class ESIOSHelpers {
public:
    static bool isPhone();
    static QString modelName();
    static QString osVersion();
    static float scaleFactor();
    static bool isRetina();
    static QString UDID();
};

#endif // ES_MOD_INSTALLER_IOSHELPERS_H
