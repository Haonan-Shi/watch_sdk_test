# mpcli Flash Programming Tool - User Guide

**Tool Version**: v4.0.0.6
**Supported IC**: RTL87X3G
**Supported Platforms**: macOS / Windows

---

## 1. Directory Structure

```
<work>/
├── bin/                          # Image directory — run all commands from here
│   ├── flash_img.json            # Full flash programming configuration file
│   └── image/
│       ├── BANK0/                # Firmware images
│       │   ├── bt_audio_receiver_bank0_*.bin
│       │   ├── boot_patch0_*.bin
│       │   ├── boot_patch1_*.bin
│       │   ├── upperstack_*.bin
│       │   ├── OTAHeader_Bank0_*.bin
│       │   ├── stack_patch_bank0_*.bin
│       │   ├── sys_patch_bank0_*.bin
│       │   ├── root_*.bin
│       │   ├── dsp_sys_image_*.bin
│       │   ├── dsp_app_image_*.bin
│       │   └── dsp_config_image_*.bin
│       └── rcfg/For EVB/         # Configuration images
│           ├── SYSTEM_Config_*.bin
│           ├── APP_Config_*.bin
│           └── VPData_*.bin
└── mpcli_metal_tool/             # Tool directory — do not modify
    ├── mpcli                     # Executable (macOS) / mpcli.exe (Windows)
    ├── fw/                       # Internal firmware — do not delete
    └── config/                   # Tool configuration — do not delete
```

---

## 2. Prerequisites

### Hardware Connection

Connect the device to your computer via a USB-to-serial adapter.


| SoC Pin | SoC Signal | Connect to  |
| ------- | ---------- | ----------- |
| P3_0    | RXD        | Adapter TX  |
| P3_1    | TXD        | Adapter RX  |
| GND     | GND        | Adapter GND |

**Entering flash mode**: Pull P2_0 to GND, then reset the device.

### Find the Serial Port

**macOS**:

```bash
ls /dev/tty.usbserial-*
```

Example output: `/dev/tty.usbserial-A10JR5W5`

**Windows**: Open Device Manager and check the COM port number, e.g. `COM6`.

### First-time Setup (macOS only)

Grant execute permission:

```bash
chmod +x ./mpcli_metal_tool/mpcli
```

---

## 3. Command Options


| Option           | Description                                                                                       |
| ---------------- | ------------------------------------------------------------------------------------------------- |
| `-c <port>`      | Serial port. macOS example:`/dev/tty.usbserial-A10JR5W5`; Windows example: `COM6`                 |
| `-T <ic_type>`   | IC type. Always set to`RTL87X3G`                                                                  |
| `-M 5`           | Operation mode. Always set to`5` (MP Loader mode)                                                 |
| `-r`             | Reboot device after operation completes                                                           |
| `-f <json_file>` | Path to the full flash programming configuration file                                             |
| `-a`             | Program all images listed in the JSON file                                                        |
| `-e`             | Erase flash. Must be used with`-A` and `-S`                                                       |
| `-A <address>`   | Flash address in hexadecimal, e.g.`-A 0x70000000`                                                 |
| `-S <size>`      | Size to erase in bytes (hex), e.g.`-S 0x2000`                                                     |
| `-p`             | Program a single image. Must be used with`-A` and `-F`                                            |
| `-F <file>`      | Path to the binary image file                                                                     |
| `--chip_erase`   | Erase the entire flash chip                                                                       |
| `-b <baud_rate>` | Programming baud rate (default:`1000000`). Use `-b 2000000` for 2 Mbps or `-b 3000000` for 3 Mbps |

> **Baud rate note**: The initial handshake with MP Loader runs at 115200 bps. Once connected, the tool automatically switches to 1 Mbps for programming. Use `-b 2000000` or `-b 3000000` to further increase the programming baud rate.

---

## 4. Operations

> **Note**: `fw/` and `config/` must reside in the same directory as the `mpcli` executable. Paths in the JSON file can be absolute paths or paths relative to the JSON file.
>
> The `mpcli` executable path and all flash image file paths specified on the command line must be absolute paths or paths relative to the current working directory.
>
> The command examples below assume the working directory has been set to the `<work>/` directory.

### 4.1 Full Flash Programming (all images from JSON)

Use case: Initial programming or full firmware update.

**macOS**:

```bash
./mpcli_metal_tool/mpcli -c /dev/tty.usbserial-A10JR5W5 -M 5 -T RTL87X3G -f ./bin/flash_img.json -a -r
```

**Windows**:

```bash
.\mpcli_metal_tool\mpcli.exe -c COM6 -M 5 -T RTL87X3G -f ./bin/flash_img.json -a -r
```

Expected output on success:

```
Download : Success | 14 image files have been downloaded successfully!
MPCLI exit: success.
```

---

### 4.2 Erase Flash Region

Use case: Clear data at a specific address range, e.g. erase User Data area.

**macOS**:

```bash
./mpcli_metal_tool/mpcli -c /dev/tty.usbserial-A10JR5W5 -M 5 -T RTL87X3G -e -A <address> -S <size> -r
```

**Example**: Erase `0x2000` bytes starting from `0x70000000`:

```bash
./mpcli_metal_tool/mpcli -c /dev/tty.usbserial-A10JR5W5 -M 5 -T RTL87X3G -e -A 0x70000000 -S 0x2000 -r
```

Expected output on success:

```
CMD: flash erase 0x70000000 0x2000  ok
MPCLI exit: success.
```

---

### 4.3 Program a Single Image

Use case: Update one specific image, e.g. App only.

**macOS**:

```bash
./mpcli_metal_tool/mpcli -c /dev/tty.usbserial-A10JR5W5 -M 5 -T RTL87X3G -p -A <address> -F <file_path> -r
```

**Example**: Program the App image to Bank0 (`0x7009E000`):

```bash
./mpcli_metal_tool/mpcli -c /dev/tty.usbserial-A10JR5W5 -M 5 -T RTL87X3G \
  -p -A 0x7009E000 \
  -F ./bin/image/BANK0/bt_audio_receiver_bank0_MP-v3.14.7.1858-934b29aa-9255043d3de936d25f5e8f9e1adae8cd.bin \
  -r
```

Expected output on success:

```
Download : Success | 1 image files have been downloaded successfully!
MPCLI exit: success.
```

---

### 4.4 Erase Entire Flash

Use case: Completely wipe all flash data and restore the chip to a blank state.

**macOS**:

```bash
./mpcli_metal_tool/mpcli -c /dev/tty.usbserial-A10JR5W5 -T RTL87X3G --chip_erase -M 5 -r
```

**Windows**:

```bash
.\mpcli_metal_tool\mpcli.exe -c COM6 -T RTL87X3G --chip_erase -M 5 -r
```

Expected output on success:

```
chip erase ok
Reboot OK!
MPCLI exit: success.
```

---

## 5. Flash Address Reference

### 5.1 High-Level Region (Bank-independent)


| Image             | Flash Address | flash_map.h Macro  | File Path (relative to`bin/`)            |
| ----------------- | ------------- | ------------------ | ---------------------------------------- |
| System Config     | `0x70002000`  | `OEM_CFG_ADDR`     | `image/rcfg/For EVB/SYSTEM_Config_*.bin` |
| Boot Patch0       | `0x70004000`  | `BOOT_PATCH0_ADDR` | `image/BANK0/boot_patch0_*.bin`          |
| Boot Patch1       | `0x70007000`  | `BOOT_PATCH1_ADDR` | `image/BANK0/boot_patch1_*.bin`          |
| Upperstack        | `0x7000A000`  | `UPPERSTACK_ADDR`  | `image/BANK0/upperstack_*.bin`           |
| User Data1 (root) | `0x70536000`  | `USER_DATA1_ADDR`  | `image/BANK0/root_*.bin`                 |
| Voice Prompt Data | `0x704F3000`  | `VP_DATA_ADDR`     | `image/rcfg/For EVB/VPData_*.bin`        |

### 5.2 OTA Bank0 Image Addresses


| Image       | Flash Address | flash_map.h Macro        | File Path (relative to`bin/`)         |
| ----------- | ------------- | ------------------------ | ------------------------------------- |
| OTA Header  | `0x70049000`  | `BANK0_OTA_HDR_ADDR`     | `image/BANK0/OTAHeader_Bank0_*.bin`   |
| Stack Patch | `0x7004A000`  | `BANK0_STACK_PATCH_ADDR` | `image/BANK0/stack_patch_bank0_*.bin` |
| Sys Patch   | `0x7007C000`  | `BANK0_SYS_PATCH_ADDR`   | `image/BANK0/sys_patch_bank0_*.bin`   |
| App         | `0x7009E000`  | `BANK0_APP_ADDR`         | `image/BANK0/bt_audio_receiver_*.bin` |
| DSP Sys     | `0x70207000`  | `BANK0_DSP_SYS_ADDR`     | `image/BANK0/dsp_sys_image_*.bin`     |
| DSP App     | `0x70261000`  | `BANK0_DSP_APP_ADDR`     | `image/BANK0/dsp_app_image_*.bin`     |
| DSP Config  | `0x70292000`  | `BANK0_DSP_CFG_ADDR`     | `image/BANK0/dsp_config_image_*.bin`  |
| App Config  | `0x7029C000`  | `BANK0_APP_CFG_ADDR`     | `image/rcfg/For EVB/APP_Config_*.bin` |

### 5.3 OTA Bank1 Image Addresses

> **Note**: When programming Bank1, use the **same binary files** as Bank0. Only replace the `-A` address with the corresponding Bank1 address below.


| Image       | Flash Address | flash_map.h Macro        |
| ----------- | ------------- | ------------------------ |
| OTA Header  | `0x7029E000`  | `BANK1_OTA_HDR_ADDR`     |
| Stack Patch | `0x7029F000`  | `BANK1_STACK_PATCH_ADDR` |
| Sys Patch   | `0x702D1000`  | `BANK1_SYS_PATCH_ADDR`   |
| App         | `0x702F3000`  | `BANK1_APP_ADDR`         |
| DSP Sys     | `0x7045C000`  | `BANK1_DSP_SYS_ADDR`     |
| DSP App     | `0x704B6000`  | `BANK1_DSP_APP_ADDR`     |
| DSP Config  | `0x704E7000`  | `BANK1_DSP_CFG_ADDR`     |
| App Config  | `0x704F1000`  | `BANK1_APP_CFG_ADDR`     |

**Bank1 single image example** (App):

```bash
./mpcli_metal_tool/mpcli -c /dev/tty.usbserial-A10JR5W5 -M 5 -T RTL87X3G \
  -p -A 0x702F3000 \
  -F ./bin/image/BANK0/bt_audio_receiver_*.bin \
  -r
```

---

## 6. Troubleshooting

**Q: Connection failed / handshake timeout**
A: Verify the serial port name is correct. Confirm the device is powered on and in MP mode. Check that the USB-to-serial driver is properly installed.

**Q: macOS says "cannot verify the developer"**
A: Go to System Settings → Privacy & Security and click "Open Anyway".

**Q: Programming succeeds but device fails to boot**
A: Confirm the image version matches the hardware variant. For full programming, verify all required images were included.
