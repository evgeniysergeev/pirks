/*
 * This file is based on src/platform/macos/av_audio.h from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#pragma once

#import <AVFoundation/AVFoundation.h>

#include "TPCircularBuffer.h"

@interface CaptureDevice: NSObject <AVCaptureAudioDataOutputSampleBufferDelegate>
{
@public
    TPCircularBuffer audioSampleBuffer;
}

// Owned by CaptureDevice under MRC.
@property (nonatomic, retain) AVCaptureSession *audioCaptureSession;
@property (nonatomic, retain) NSCondition *samplesArrivedSignal;

// Borrowed from AVCaptureAudioDataOutput/AVCaptureSession; CaptureDevice does not own it.
@property (nonatomic, assign) AVCaptureConnection *audioConnection;

+ (NSArray<NSString *> *)captureDeviceNames;
+ (AVCaptureDevice *)findCaptureDevice:(NSString *)name;

- (int)setupCaptureDevice:(AVCaptureDevice *)device
               sampleRate:(UInt32)sampleRate
                frameSize:(UInt32)frameSize
                 channels:(UInt8)channels;

@end
