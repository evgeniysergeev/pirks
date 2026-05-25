# Windows microphone access

`microphone-test.exe` needs access to a Windows audio capture device.
If Windows privacy settings block microphone access, the test can print
`HRESULT = 0x80070005` and skip with:

```
No accessible audio capture device found
```

## Enable microphone access

Open Windows Settings and go to:

```
Privacy & security -> Microphone
```

On Windows 10 this page can be named:

```
Privacy -> Microphone
```

Enable these options:

- `Microphone access`
- `Let apps access your microphone`
- `Let desktop apps access your microphone`

You can open the microphone privacy page directly:

1. Press `Win + R`.
2. Enter `ms-settings:privacy-microphone`.
3. Press `Enter`.

## Check input device

Open Windows Settings and go to:

```
System -> Sound -> Input
```

Make sure a working microphone is selected and enabled.

## Troubleshooting

If `microphone-test.exe` still reports `HRESULT = 0x80070005`, Windows is
denying access to the microphone. Re-check the privacy toggles above and run
the test again.

Windows may not show a separate toggle for `microphone-test.exe` because it is
a desktop application. The important setting is `Let desktop apps access your
microphone`.
