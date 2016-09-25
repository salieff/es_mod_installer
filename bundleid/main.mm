#import <Foundation/Foundation.h>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
	std::cerr << "Usage: " << argv[0] << " <plist> [key = CFBundleIdentifier]" << std::endl;
	return 1;
    }

    @autoreleasepool
    {
	NSString *plistPath = [NSString stringWithUTF8String: argv[1]];

	NSString *plistKey = nil;
	if (argc >= 3)
	    plistKey = [NSString stringWithUTF8String: argv[2]];
	else
	    plistKey = @"CFBundleIdentifier";

	NSDictionary *plistDict = [NSDictionary dictionaryWithContentsOfFile: plistPath];

	NSString *bundleId = [plistDict objectForKey: plistKey];
	if (bundleId == nil)
	{
	    std::cerr << argv[1] << " doesn't contain " << [plistKey UTF8String] << std::endl;
	    return 2;
	}

	std::cout << [bundleId UTF8String] << std::endl;
    }

    return 0;
}
