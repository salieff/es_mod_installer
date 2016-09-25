XCODE_BASE=/Applications/Xcode.app/Contents
SIMULATOR_BASE=$(XCODE_BASE)/Developer/Platforms/iPhoneSimulator.platform
SIMULATOR_SYSROOT=$(SIMULATOR_BASE)/Developer/SDKs/iPhoneSimulator.sdk
IOS_BASE=$(XCODE_BASE)/Developer/Platforms/iPhoneOS.platform
IOS_SYSROOT=$(IOS_BASE)/Developer/SDKs/iPhoneOS.sdk

#CXX=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++
#ADD_FLAGS=-x objective-c++ -fmessage-length=190 -fdiagnostics-show-note-include-stack -fmacro-backtrace-limit=0 -fcolor-diagnostics -std=gnu++11 -stdlib=libc++ -fobjc-arc -fpascal-strings -Os -fno-common -DNS_BLOCK_ASSERTIONS=1 -DOBJC_OLD_DISPATCH_PROTOTYPES=0 -fstrict-aliasing -fvisibility=hidden -fvisibility-inlines-hidden -dead_strip -fobjc-link-runtime
#BITCODE_FLAGS=-fembed-bitcode -Xlinker -bitcode_verify -Xlinker -bitcode_hide_symbols

all: bundleid_simulator bundleid
	security unlock-keychain ~/Library/Keychains/login.keychain
	codesign -fs "Nuevo iOS Developer" bundleid_simulator
	codesign -fs "Nuevo iOS Developer" bundleid

clean:
	rm -f bundleid_simulator bundleid

#bundleid_simulator: main.mm
#	$(CXX) $(ADD_FLAGS) -arch i386 -arch x86_64 -mios-simulator-version-min=6.0 -isysroot $(SIMULATOR_SYSROOT) -Wall main.mm -o bundleid_simulator -framework Foundation

#bundleid: main.mm
#	$(CXX) $(ADD_FLAGS) $(BITCODE_FLAGS) -arch armv7 -arch arm64 -miphoneos-version-min=6.0 -isysroot $(IOS_SYSROOT) -Wall main.mm -o bundleid -framework Foundation

bundleid_simulator: main.mm
	clang++ -arch i386 -arch x86_64 -mios-simulator-version-min=6.0 -isysroot $(SIMULATOR_SYSROOT) -Wall -Wextra -O3 main.mm -o bundleid_simulator -framework Foundation

bundleid: main.mm
	clang++ -arch armv7 -arch arm64 -miphoneos-version-min=6.0 -isysroot $(IOS_SYSROOT) -Wall -Wextra -O3 main.mm -o bundleid -framework Foundation
