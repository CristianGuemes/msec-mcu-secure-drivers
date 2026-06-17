![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)

# MSEC MCU Secure Drivers - PUF Driver

The Physical Unclonable Function (PUF) Driver offers an easy access to configure and use the SRAM PUF hardware peripheral in PIC32 devices.

The functionalities provided by the PUF Driver are the following:
- Enrollment
- Reconstruction (Start or Reconstruct)
- Key derivation
- Wrapping and unwrapping of user-defined keys
- Wrapping and unwrapping of random intrinsic keys
- Random number generation
- Zeroization
- Test functionality

Additionally, the PUF Driver provides access to configuration registers of the PUF peripheral:
- Soft reset
- Clock enable
- Privileged access management
- Status
- Command disable
- Restrict user context
- SRAM power control

# Contents Summary

| Folder    | Description                          |
| --------- | ------------------------------------ |
| include   | Public header files for the PUF API  |
| source    | Source implementation files           |

## Documentation

[Click here](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=puf_api_ref&redirect=true) to view documentation of the PUF API.

[Click here](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=puf_user_guide_intro&redirect=true) to view the PUF user guide.

## Supported Devices

- PIC32CM SG00
