/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_ecdsa.c

  Summary:
    ECDSA Hardware driver implementation

  Description:
    This source file interacts directly with the PKE hardware
    for ECDSA algorithms for Microchip microcontrollers equipped with a Crypto Accelerator Module.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "../inc/cam_ecdsa.h"
#include "cam_pke_local.h"

// *****************************************************************************
// *****************************************************************************
// Section: ECDSA Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_InitEccParamsSign(PKE_CONFIG *eccData, uint8_t *inputHash,
        uint32_t hashLength, uint8_t * privateKey, uint32_t privateKeyLength, PKE_ECC_CURVE eccCurve)
{
    PKE_ECC_SIZE size;
    CRYPTO_PKE_RESULT result = DRV_CRYPTO_PKE_GetCurveSize(eccCurve, &size);

    if (result == CRYPTO_PKE_RESULT_SUCCESS)
    {
        eccData->curve = eccCurve;
        eccData->operation = ECDSA_SIGNATURE_GENERATION;
        eccData->operand1.data = privateKey;
        eccData->operand1.size = privateKeyLength;
        //Subtract 2 from size since P521 is not on a byte boundary
        eccData->operand2.size = (uint32_t) size - 2U;
        eccData->operand3.data = inputHash;
        eccData->operand3.size = hashLength;
        //Required by the CAMPKCMD registers OPSIZE bitfield 
        eccData->operandSize = (uint32_t) size - 1U;

        DRV_CRYPTO_PKE_SetupEngine();
    }

    return result;
}

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_Sign(PKE_CONFIG *eccData, uint8_t * outputSignature, uint32_t signatureLength)
{
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_SUCCESS;
    ECC_ERROR errorCode;

    errorCode = DRV_CRYPTO_PKE_ExecuteCommand(eccData);

    if (errorCode == ECC_NO_ERROR)
    {
        errorCode = DRV_CRYPTO_PKE_ReadLocation(ECDSA_R, outputSignature, signatureLength / 2U);
    }

    if (errorCode == ECC_NO_ERROR)
    {
        uint32_t halfLength = signatureLength / 2U;
        errorCode = DRV_CRYPTO_PKE_ReadLocation(ECDSA_S, &outputSignature[halfLength], halfLength);
    }

    if(errorCode != ECC_NO_ERROR)
    {
        result = CRYPTO_PKE_RESULT_ERROR_FAIL;
    }

    // Clear memory since there is no need for the input data to stay in RAM
    DRV_CRYPTO_PKE_ClearMemory();
    
    return result;
}

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_InitEccParamsVerify(PKE_CONFIG *eccData, uint8_t *inputHash,
        uint32_t hashLength, uint8_t *inputSignature, uint32_t signatureLength, uint8_t *publicKey,
        uint32_t publicKeyLength, PKE_ECC_CURVE eccCurve)
{
    PKE_ECC_SIZE size;
    CRYPTO_PKE_RESULT result = DRV_CRYPTO_PKE_GetCurveSize(eccCurve, &size);
    /* Compressed keys not supported */
    if (publicKey[0] != UNCOMPRESSED_TYPE)
    {
        result =  CRYPTO_PKE_ERROR_PUBKEYCOMPRESS;
    }

    if (result == CRYPTO_PKE_RESULT_SUCCESS)
    {
        //The signature is broken up into its S and R values of equal length.
        uint32_t halfSignatureLength = signatureLength / 2U;
        //The public key compression type is removed and broken into its X and Y values of equal length
        uint32_t halfPubKeyLength = (publicKeyLength - 1U) / 2U;
        eccData->curve = eccCurve;
        eccData->operation = ECDSA_SIGNATURE_VERIFICATION;
        eccData->operand1.data = inputSignature;
        eccData->operand1.size = halfSignatureLength;
        eccData->operand2.data = &inputSignature[halfSignatureLength];
        eccData->operand2.size = halfSignatureLength;
        eccData->operand3.data = inputHash;
        eccData->operand3.size = hashLength;

        eccData->publicKey.x   = &publicKey[1U]; //Starts at 1 to skip key compression type
        eccData->publicKey.y   = &publicKey[1U + halfPubKeyLength]; //Similar to above and skips the x portion 
        eccData->publicKey.size = halfPubKeyLength;
        //Required by the CAMPKCMD registers OPSIZE bitfield 
        eccData->operandSize = ((uint8_t) size - 1U);

        DRV_CRYPTO_PKE_SetupEngine();
    }

    return result;
}


CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_Verify(PKE_CONFIG *eccData)
{
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_SUCCESS;
    ECC_ERROR errorCode;

    errorCode = DRV_CRYPTO_PKE_ExecuteCommand(eccData);

    if (errorCode != ECC_NO_ERROR)
    {
        result = CRYPTO_PKE_RESULT_ERROR_FAIL;
    }
    
    // Clear memory since there is no need for the input data to stay in RAM
    DRV_CRYPTO_PKE_ClearMemory();
    
    return result;
}

// *****************************************************************************
// *****************************************************************************
// Section: Non-Blocking ECDSA Common Implementation
// *****************************************************************************
// *****************************************************************************

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_Sign_Start(uint8_t *inputHash, 
    uint32_t hashLength, uint8_t *privateKey, uint32_t privateKeyLength, 
    PKE_ECC_CURVE eccCurve)
{
    PKE_CONFIG eccData;
    PKE_ECC_SIZE size;
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_ERROR_FAIL;
    
    if (DRV_CRYPTO_ECDSA_GetStatus() != CRYPTO_PKE_RESULT_BUSY)
    {
        result = DRV_CRYPTO_PKE_GetCurveSize(eccCurve, &size);
        
        if (result == CRYPTO_PKE_RESULT_SUCCESS)
        {
            eccData.curve = eccCurve;
            eccData.operation = ECDSA_SIGNATURE_GENERATION;
            eccData.operand1.data = privateKey;
            eccData.operand1.size = privateKeyLength;
            //Subtract 2 from size since P521 is not on a byte boundary
            eccData.operand2.size = (uint32_t) size - 2U;
            eccData.operand3.data = inputHash;
            eccData.operand3.size = hashLength;
            //Required by the CAMPKCMD registers OPSIZE bitfield 
            eccData.operandSize = (uint32_t) size - 1U;

            ECC_ERROR errorCode = DRV_CRYPTO_PKE_SetVariableLocations(&eccData);

            if (errorCode == ECC_NO_ERROR)
            {
                DRV_CRYPTO_PKE_SetCommand(&eccData);
                DRV_CRYPTO_PKE_StartEngine_NoWait();
                errorCode = DRV_CRYPTO_PKE_CheckErrors();
            }

            if (errorCode != ECC_NO_ERROR)
            {
                result = DRV_CRYPTO_PKE_ConvertErrorToResult(errorCode);
            }
        }
    }
    else
    {
        result = CRYPTO_PKE_RESULT_BUSY;
    }
    
    return result;
}

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_Verify_Start(uint8_t *inputHash, uint32_t hashLength, 
        uint8_t *inputSignature, uint32_t signatureLength, uint8_t *publicKey, uint32_t publicKeyLength, 
        PKE_ECC_CURVE eccCurve)
{
    PKE_CONFIG eccData;
    PKE_ECC_SIZE size;
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_ERROR_FAIL;
    
    if (DRV_CRYPTO_ECDSA_GetStatus() != CRYPTO_PKE_RESULT_BUSY)
    {
        result = DRV_CRYPTO_PKE_GetCurveSize(eccCurve, &size);
            
        /* Compressed keys not supported */
        if (publicKey[0] != UNCOMPRESSED_TYPE)
        {
            result = CRYPTO_PKE_ERROR_PUBKEYCOMPRESS;
        }

        if (result == CRYPTO_PKE_RESULT_SUCCESS)
        {
            // The signature is broken up into its S and R values of equal length.
            uint32_t halfSignatureLength = signatureLength / 2U;
            // The public key compression type is removed and broken into its X and Y values of equal length
            uint32_t halfPubKeyLength = (publicKeyLength - 1U) / 2U;
            eccData.curve = eccCurve;
            eccData.operation = ECDSA_SIGNATURE_VERIFICATION;
            eccData.operand1.data = inputSignature;
            eccData.operand1.size = halfSignatureLength;
            eccData.operand2.data = &inputSignature[halfSignatureLength];
            eccData.operand2.size = halfSignatureLength;
            eccData.operand3.data = inputHash;
            eccData.operand3.size = hashLength;

            eccData.publicKey.x   = &publicKey[1U]; // Starts at 1 to skip key compression type
            eccData.publicKey.y   = &publicKey[1U + halfPubKeyLength]; // Similar to above and skips the x portion 
            eccData.publicKey.size = halfPubKeyLength;
            //Required by the CAMPKCMD registers OPSIZE bitfield 
            eccData.operandSize = ((uint8_t) size - 1U);
            
            ECC_ERROR errorCode = DRV_CRYPTO_PKE_SetVariableLocations(&eccData);

            if (errorCode == ECC_NO_ERROR)
            {
                DRV_CRYPTO_PKE_SetCommand(&eccData);
                DRV_CRYPTO_PKE_StartEngine_NoWait();
                errorCode = DRV_CRYPTO_PKE_CheckErrors();
            }

            if (errorCode != ECC_NO_ERROR)
            {
                result = DRV_CRYPTO_PKE_ConvertErrorToResult(errorCode);
            }
        }
    }
    else
    {
        result = CRYPTO_PKE_RESULT_BUSY;
    }
    
    return result;
}

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_Sign_GetResult(uint8_t *outputSignature, uint32_t signatureLength)
{
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_SUCCESS;
    ECC_ERROR errorCode = DRV_CRYPTO_PKE_CheckErrors();
    
    if (DRV_CRYPTO_ECDSA_GetStatus() != CRYPTO_PKE_RESULT_BUSY)
    {
        
        if (errorCode == ECC_NO_ERROR)
        {
            errorCode = DRV_CRYPTO_PKE_ReadLocation(ECDSA_R, outputSignature, signatureLength / 2U);
        }

        if (errorCode == ECC_NO_ERROR)
        {
            uint32_t halfLength = signatureLength / 2U;
            errorCode = DRV_CRYPTO_PKE_ReadLocation(ECDSA_S, &outputSignature[halfLength], halfLength);
        }
        
        if (errorCode != ECC_NO_ERROR)
        {
            result = DRV_CRYPTO_PKE_ConvertErrorToResult(errorCode);
        }
    }
    else
    {
        result = CRYPTO_PKE_RESULT_BUSY;
    }

    DRV_CRYPTO_PKE_ClearMemory_NoWait();
    return result;
}

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_Verify_GetResult(void)
{
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_ERROR_FAIL;
    ECC_ERROR errorCode = DRV_CRYPTO_PKE_CheckErrors();
    
    if (DRV_CRYPTO_ECDSA_GetStatus() != CRYPTO_PKE_RESULT_BUSY)
    {
        if (errorCode == ECC_NO_ERROR)
        {
            result = CRYPTO_PKE_RESULT_SUCCESS;
        }
        
        DRV_CRYPTO_PKE_ClearMemory_NoWait();
    }
    else
    {
        result = CRYPTO_PKE_RESULT_BUSY;
    }
    
    return result;
}

CRYPTO_PKE_RESULT DRV_CRYPTO_ECDSA_GetStatus(void)
{
    CRYPTO_PKE_STATUS status = (CRYPTO_PKE_STATUS)CAMPKSTAT.bits.PKBUSY;
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_BUSY;
    if(status == CRYPTO_PKE_STATUS_IDLE)
    {
        result = CRYPTO_PKE_RESULT_IDLE;
    }
    return result;
}
