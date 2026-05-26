/*
 * This file is based on src/platform/macos/av_audio.m from Sunshine (GPL-3.0 license)
 * see README.md in the root directory for details
 */

#import "CaptureDevice.h"

#define kBufferLength 4096

// Compiled with -fno-objc-arc; explicit retain/release/dealloc below are intentional.
@implementation CaptureDevice

+ (NSArray<AVCaptureDevice *> *)captureDevices
{
    if ([[NSProcessInfo processInfo] isOperatingSystemAtLeastVersion:((NSOperatingSystemVersion) {10, 15, 0})]) {
        // This will generate a warning about AVCaptureDeviceDiscoverySession being
        // unavailable before macOS 10.15, but we have a guard to prevent it from
        // being called on those earlier systems.
        // Unfortunately the supported way to silence this warning, using @available,
        // produces linker errors for __isPlatformVersionAtLeast, so we have to use
        // a different method.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"
        AVCaptureDeviceDiscoverySession *discoverySession =
            [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeMicrophone,
                                                                               AVCaptureDeviceTypeExternal]
                                                                   mediaType:AVMediaTypeAudio
                                                                    position:AVCaptureDevicePositionUnspecified];
        return discoverySession.devices;
#pragma clang diagnostic pop
    } else {
        // We're intentionally using a deprecated API here specifically for versions
        // of macOS where it's not deprecated, so we can ignore any deprecation
        // warnings:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        return [AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio];
#pragma clang diagnostic pop
    }
}

+ (NSArray<NSString *> *)captureDeviceNames;
{
    NSMutableArray *result = [NSMutableArray array];

    for (AVCaptureDevice *device in [CaptureDevice captureDevices]) {
        [result addObject:[device localizedName]];
    }

    return result;
}

+ (AVCaptureDevice *)findCaptureDevice:(NSString *)name
{
    for (AVCaptureDevice *device in [CaptureDevice captureDevices]) {
        if ([[device localizedName] isEqualToString:name]) {
            return device;
        }
    }

    return nil;
}

- (void)dealloc
{
    // make sure we don't process any further samples
    [_audioCaptureSession stopRunning];
    _audioConnection = nil;

    // make sure nothing gets stuck on this signal
    [_samplesArrivedSignal lock];
    [_samplesArrivedSignal signal];
    [_samplesArrivedSignal unlock];
    [_samplesArrivedSignal release];
    _samplesArrivedSignal = nil;

    [_audioCaptureSession release];
    _audioCaptureSession = nil;

    if (audioSampleBuffer.buffer != NULL) {
        TPCircularBufferCleanup(&audioSampleBuffer);
    }

    [super dealloc];
}

- (int)setupCaptureDevice:(AVCaptureDevice *)device
               sampleRate:(UInt32)sampleRate
                frameSize:(UInt32)frameSize
                 channels:(UInt8)channels
{
    AVCaptureSession *audioCaptureSession = [[AVCaptureSession alloc] init];
    self.audioCaptureSession = audioCaptureSession;
    // CaptureDevice keeps it through the retained property; release our local alloc.
    [audioCaptureSession release];

    NSError *error = nil;
    AVCaptureDeviceInput *audioInput = 
        [AVCaptureDeviceInput deviceInputWithDevice:device
                                              error:&error];
    if (audioInput == nil) {
        return -1;
    }

    if ([self.audioCaptureSession canAddInput:audioInput]) {
        [self.audioCaptureSession addInput:audioInput];
    } else {
        return -1;
    }

    AVCaptureAudioDataOutput *audioOutput = [[AVCaptureAudioDataOutput alloc] init];

    [audioOutput setAudioSettings:@{
        (NSString *) AVFormatIDKey: [NSNumber numberWithUnsignedInt:kAudioFormatLinearPCM],
        (NSString *) AVSampleRateKey: [NSNumber numberWithUnsignedInt:sampleRate],
        (NSString *) AVNumberOfChannelsKey: [NSNumber numberWithUnsignedInt:channels],
        (NSString *) AVLinearPCMBitDepthKey: [NSNumber numberWithUnsignedInt:32],
        (NSString *) AVLinearPCMIsFloatKey: @YES,
        (NSString *) AVLinearPCMIsNonInterleaved: @NO
    }];

    // TPCircularBuffer is used as a single-producer/single-consumer buffer here.
    // Keep sample callbacks serialized so only one producer writes to it at a time.
    dispatch_queue_attr_t qos =
        dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, 0);
    dispatch_queue_t recordingQueue = dispatch_queue_create("audioSamplingQueue", qos);

    [audioOutput setSampleBufferDelegate:self queue:recordingQueue];
    // The output keeps the queue for callbacks; release our local create ownership.
    dispatch_release(recordingQueue);

    if ([self.audioCaptureSession canAddOutput:audioOutput]) {
        [self.audioCaptureSession addOutput:audioOutput];
    } else {
        // The session did not keep the output; release our local alloc.
        [audioOutput release];
        return -1;
    }

    self.audioConnection = [audioOutput connectionWithMediaType:AVMediaTypeAudio];

    NSCondition *samplesArrivedSignal = [[NSCondition alloc] init];
    self.samplesArrivedSignal = samplesArrivedSignal;
    // CaptureDevice keeps it through the retained property; release our local alloc.
    [samplesArrivedSignal release];

    if (!TPCircularBufferInit(&self->audioSampleBuffer, kBufferLength * channels)) {
        // The session kept the output, but setup failed; release our local alloc.
        [audioOutput release];
        return -1;
    }

    [self.audioCaptureSession startRunning];
    // The session keeps added outputs; release our local alloc.
    [audioOutput release];

    return 0;
}

- (void)captureOutput:(AVCaptureOutput *)output
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection
{
    if (connection == self.audioConnection) {
        AudioBufferList audioBufferList;
        CMBlockBufferRef blockBuffer = NULL;

        OSStatus status = CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer(
                sampleBuffer,
                NULL,
                &audioBufferList,
                sizeof(audioBufferList),
                NULL,
                NULL,
                0,
                &blockBuffer);
        if (status != noErr || audioBufferList.mNumberBuffers == 0) {
            if (blockBuffer != NULL) {
                CFRelease(blockBuffer);
            }
            return;
        }

        // NSAssert(audioBufferList.mNumberBuffers == 1, @"Expected interleaved PCM format but buffer contained %u streams", audioBufferList.mNumberBuffers);

        // this is safe, because an interleaved PCM stream has exactly one buffer,
        // and we don't want to do sanity checks in a performance critical exec path
        AudioBuffer audioBuffer = audioBufferList.mBuffers[0];

        TPCircularBufferProduceBytes(&self->audioSampleBuffer, audioBuffer.mData, audioBuffer.mDataByteSize);
        if (blockBuffer != NULL) {
            CFRelease(blockBuffer);
        }

        // NSCondition requires signaling while holding its lock.
        // TPCircularBuffer remains the atomic single-producer/single-consumer state.
        [self.samplesArrivedSignal lock];
        [self.samplesArrivedSignal signal];
        [self.samplesArrivedSignal unlock];
    }
}

@end
