/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_ecdh.c

  Summary:
    ECDH Hardware driver implementation

  Description:
    This source file interacts directly with the PKE hardware
    for ECDH Shared Secret generation for the following families of
    Microchip microcontrollers:
    dsPIC33AK with Crypto Accelerator Module.
**************************************************************************/

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

/* MISRA C:2012 Rule 8.4 Deviation Justification
 *
 * ID      Category   Description
 * 8.4     Required   A compatible declaration shall be visible when an object or
 *                    function with external linkage is defined
 *
 * Justification:
 * All externally visible CAM/HSM driver APIs (e.g., DRV_CRYPTO_* functions)
 * are declared in their corresponding public header files (such as cam_aes.h,
 * cam_ecdh.h, cam_ecdsa.h, cam_hash.h). However, some MISRA analysis tools may
 * not fully parse these external headers, resulting in false positives where it
 * appears that compatible declarations are missing. The implementation is
 * compliant, as each externally linked function has a matching prototype defined
 * in the associated header file.
 *
 * Scope:
 * This justification applies to all functions with external linkage implemented
 * in this source file.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************


#include "../inc/cam_ecdh.h"
#include "cam_pke_local.h"

// *****************************************************************************
// *****************************************************************************
// Section: ECDH Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDH_InitEccParams(PKE_CONFIG *eccData, uint8_t *privateKey, 
        uint32_t privateKeyLength, uint8_t *publicKey, uint32_t publicKeyLength, PKE_ECC_CURVE hwEccCurve)
{
    PKE_ECC_SIZE size;
    CRYPTO_PKE_RESULT result = DRV_CRYPTO_PKE_GetCurveSize(hwEccCurve, &size);

    if (result == CRYPTO_PKE_RESULT_SUCCESS)
    {
        eccData->operation = ECDH_ECC_MULTIPLY;
        eccData->curve = hwEccCurve;
        
        eccData->publicKey.x = publicKey;
        uint32_t halfLength = publicKeyLength / 2U;
        eccData->publicKey.y = &publicKey[halfLength];
        eccData->publicKey.size = halfLength;

        
        eccData->operand1.data = privateKey;
        eccData->operand1.size = privateKeyLength;
        eccData->operandSize = size;
        DRV_CRYPTO_PKE_SetupEngine();
    }

    return result;
}

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDH_GetSharedSecret(PKE_CONFIG *eccData,
                                                  uint8_t *secret,
                                                  uint32_t secretLength)
{
    ECC_ERROR errorCode = DRV_CRYPTO_PKE_ExecuteCommand(eccData);

    if (errorCode == ECC_NO_ERROR)
    {
        /* Read X coordinate only */
        errorCode = DRV_CRYPTO_PKE_ReadLocation(MODULUS_CX, secret, secretLength);
    }
    
    // Clear memory since there is no need for the input data to stay in RAM
    DRV_CRYPTO_PKE_ClearMemory();

    return DRV_CRYPTO_PKE_ConvertErrorToResult(errorCode);

}
