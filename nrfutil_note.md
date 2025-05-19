# nrfutil Erase Note

## Basic Commands

### Check connected devices
```
nrfutil device list
```

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

## Troubleshooting

If erase fails:
1. Update nrfutil: `nrfutil upgrade device`
2. Try recover command instead
3. Check J-Link software version
