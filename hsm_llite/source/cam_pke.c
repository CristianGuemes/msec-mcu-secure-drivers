/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_pke.c

  Summary:
    PKE functions for asymmetric crypto engine.

  Description:
    This source file contains functions to execute common PKE operations
    for Microchip microcontrollers equipped with a Crypto Accelerator Module.
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

/* MISRA C:2012 Rule 11.4 Deviation Justification
 *
 * ID      Category   Description
 * 11.4    Advisory   A conversion should not be performed between a pointer
 *                    to object and an integer type.
 *
 * Justification:
 * The PKE engine requires base addresses (e.g., pkeMicrocode, pkeRam) to be
 * written into hardware registers as integer values. This necessitates casting
 * object pointers to integer types. The conversion is safe, controlled, and
 * required by the hardware design to configure memory-mapped IO. No arithmetic
 * is performed on the converted values.
 *
 * Scope:
 * Applies only to pointer-to-integer casts for hardware register configuration
 * within this source file.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "cam_pke_local.h"
#include "../inc/cam_pke.h"
#include "../inc/cam_trng.h"

// *****************************************************************************
// *****************************************************************************
// Section: ECDH Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

void DRV_CRYPTO_PKE_IsrHelper(void)
{
    CAMPKTMR = 0;
    CAMPKCON.val = 0x2;
}

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Function Implementations
// *****************************************************************************
// *****************************************************************************

/**
 *  @brief This function starts the PKE engine with the initialized parameters.
 **/
static void lDRV_CRYPTO_PKE_StartEngine(void)
{
    CAMPKCON.bits.PKSTRT = ENABLE;
    while (CAMPKSTAT.bits.PKBUSY == (uint32_t)CRYPTO_PKE_STATUS_BUSY)
    {
    }
}

/**
 *  @brief Copies and swaps the endianness of the data at a location.
 *  @param output Output Data that is at little endian.
 *  @param input Input Data that will be swapped endianness.
 *  @param dataLength Length of the data to be swapped.
 **/
static void lDRV_CRYPTO_PKE_CopyAndSwapEndianness(uint8_t *output, const uint8_t *input, uint16_t dataLength)
{
    for (uint16_t i = 0; i < dataLength; i++) {
        output[i] = input[dataLength - 1U - i];
    }
}

/**
 *  @brief Write data to a given location.
 *  @param location Location to write the data to.
 *  @param data Variable to read data to the location.
 *  @param dataLength Length of the data to read in.
 *  @return ECC_NO_ERROR on success.  ECC_OP_SIZE_ERROR or ECC_OP_LOCATION_ERROR on failure.
 **/
static ECC_ERROR lDRV_CRYPTO_PKE_WriteLocation(uint8_t location, const uint8_t* data, uint16_t dataLength)
{
    ECC_ERROR errorCode = ECC_NO_ERROR;

    if (dataLength > PKE_OP_OFFSET)
    {
        errorCode = ECC_OP_SIZE_ERROR;
    }
    else if (location > (uint8_t) MAX_LOCATION)
    {
        errorCode = ECC_OP_LOCATION_ERROR;
    }
    else
    {
        volatile uint8_t* address = pkeRam[location];
        lDRV_CRYPTO_PKE_CopyAndSwapEndianness((void*)address, data, dataLength);
    }

    return errorCode;
}

/**
 *  @brief Converts the operation to a type usable by the PKE engine.
 *  @param inputOperation operation that has obfuscated values outside the library.
 *  @return Operation type usable by the pke engine.
 **/
static PKE_CMD_OPERATIONS lDRV_CRYPTO_PKE_GetOperation(PKE_OPERATIONS inputOperation){
    PKE_CMD_OPERATIONS operation = CMD_DEFAULT;
    switch(inputOperation)
    {
        case PKE_CLEAR_MEMORY:
            operation = CMD_PKE_CLEAR_MEMORY;
            break;
        case ECDSA_SIGNATURE_GENERATION:
            operation = CMD_ECDSA_SIGNATURE_GENERATION;
            break;
        case ECDSA_SIGNATURE_VERIFICATION:
            operation = CMD_ECDSA_SIGNATURE_VERIFICATION;
            break;
        case ECDH_ECC_MULTIPLY:
            operation = CMD_ECDH_ECC_MULTIPLY;
            break;
        default:
            operation = CMD_DEFAULT;
            break;
    }
    return operation;
}

/**
 *  @brief Get a TRNG Random number for signature generation.
 *  @param data Random number data.
 *  @param dataLength Length of the random number. 
 *  @return TRNG_NO_ERROR on success.  TRNG_EXEC_FAIL on failure.
 **/
static TRNG_ERROR lDRV_CRYPTO_PKE_GetRandomNumber(uint8_t* data, uint32_t dataLength)
{
    TRNG_ERROR trngStatus = TRNG_EXEC_FAIL;
    trngStatus = DRV_CRYPTO_TRNG_Setup();

    if (trngStatus == TRNG_NO_ERROR)
    {
        trngStatus = DRV_CRYPTO_TRNG_ReadData(data, dataLength);
    }

    return trngStatus;
}

// *****************************************************************************
// *****************************************************************************
// Section: ECC Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

void DRV_CRYPTO_PKE_StartEngine_NoWait(void)
{
    CAMPKCON.bits.PKSTRT = ENABLE;
}

void DRV_CRYPTO_PKE_SetCommand(const PKE_CONFIG* eccData)
{
    PKE_CMD_OPERATIONS op = lDRV_CRYPTO_PKE_GetOperation(eccData->operation);
    CAMPKCMD.val = 0;
    CAMPKCMD.bits.OPTYPE = op;
    CAMPKCMD.bits.FIELD = FIELD_PRIME;
    CAMPKCMD.bits.OPSIZE = eccData->operandSize;
    CAMPKCMD.bits.SELCRV = eccData->curve;
    CAMPKCMD.bits.EDWARDS = EDWARDS_OFF;
    CAMPKCMD.bits.FLAGA = 0;
}

/**
 *  @brief Return the ECC_ERRROR enum based on a given input.
 *  @param value Number that correlates to an ECC_ERROR type.
 *  @return ECC_NO_ERROR on success. Any other ECC_ERROR status on failure.
 **/
static ECC_ERROR lDRV_CRYPTO_PKE_GetErrorEnum(uint32_t value)
{
    ECC_ERROR errorCode = ECC_NO_ERROR;
    switch(value)
    {   
        case ECC_NO_ERROR:
        case ECC_OP_SIZE_ERROR:
        case ECC_OP_LOCATION_ERROR:
        case ECC_INVALID_OPERATION:
        case ECC_TRNG_ERROR:
        case ECC_HASH_ERROR:
        case ECC_INVALID_ECC_CURVE:
        case ECC_EXEC_BAD_ORDER:
        case ECC_EXEC_NON_Q_RESIDUE:
        case ECC_EXEC_COMPOSITE:
        case ECC_EXEC_NON_INVERTIBLE:
        case ECC_EXEC_INVALID_PARAM:
        case ECC_EXEC_INVALID_SIGN:
        case ECC_EXEC_NOT_IMPLEMENTED:
        case ECC_EXEC_INVALID_MOD:
        case ECC_EXEC_OUT_OF_RANGE:
        case ECC_EXEC_INVALID_MICCODE:
        case ECC_EXEC_POINT_NOT_ON_CURVE:
            errorCode = (ECC_ERROR)value;
            break;
        default:
            errorCode = ECC_UNKNOWN_ERROR;
            break;
    }
    return errorCode;
}

ECC_ERROR DRV_CRYPTO_PKE_CheckErrors(void)
{
    uint32_t stat = CAMPKSTAT.val;
    uint32_t errorCode = ECC_NO_ERROR;
    
    for (uint32_t i = 0; i < MAX_ERRORS; i++)
    {
        if ((stat & (((uint32_t) 0x1U) << i)) > 0U)
        {
            errorCode = ((uint32_t)SHIFT_ERROR_CODE | i);
            break;
        }
    }
    return lDRV_CRYPTO_PKE_GetErrorEnum(errorCode);
}

CRYPTO_PKE_RESULT DRV_CRYPTO_PKE_ConvertErrorToResult(ECC_ERROR error)
{
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_ERROR_FAIL;
    switch(error)
    {   
        case ECC_NO_ERROR:
            result = CRYPTO_PKE_RESULT_SUCCESS;
            break;
        case ECC_TRNG_ERROR:
            result = CRYPTO_PKE_RESULT_ERROR_RNG;
            break;
        case ECC_EXEC_POINT_NOT_ON_CURVE:
        case ECC_INVALID_ECC_CURVE:
            result = CRYPTO_PKE_RESULT_ERROR_CURVE;
        break;
        case ECC_OP_SIZE_ERROR:
        case ECC_OP_LOCATION_ERROR:
        case ECC_INVALID_OPERATION:
        case ECC_HASH_ERROR:
        case ECC_EXEC_BAD_ORDER:
        case ECC_EXEC_NON_Q_RESIDUE:
        case ECC_EXEC_COMPOSITE:
        case ECC_EXEC_NON_INVERTIBLE:
        case ECC_EXEC_INVALID_PARAM:
        case ECC_EXEC_INVALID_SIGN:
        case ECC_EXEC_NOT_IMPLEMENTED:
        case ECC_EXEC_INVALID_MOD:
        case ECC_EXEC_OUT_OF_RANGE:
        case ECC_EXEC_INVALID_MICCODE:
            result = CRYPTO_PKE_RESULT_ERROR_FAIL;
        break;
        default:
            result = CRYPTO_PKE_RESULT_ERROR_FAIL;
        break;
    }
    return result;
}

ECC_ERROR DRV_CRYPTO_PKE_SetVariableLocations(PKE_CONFIG* eccData)
{
    ECC_ERROR errorCode = ECC_NO_ERROR;

    switch (eccData->operation)
    {
        case ECDSA_SIGNATURE_GENERATION:
            errorCode = lDRV_CRYPTO_PKE_WriteLocation(ECDSA_D, eccData->operand1.data, eccData->operand1.size);

            if (errorCode == ECC_NO_ERROR)
            {
                uint8_t randomBytes[P521_size] = {0};
                TRNG_ERROR trngStatus = lDRV_CRYPTO_PKE_GetRandomNumber(randomBytes, sizeof (randomBytes));

                if (trngStatus == TRNG_NO_ERROR)
                {
                    errorCode = lDRV_CRYPTO_PKE_WriteLocation(ECDSA_K, randomBytes, eccData->operand2.size);
                }
                else
                {
                    errorCode = ECC_TRNG_ERROR;
                }
            }

            if (errorCode == ECC_NO_ERROR)
            {
                errorCode = lDRV_CRYPTO_PKE_WriteLocation(ECDSA_H, eccData->operand3.data, eccData->operand3.size);
            }
            break;

        case ECDSA_SIGNATURE_VERIFICATION:
            errorCode = lDRV_CRYPTO_PKE_WriteLocation(ECDSA_R, eccData->operand1.data, eccData->operand1.size);

            if (errorCode == ECC_NO_ERROR)
            {
                errorCode = lDRV_CRYPTO_PKE_WriteLocation(ECDSA_S, eccData->operand2.data, eccData->operand2.size);
            }

            if (errorCode == ECC_NO_ERROR)
            {
                errorCode = lDRV_CRYPTO_PKE_WriteLocation(ECDSA_H, eccData->operand3.data, eccData->operand3.size);
            }

            if (errorCode == ECC_NO_ERROR)
            {
                errorCode = lDRV_CRYPTO_PKE_WriteLocation(ECDSA_QX, eccData->publicKey.x, eccData->publicKey.size);
            }

            if (errorCode == ECC_NO_ERROR)
            {
                errorCode = lDRV_CRYPTO_PKE_WriteLocation(ECDSA_QY, eccData->publicKey.y, eccData->publicKey.size);
            }
            break;
		case ECDH_ECC_MULTIPLY:

			errorCode = lDRV_CRYPTO_PKE_WriteLocation(MODULUS_A, eccData->publicKey.x, eccData->publicKey.size);

			if (errorCode == ECC_NO_ERROR)
			{
				errorCode = lDRV_CRYPTO_PKE_WriteLocation(MODULUS_A + 1, eccData->publicKey.y, eccData->publicKey.size);
			}

			if (errorCode == ECC_NO_ERROR)
			{
				errorCode = lDRV_CRYPTO_PKE_WriteLocation(MODULUS_B, eccData->operand1.data, eccData->operand1.size);
			}
            break;
        default:
            errorCode = ECC_INVALID_OPERATION;
            break;
    }
    return errorCode;
}

void DRV_CRYPTO_PKE_ClearMemory(void)
{
    PKE_CONFIG eccData = {
        .operation = PKE_CLEAR_MEMORY,
        .operandSize = 0,
        .curve = 0,
    };

    DRV_CRYPTO_PKE_SetCommand(&eccData);

    lDRV_CRYPTO_PKE_StartEngine();
}

void DRV_CRYPTO_PKE_ClearMemory_NoWait(void)
{
    PKE_CONFIG eccData = {
        .operation = PKE_CLEAR_MEMORY,
        .operandSize = 0,
        .curve = 0,
    };
    
    DRV_CRYPTO_PKE_SetupEngine();

    DRV_CRYPTO_PKE_SetCommand(&eccData);

    (void)DRV_CRYPTO_PKE_StartEngine_NoWait();
}

void DRV_CRYPTO_PKE_SetupEngine(void)
{
	CAMCON.val |= CAMCON_VAL_INIT;
    /* cppcheck-suppress misra-c2012-11.4 */
    CAMPKMICOFF = (uint32_t) pkeMicrocode;
    /* cppcheck-suppress misra-c2012-11.4 */
    CAMPKMEMOFF = (uint32_t) &pkeRam;
    CAMPKOPSZ.val = PKE_OP_SIZE;
    CAMPKPTR.val = 0;
    CAMPKPTR.bits.OPPTRA = MODULUS_A;
    CAMPKPTR.bits.OPPTRB = MODULUS_B;
    CAMPKPTR.bits.OPPTRC = MODULUS_CX;
    CAMPKPTR.bits.OPPTRN = MODULUS_N;
}

ECC_ERROR DRV_CRYPTO_PKE_ReadLocation(uint8_t location, uint8_t* data, uint32_t dataLength)
{
    ECC_ERROR errorCode = ECC_NO_ERROR;

    if (dataLength > PKE_OP_OFFSET)
    {
        errorCode = ECC_OP_SIZE_ERROR;
    }
    else if (location > (uint8_t) MAX_LOCATION)
    {
        errorCode = ECC_OP_LOCATION_ERROR;
    }
    else
    {
        volatile uint8_t* address = pkeRam[location];
        lDRV_CRYPTO_PKE_CopyAndSwapEndianness(data, (void*)address, dataLength);
    }

    return errorCode;
}

CRYPTO_PKE_RESULT DRV_CRYPTO_PKE_GetCurveSize(PKE_ECC_CURVE eccCurve, PKE_ECC_SIZE *size)
{
    CRYPTO_PKE_RESULT result = CRYPTO_PKE_RESULT_SUCCESS;
    switch (eccCurve)
    {
        case P192:
            *size = P192_size;
            break;

        case P256:
            *size = P256_size;
            break;

        case P384:
            *size = P384_size;
            break;

        case P521:
            *size = P521_size;
            break;

        default:
            result = CRYPTO_PKE_RESULT_ERROR_CURVE;
            break;
    }
    return result;
}

ECC_ERROR DRV_CRYPTO_PKE_ExecuteCommand(PKE_CONFIG* eccData)
{
    ECC_ERROR errorCode = DRV_CRYPTO_PKE_SetVariableLocations(eccData);
        
    if (errorCode == ECC_NO_ERROR)
    {
        DRV_CRYPTO_PKE_SetCommand(eccData);
        lDRV_CRYPTO_PKE_StartEngine();
        errorCode = DRV_CRYPTO_PKE_CheckErrors();
    }

    return errorCode;
}

PKE_OPERATION_TYPE DRV_CRYPTO_PKE_OperationCompleteGet(void)
{
    PKE_OPERATION_TYPE operation = CRYPTO_PKE_OPERATION_UNKNOWN;
    switch(CAMPKCMD.bits.OPTYPE)
    {
        case CMD_ECDSA_SIGNATURE_GENERATION:
            operation = CRYPTO_PKE_OPERATION_SIGN;
            break;
        case CMD_ECDSA_SIGNATURE_VERIFICATION:
            operation = CRYPTO_PKE_OPERATION_VERIFY;
            break;
        default:
            operation = CRYPTO_PKE_OPERATION_UNKNOWN;
            break;
    }
    return operation;
}