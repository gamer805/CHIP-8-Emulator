#include "file_dialog.h"

#import <Cocoa/Cocoa.h>

std::string openRomFileDialog() {
    @autoreleasepool {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        panel.canChooseFiles = YES;
        panel.canChooseDirectories = NO;
        panel.allowsMultipleSelection = NO;
        panel.title = @"Choose a CHIP-8 ROM";
        panel.prompt = @"Load";

        if ([panel runModal] == NSModalResponseOK) {
            return std::string([panel.URL.path UTF8String]);
        }
    }
    return {};
}
