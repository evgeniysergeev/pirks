# macOS audio TODO

These are static-audit findings for the macOS microphone implementation. Verify
them on macOS with the macOS target/build before changing behavior.

## Ownership and cleanup

- Release `CMBlockBufferRef blockBuffer` returned by
  `CMSampleBufferGetAudioBufferListWithRetainedBlockBuffer`.
- Fix `captureDeviceNames` returning an `alloc/init` array without autorelease
  or explicit ownership transfer.
- Review `audioCaptureSession` ownership. It is currently an `assign` property,
  allocated with `alloc/init`, and is not stopped or released in `dealloc`.
- Fix the `MacAudioInput` constructor failure path so `captureDevice_` is
  released if `setupCaptureDevice` fails.
- Review `dispatch_queue_create` ownership and whether the recording queue needs
  release under the project's Objective-C memory mode.
- Replace direct `[audioInput dealloc]` with normal Objective-C memory
  management.
