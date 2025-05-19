# nrfutil Commands Note

## Device Information

### Check connected devices
```
nrfutil device list
```

## Erase Commands

### Erase device
```
nrfutil device erase --serial-number 821005723
```

### Erase with J-Link trait explicitly specified
```
nrfutil device erase --serial-number 821005723 --traits jlink
```

### Recover device (use when erase fails or for readback protection)
```
nrfutil device recover --serial-number 821005723
```

## Firmware Update Commands

### Basic firmware update
```
nrfutil device program --firmware your_firmware.hex --serial-number 821005723
```

### Update with explicit J-Link trait
```
nrfutil device program --firmware your_firmware.hex --serial-number 821005723 --traits jlink
```

### Update with verification and specific erase mode
```
nrfutil device program --firmware your_firmware.hex --serial-number 821005723 --options verify=VERIFY_READ,chip_erase_mode=ERASE_ALL
```

### Update with reset after programming
```
nrfutil device program --firmware your_firmware.hex --serial-number 821005723 --options reset=RESET_SYSTEM
```

## Troubleshooting

If commands fail:
1. Update nrfutil: `nrfutil upgrade device`
2. Try recover command for erase failures
3. Check J-Link software version
4. For programming issues: `nrfutil device program --log-level trace` for detailed logs
