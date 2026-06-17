![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)

# Microchip MSEC MCU Secure Drivers Release Notes

## MSEC MCU Secure Drivers v1.0.0

This release introduces initial support for the following security drivers:

- **PUF Driver**
  - Physical Unclonable Function (PUF) driver for SRAM PUF hardware peripheral
  - Supported devices: PIC32CM SG00

- **HSM Lite Driver (CAM)**
  - Crypto Acceleration Module driver for HSM Lite hardware
  - Supported devices: PIC32CK SG/GC

- **New Features and Enhancements**
  - Initial release of PUF Driver with support for enrollment, reconstruction, key derivation, key wrapping/unwrapping, random number generation, zeroization, and test functionality.
  - Initial release of HSM Lite CAM Driver with support for AES, ECDSA, ECDH, hashing (SHA-2), PKE operations, and TRNG.

- **Bug Fixes**
  - None.

**Known Issues**
  - None.

**Development Tools**

- [MPLAB® X IDE v6.20](https://www.microchip.com/mplab/mplab-x-ide) or higher
- [MPLAB® XC32 C/C++ Compiler v4.45](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers) or higher
