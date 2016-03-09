#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

#include "ios_helpers.h"

#define SYSTEM_VERSION_EQUAL_TO(v)                  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedSame)
#define SYSTEM_VERSION_GREATER_THAN(v)              ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedDescending)
#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)
#define SYSTEM_VERSION_LESS_THAN(v)                 ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] == NSOrderedAscending)
#define SYSTEM_VERSION_LESS_THAN_OR_EQUAL_TO(v)     ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedDescending)

bool ESIOSHelpers::isPhone()
{
    return ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone);
}

QString ESIOSHelpers::modelName()
{
    return QString::fromNSString([[UIDevice currentDevice] model]);
}

QString ESIOSHelpers::osVersion()
{
    return QString::fromNSString([[UIDevice currentDevice] systemVersion]);
}

float ESIOSHelpers::scaleFactor()
{
    return (float)([[UIScreen mainScreen] scale]);
}

bool ESIOSHelpers::isRetina()
{
    return ([[UIScreen mainScreen] respondsToSelector:@selector(displayLinkWithTarget:selector:)] && ([UIScreen mainScreen].scale == 2.0)) ? true : false;
}

QString ESIOSHelpers::UDID()
{
    if ([[UIDevice currentDevice] respondsToSelector:@selector(identifierForVendor)])
        return QString::fromNSString([[[UIDevice currentDevice] identifierForVendor] UUIDString]);

    return QString::fromNSString([[UIDevice currentDevice] uniqueIdentifier]);
}
