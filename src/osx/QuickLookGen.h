// Apart of the BORA Source which uses the TAOSU License
// Check LICENSE.md for more information regarding the BORA license.

/* ? [Common or PDS (Platform Dependent Source)]

 * FileName: QuickLookGen.h
 * Title: ?
 * Author: ?
 * Purpose: ?

 * Compatibility: ?

 * Updates - ?
 * Known issues - ?
 */

#import <QuickLook/QuickLook.h>

OSStatus GenerateThumbnailForURL(
    void* thisInterface,
    QLThumbnailRequestRef thumbnail,
    CFURLRef url,
    CFStringRef contentTypeUTI,
    CFDictionaryRef options,
    CGSize maxSize)
{
    // Load your .bapp file, extract logo, render it
    // maxSize tells QuickLook what size it wants
    // Use CoreGraphics to create a CGImageRef
    // Example: placeholder
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(NULL, maxSize.width, maxSize.height, 8, 4*maxSize.width, colorSpace, kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);

    // fill with a placeholder color
    CGContextSetRGBFillColor(ctx, 0.1, 0.6, 0.8, 1.0);
    CGContextFillRect(ctx, CGRectMake(0, 0, maxSize.width, maxSize.height));

    CGImageRef img = CGBitmapContextCreateImage(ctx);
    QLThumbnailRequestSetImage(thumbnail, img, NULL);
    CGImageRelease(img);
    CGContextRelease(ctx);

    return noErr;
}

void CancelThumbnailGeneration(void* thisInterface, QLThumbnailRequestRef thumbnail)
{
    // Handle cancellation
}
