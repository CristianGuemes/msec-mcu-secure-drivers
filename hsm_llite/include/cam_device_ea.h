/**************************************************************************
  Crypto Library "Early Adopter" Device Support Header

  Company:
    Microchip Technology Inc.

  File Name:
    cam_device_ea.h

  Summary:
    CAM/HSM-Lite Hardware Driver Early Adopter Device Support.

  Description:
    This header file defines supported "Early Adopter" devices by the CAM/HSM-Lite Hardware Driver.

  File created on 2026-03-25t11:36:06
**************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026 Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/
//DOM-IGNORE-END

/** @cond INTERNAL */
/**
 * Crypto Framework Library Source
 *
 * @file cam_device_ea.h
 * @defgroup  device_ea DEVICE_EA
 *
 * @brief This header file defines supported "Early Adopter" devices by the CAM/HSM-Lite Hardware Driver.
 *
 */

#ifndef CAM_DEVICE_EA_H
#define CAM_DEVICE_EA_H

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

/** @cond INTERNAL **/
#if defined(__dsPIC33AKV256GMS505__) \
    || defined (__dsPIC33AKV256GMS506__) \
    || defined (__dsPIC33AKV256GMS508__) \
    || defined (__dsPIC33AKV256GMS510__) \
    || defined (__dsPIC33AKV512GMS505__) \
    || defined (__dsPIC33AKV512GMS506__) \
    || defined (__dsPIC33AKV512GMS508__) \
    || defined (__dsPIC33AKV512GMS510__) \
    || defined (__dsPIC33AKV256GMS205__) \
    || defined (__dsPIC33AKV256GMS206__) \
    || defined (__dsPIC33AKV256GMS208__) \
    || defined (__dsPIC33AKV256GMS210__) \
    || defined (__dsPIC33AKV512GMS205__) \
    || defined (__dsPIC33AKV512GMS206__) \
    || defined (__dsPIC33AKV512GMS208__) \
    || defined (__dsPIC33AKV512GMS210__)
    /**
     * @ingroup device_ea
     * @def CAM_HW_VERSION
     * @brief Defines the hardware version for the CAM 05346 target.
     *
     * This macro is defined for supported CAM 05346 device families.
     */
    #define CAM_HW_VERSION CAM_05346
#endif
/** @endcond **/

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif // CAM_DEVICE_EA_H

/**
 End of File
*/
