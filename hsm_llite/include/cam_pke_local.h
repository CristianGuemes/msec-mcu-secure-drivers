/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_pke_local.h

  Summary:
    Hardware abstraction layer header.

  Description:
    This header file defined the functions and structures for the CAM pke
    hardware driver for Microchip microcontrollers equipped with a Crypto Accelerator Module.
**************************************************************************/

/*
Copyright (C) 2025, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

#ifndef CAM_PKE_LOCAL_H
#define	CAM_PKE_LOCAL_H

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "../inc/cam_pke.h"
#include "../inc/cam_device.h"

#if defined(UNIT_TESTING)
    #include "../scripts/testing/mocks/cam_hw_mock.h"
#elif defined(GENERIC_TARGET_CAM_05346)
    #include "sec_cam_05346_v2_07A0000_local.h"
#elif defined(GENERIC_TARGET_HSM_LITE_04777)
    #include "sec_hsm_lite_04777_v1_4F000000_local.h"
#elif defined(GENERIC_TARGET_CAM_06048)
    #include "sec_cam_06048_v3_07A0000_local.h"
#else
    #error "No TRNG_READ_ADDRESS mapping: define UNIT_TESTING or supported device macro."
#endif
// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

#ifndef __XC__
  #define space(x)
#endif

#define PKE_OP_SIZE                             (BITS_0209)
#define PKE_NUM_OP                              (32)
#define PKE_OP_OFFSET                           (0x80U)

#if defined(GENERIC_TARGET_CAM_06048)
#define PKE_MICROCODE_SIZE                      (1301)
#else
#define PKE_MICROCODE_SIZE                      (1128)
#endif

#define ENABLE                                  (0x1U)
#define MAX_ERRORS                              (0xEU)
#define SHIFT_ERROR_CODE                        (0x100U)
#define UNCOMPRESSED_TYPE                       (0x04U)

/*********** ECC Locations *****************/
#define ECDSA_D                                 (0x6)
#define ECDSA_K                                 (0x7)
#define ECDSA_QX                                (0x8)
#define ECDSA_QY                                (0x9)
#define ECDSA_R                                 (0xA)
#define ECDSA_S                                 (0xB)
#define ECDSA_H                                 (0xC)

/*********** Modulo Arithmetic Operation  ************/
#define MODULUS_A                               (0x6)
#define MODULUS_B                               (0x8)
#define MODULUS_CX                              (0xA)
#define MODULUS_CY                              (0xB)
#define MODULUS_N                               (0xC)

#define MAX_LOCATION                            (0xF)
#define DEFAULT_RESULT_LOCATION                 (0x99U)

#if defined(GENERIC_TARGET_CAM_05346) || defined(GENERIC_TARGET_CAM_06048)
extern const uint32_t pkeMicrocode[PKE_MICROCODE_SIZE] __attribute__((space(prog)));
#elif defined(GENERIC_TARGET_HSM_LITE_04777) 
extern const uint32_t* const pkeMicrocode;
#endif /*GENERIC_TARGET_CAM_05346*/
extern volatile uint8_t pkeRam[PKE_NUM_OP][PKE_OP_OFFSET];

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef enum
{
  P256_size = 256 / 8,
  P384_size = 384 / 8,
  P521_size = 528 / 8,
  P192_size = 192 / 8,
} PKE_ECC_SIZE;

typedef enum
{
  BITS_0100 = 256,
  BITS_0209 = 521,
  BITS_0800 = 2048,
  BITS_0C00 = 3072,
  BITS_1000 = 4096,
} PKE_OP_SIZE_ENUM;

typedef enum
{
  FIELD_PRIME = 0,
  FIELD_BINARY = 1,
} ECC_CMD_FIELD;

typedef enum
{
  EDWARDS_OFF = 0,
  EDWARDS_ON = 1,
} ECC_CMD_EDWARDS;

typedef enum
{
    CMD_DEFAULT = 0,
    CMD_PKE_CLEAR_MEMORY = 0xF,
    CMD_ECDSA_SIGNATURE_GENERATION = 0x30,
    CMD_ECDSA_SIGNATURE_VERIFICATION = 0x31,
    CMD_ECDH_ECC_MULTIPLY = 0x22,
} PKE_CMD_OPERATIONS;

typedef enum
{
    ECC_NO_ERROR = 0,
    ECC_OP_SIZE_ERROR = 1,
    ECC_OP_LOCATION_ERROR = 2,
    ECC_INVALID_OPERATION = 3,
    ECC_TRNG_ERROR = 4,
    ECC_HASH_ERROR = 5,
    ECC_INVALID_ECC_CURVE = 6,
    ECC_EXEC_BAD_ORDER = 0x10E,
    ECC_EXEC_NON_Q_RESIDUE = 0x10D,
    ECC_EXEC_COMPOSITE = 0x10C,
    ECC_EXEC_NON_INVERTIBLE = 0x10B,
    ECC_EXEC_INVALID_PARAM = 0x10A,
    ECC_EXEC_INVALID_SIGN = 0x109,
    ECC_EXEC_NOT_IMPLEMENTED = 0x108,
    ECC_EXEC_INVALID_MOD = 0x107,
    ECC_EXEC_OUT_OF_RANGE = 0x106,
    ECC_EXEC_INVALID_MICCODE = 0x105,
    ECC_EXEC_POINT_NOT_ON_CURVE = 0x104,
    ECC_UNKNOWN_ERROR = 0x105,
} ECC_ERROR;

typedef enum 
{
    CRYPTO_PKE_STATUS_IDLE = 0U,           /**< PKE is ready for a new operation */
    CRYPTO_PKE_STATUS_BUSY = 1U,           /**< PKE operation is in progress */
} CRYPTO_PKE_STATUS;

// *****************************************************************************
// *****************************************************************************
// Section: PKE Common Interface
// *****************************************************************************
// *****************************************************************************

/**
 *  @brief Read output data from the PKE engine.
 *  @param location Location to read the data from.
 *  @param data Variable to store the read in values from the location.
 *  @param dataLength Length of the data to read in.
 *  @return ECC_NO_ERROR on success.  ECC_OP_SIZE_ERROR or ECC_OP_LOCATION_ERROR on failure.
 **/
ECC_ERROR DRV_CRYPTO_PKE_ReadLocation(uint8_t location, uint8_t* data, uint32_t dataLength);

/**
 *  @brief Sets the size based on the given curve.
 *  @param eccCurve Curve type used to set the size.
 *  @param size Size of the curve represented by an enumeration.
 *  @return CRYPTO_PKE_RESULT_SUCCESS on success.  CRYPTO_PKE_RESULT_ERROR_FAIL on failure.
 **/
CRYPTO_PKE_RESULT DRV_CRYPTO_PKE_GetCurveSize(PKE_ECC_CURVE eccCurve, PKE_ECC_SIZE *size);

/**
 *  @brief Starts the execution of the PKE engine with given configurations and data.
 *  @param eccData Configuration structure for PKE information.
 *  @return ECC_NO_ERROR on success.  All other possible ECC_ERROR values on failure.
 **/
ECC_ERROR DRV_CRYPTO_PKE_ExecuteCommand(PKE_CONFIG* eccData);

/**
 *  @brief This function starts the PKE engine with the initialized parameters without waiting for completion.
 **/
void DRV_CRYPTO_PKE_StartEngine_NoWait(void);

/**
 *  @brief Sets the command and operation details to the PKE engine.
 *  @param eccData Configuration structure to store PKE information.
 **/
void DRV_CRYPTO_PKE_SetCommand(const PKE_CONFIG* eccData);

/**
 *  @brief Check the CAMPKSTAT register if an status error is present.
 *  @return ECC_NO_ERROR on success. Any other ECC_ERROR status on failure.
 **/
ECC_ERROR DRV_CRYPTO_PKE_CheckErrors(void);

/**
 *  @brief Sets up and writes the operands to the expected locations for the PKE engine.
 *  @param eccData Configuration structure to store PKE information.
 *  @return ECC_NO_ERROR on success. ECC_INVALID_OPERATION, ECC_OP_SIZE_ERROR or ECC_OP_LOCATION_ERROR on failure.
 **/
ECC_ERROR DRV_CRYPTO_PKE_SetVariableLocations(PKE_CONFIG* eccData);

/**
 *  @brief Converts the ECC_ERROR enum to the CRYPTO_PKE_RESULT output
 *  @param error Configuration structure to store PKE information.
 *  @return CRYPTO_PKE_RESULT that corresponds to the ECC_ERROR type.
 **/
CRYPTO_PKE_RESULT DRV_CRYPTO_PKE_ConvertErrorToResult(ECC_ERROR error);

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif // CAM_PKE_LOCAL_H
