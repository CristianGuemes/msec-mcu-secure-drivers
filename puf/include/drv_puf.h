/*******************************************************************************
* Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

/**
 * PUF Driver Header File
 *
 * @file drv_puf.h
 *
 * @ingroup puf
 *
 * @brief This file contains the API implementations for the Physical Unclonable 
  *       Function (PUF) driver.
 *
 * @version PUF Driver Version 1.0.0
 *
 */

#ifndef DRV_PUF_H
#define DRV_PUF_H

#include <xc.h>
#include <stdbool.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
extern "C" {
#endif
// DOM-IGNORE-END

/**
 * @ingroup puf
 * @def DRV_PUF_ACTIVATION_CODE_SIZE
 * @brief This macro defines the size in bytes of activation code.
 */
#define DRV_PUF_ACTIVATION_CODE_SIZE (996)

/**
 * @ingroup puf
 * @def DRV_PUF_KEY_CONTEXT_SIZE
 * @brief This macro defines the size in bytes of key context.
 */
#define DRV_PUF_KEY_CONTEXT_SIZE (16)

/**
 * @ingroup puf
 * @def RND_CONTEXT_SIZE
 * @brief This macro defines the size in bytes of context for random generation.
 */
#define DRV_PUF_RND_CONTEXT_SIZE (4)

/**
 * @ingroup puf
 * @def DRV_PUF_KEY_CODE_SIZE(keyLenBits)
 * @brief This macro calculates the size in bytes of the key code for a given key length in bits.
 *
 * The size is computed as:
 *   36 bytes (base) +
 *   (keyLenBits / 8) bytes (key length in bytes) +
 *   16 bytes for each 384 bits in the key length (rounded up).
 *
 * @param keyLenBits Key length in bits.
 */
#define DRV_PUF_KEY_CODE_SIZE(keyLenBits) \
    ((uint32_t)(36U + ((uint32_t)(keyLenBits) / 8U) + \
    (16U * ((uint32_t)(((uint32_t)(keyLenBits) + 383U) / 384U)))))

/**
 * @ingroup puf
 * @typedef drv_puf_result_t
 * @brief Enumeration of the result codes for PUF operations and register management.
 */
typedef enum
{
    DRV_PUF_RES_OK                        = 0x00U, /**< Last operation was successful or operation is in progress */
    DRV_PUF_RES_REGISTER_ERR              = 0x01U, /**< Error in register management */
    DRV_PUF_RES_GENERIC_ERR               = 0xD0U, /**< A generic error occurred */
    DRV_PUF_RES_PARAM_ERR                 = 0xD1U, /**< Parameter to function is not correct, e.g. null pointer */
    DRV_PUF_RES_CMD_NOT_ALLOWED_ERR       = 0xD2U, /**< Command is not allowed */
    DRV_PUF_RES_CMD_REJECTED_ERR          = 0xD3U, /**< Command was rejected */
    DRV_PUF_RES_BIST_ERR                  = 0xD4U, /**< BIST failed */
    DRV_PUF_RES_TRANSFER_ABORTED_ERR      = 0xD5U, /**< Transfer of data input or output aborted */
    DRV_PUF_RES_WRONG_AC_ERR              = 0xF0U, /**< The provided AC is not for the current product/version */
    DRV_PUF_RES_WRONG_AC_PH2_ERR          = 0xF1U, /**< The AC in the second phase is not for the current product/version */
    DRV_PUF_RES_AC_CORRUPTED_ERR          = 0xF2U, /**< The provided AC is corrupted */
    DRV_PUF_RES_AC_CORRUPTED_PH2_ERR      = 0xF3U, /**< The AC in the second phase is corrupted */
    DRV_PUF_RES_AC_AUTH_FAILED_ERR        = 0xF4U, /**< Authentication of the provided AC failed */
    DRV_PUF_RES_AC_AUTH_FAILED_PH2_ERR    = 0xF5U, /**< Authentication of the provided AC failed in the second phase */
    DRV_PUF_RES_QUALITY_ERR               = 0xF6U, /**< PUF quality verification fails */
    DRV_PUF_RES_CONTEXT_ERR               = 0xF7U, /**< An incorrect or unsupported context is provided */
    DRV_PUF_RES_DESTINATION_ERR           = 0xF8U, /**< A data destination was set that is not allowed */
    DRV_PUF_RES_FAILURE_SRAM_ERR          = 0xFFU, /**< PUF SRAM access has failed */
} drv_puf_result_t;

/**
 * @ingroup puf
 * @typedef drv_puf_key_scope_t
 * @brief Enumeration of key scopes for Enrolled and Started states.
 */
typedef enum
{
    DRV_PUF_KEY_SCOPE_NONE           = 0x00U, /**< Key cannot go to any output */
    DRV_PUF_KEY_SCOPE_REG            = 0x01U, /**< Key can go to the Data Output register */
    DRV_PUF_KEY_SCOPE_SO             = 0x02U, /**< Key can go to the Secure Output interface */
    DRV_PUF_KEY_SCOPE_NOT_RESTRICTED = 0x03U  /**< Key can go to any output */
} drv_puf_key_scope_t;

/**
 * @ingroup puf
 * @typedef drv_puf_data_destination_t
 * @brief Enumeration of data destinations.
 */
typedef enum
{
    DRV_PUF_DATA_DEST_NONE  = 0x00U, /**< No data destination selected */
    DRV_PUF_DATA_DEST_REG   = 0x01U, /**< Data destination is the Data Output register */
    DRV_PUF_DATA_DEST_SO    = 0x02U  /**< Data destination is the dedicated Secure Output interface */
} drv_puf_data_destination_t;

/**
 * @ingroup puf
 * @typedef drv_puf_key_index_t
 * @brief Enumeration of PUF key indexes.
 */
typedef enum
{
    DRV_PUF_KEY_INDEX_RSVD_NO_DEST        = 0x00U, /**< Reserved, no destination */
    DRV_PUF_KEY_INDEX_AES_EXTKEY_1        = 0x01U, /**< Flash key */
    DRV_PUF_KEY_INDEX_AES_EXTKEY_2        = 0x02U, /**< Reserved for future use in AES engine */
    DRV_PUF_KEY_INDEX_SHA                 = 0x03U, /**< UDS for DICE */
    DRV_PUF_KEY_INDEX_INT_DECRYPT         = 0x04U, /**< Internal code execution decryption engine */
    DRV_PUF_KEY_INDEX_EXT_DECRYPT         = 0x05U, /**< External code execution decryption engine */
    DRV_PUF_KEY_INDEX_RSVD_1              = 0x06U, /**< Reserved for future use */
    DRV_PUF_KEY_INDEX_RSVD_2              = 0x07U, /**< Reserved for future use */
    DRV_PUF_KEY_INDEX_RSVD_DEV_1          = 0x08U, /**< Reserved for Device Level Definition */
    DRV_PUF_KEY_INDEX_RSVD_DEV_2          = 0x09U, /**< Reserved for Device Level Definition */
    DRV_PUF_KEY_INDEX_RSVD_DEV_3          = 0x0AU, /**< Reserved for Device Level Definition */
    DRV_PUF_KEY_INDEX_RSVD_DEV_4          = 0x0BU, /**< Reserved for Device Level Definition */
    DRV_PUF_KEY_INDEX_RSVD_DEV_5          = 0x0CU, /**< Reserved for Device Level Definition */
    DRV_PUF_KEY_INDEX_RSVD_DEV_6          = 0x0DU, /**< Reserved for Device Level Definition */
    DRV_PUF_KEY_INDEX_RSVD_DEV_7          = 0x0EU, /**< Reserved for Device Level Definition */
    DRV_PUF_KEY_INDEX_RSVD_DEV_8          = 0x0FU  /**< Reserved for Device Level Definition */
} drv_puf_key_index_t;

/**
 * @ingroup puf
 * @typedef drv_puf_user_ctx_t
 * @brief Enumeration of PUF user contexts.
 */
typedef enum
{
	DRV_PUF_USER_CONTEXT_0   = 0x00U, /**< User context 0 */
	DRV_PUF_USER_CONTEXT_1   = 0x01U  /**< User context 1 */
} drv_puf_user_ctx_t;

/**
 * @ingroup puf
 * @typedef struct drv_puf_init_t
 * @brief Structure for the initialization of PUF.
 */
typedef struct
{
    uint8_t reserved;         /**< Reserved for future use. */
} drv_puf_init_t;

/**
 * @ingroup puf
 * @typedef struct drv_puf_start_t
 * @brief Structure for the start of PUF.
 */
typedef struct
{
    uint32_t            initCmdDisableMask;  /**< DISABLE register settings - this covers all commands */
    uint32_t            initSramPwrUpTime;   /**< SRAM Power Up Time */
    bool                initSramPwrSwEnable; /**< SRAM Power Switch Enable */
    bool                initPrivAccess;      /**< Privileged Access Only Enable */
} drv_puf_start_t;

/**
 * @ingroup puf
 * @brief Initializes the PUF driver.
 * @param pufRegPtr Pointer to the PUF initialization structure
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 */
drv_puf_result_t DRV_PUF_Initialize(void);

/**
 * @ingroup puf
 * @brief Starts the PUF driver.
 * @param pufStartConfig Pointer to the PUF start structure
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized to run this function successfully.
 */
drv_puf_result_t DRV_PUF_Start(const drv_puf_start_t *const pufStartConfig);

/**
 * @ingroup puf
 * @brief Enables the PUF clock.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_Enable(void);

/**
 * @ingroup puf
 * @brief Disables the PUF clock and holds in reset.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_Disable(void);

/**
 * @ingroup puf
 * @brief Software resets registers and internal state of PUF.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_SoftReset(void);

/**
 * @ingroup puf
 * @brief Enables SRAM Power Switch.
 * @param onTime Time in clock cycles for SRAM power-on duration
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_SramPwrSwitchEnable(uint32_t onTime);

/**
 * @ingroup puf
 * @brief Disables SRAM Power Switch.
 * @param offTime Time in clock cycles for SRAM power-off duration
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_SramPwrSwitchDisable(uint32_t offTime);

/**
 * @ingroup puf
 * @brief Enables the PUF Privileged Access mode.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_PrivilegedAccessEnable(void);

/**
 * @ingroup puf
 * @brief Disables the PUF Privileged Access mode.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_PrivilegedAccessDisable(void);

/**
 * @ingroup puf
 * @brief Restricts the PUF user context.
 * @param ctxNum Context number (0 or 1)
 * @param ctxVal Context value (word)
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_RestrictContext(drv_puf_user_ctx_t ctxNum, uint32_t ctxVal);

/**
 * @ingroup puf
 * @brief Disables the PUF commands.
 * @param cmdMask Command disable mask: 
 * Bit 31: Test PUF
 * Bit 30: Test Memory
 * Bit 29: Skip Test Memory
 * Bit 15: Generate Random
 * Bit 9: Wrap
 * Bit 8: Wrap Generate Random
 * Bit 7: Unwrap
 * Bit 6: Get Key
 * Bit 5: Stop
 * Bit 3: Reconstruct
 * Bit 2: Start
 * Bit 1: Enroll
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_DisableCommand(uint32_t cmdMask);

/**
 * @ingroup puf
 * @brief Checks which commands have been disabled.
 * @param cmdMask Pointer to disabled command mask:
 * Bit 31: Test PUF
 * Bit 30: Test Memory
 * Bit 29: Skip Test Memory
 * Bit 15: Generate Random
 * Bit 9: Wrap
 * Bit 8: Wrap Generate Random
 * Bit 7: Unwrap
 * Bit 6: Get Key
 * Bit 5: Stop
 * Bit 3: Reconstruct
 * Bit 2: Start
 * Bit 1: Enroll
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
drv_puf_result_t DRV_PUF_CheckDisableCommand(uint32_t *cmdMask);

/**
 * @ingroup puf
 * @brief Checks if PUF is busy.
 * @param None.
 * @retval TRUE Write protect is locked
 * @retval FALSE Write protect is not locked
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 */
bool DRV_PUF_IsBusy(void);

/**
 * @ingroup puf
 * @brief Clears the PUF Key.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized, not necessarily started, to run this function successfully.
 * @note The clearing only applies to the Secure Output.
 */
drv_puf_result_t DRV_PUF_ClearKey(void);

/**
 * @ingroup puf
 * @brief Performs the Enroll command.
 * @param activationCode Pointer to the byte array that will hold the generated activation code
 * @param pufScore Pointer to the byte that will hold the PUF score
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized and started to run this command successfully.
 */
drv_puf_result_t DRV_PUF_EnrollCmd(uint8_t * const activationCode, uint8_t *pufScore);

 /**
 * @ingroup puf
 * @brief Performs the Start command.
 * @param activationCode Pointer to the byte array that holds the activation code
 * @param pufScore Pointer to the byte that will hold the PUF score
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The PUF must have been enrolled or the driver must have been initialized and started (after a 
  *      previous enrollment) to run this command successfully.
 */
drv_puf_result_t DRV_PUF_StartCmd(const uint8_t * const activationCode, uint8_t *pufScore);

 /**
 * @ingroup puf
 * @brief Performs the Reconstruct command.
 * @param activationCode Pointer to the byte array that holds the activation code
 * @param pufScore Pointer to the byte that will hold the PUF score
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The PUF must have been enrolled or the driver must have been initialized and started (after a 
  *      previous enrollment) to run this command successfully.
 */
drv_puf_result_t DRV_PUF_ReconstructCmd(const uint8_t * const activationCode, uint8_t *pufScore);

 /**
 * @ingroup puf
 * @brief Performs the Stop command.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The PUF must have been enrolled or started/reconstructed to run this command successfully.
 */
drv_puf_result_t DRV_PUF_StopCmd(void);

 /**
 * @ingroup puf
 * @brief Creates the user context 0 for a key operation when the PUF key index is provided.
 * @param keyIndex PUF key index
 * @param userContext0 User context word 0 with bits 3-0 empty
 * @param combinedUserContext0 Pointer to word that will hold the user context word 0 with the key index
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 */
drv_puf_result_t DRV_PUF_CreateUserContext0(drv_puf_key_index_t keyIndex,
        uint32_t userContext0, uint32_t *combinedUserContext0);

 /**
 * @ingroup puf
 * @brief Performs the Get Key command.
 * @param keyLenBits Length of the key in bits. Accepted values: Till 1024: Multiples of 64; and 2048, 3072, 4096.
 * @param keyScopeStarted Key scope in Started state
 * @param keyScopeEnrolled Key scope in Enrolled state
 * @param userContext0 User context word 0
 * @param userContext1 User context word 1
 * @param keyDest Key destination
 * @param key Pointer to the byte array that will hold the key
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The PUF must have been enrolled or started/reconstructed to run this command successfully.
 */
drv_puf_result_t DRV_PUF_GetKeyCmd(uint16_t keyLenBits,
        drv_puf_key_scope_t keyScopeStarted, drv_puf_key_scope_t keyScopeEnrolled,
        uint32_t userContext0, uint32_t userContext1,
        drv_puf_data_destination_t keyDest, uint8_t *key);

/**
 * @ingroup puf
 * @brief Performs the Wrap Generated Random command.
 * @param keyLenBits Length of the key in bits. Accepted values: Till 1024: Multiples of 64; and 2048, 3072, 4096.
 * @param keyScopeStarted Key scope in Started state
 * @param keyScopeEnrolled Key scope in Enrolled state
 * @param userContext0 User context word 0
 * @param userContext1 User context word 1
 * @param keyCode Pointer to the byte array that will hold the key code. Size of the array must be DRV_PUF_KEY_CODE_SIZE(keyLenBits).
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The PUF must have been enrolled or started/reconstructed to run this command successfully.
 */
drv_puf_result_t DRV_PUF_WrapGeneratedRandomCmd(uint16_t keyLenBits,
        drv_puf_key_scope_t keyScopeStarted, drv_puf_key_scope_t keyScopeEnrolled,
        uint32_t userContext0, uint32_t userContext1, uint8_t *keyCode);

/**
 * @ingroup puf
 * @brief Performs the Wrap command.
 * @param keyLenBits Length of the key in bits. Accepted values: Till 1024: Multiples of 64; and 2048, 3072, 4096.
 * @param keyScopeStarted Key scope in Started state
 * @param keyScopeEnrolled Key scope in Enrolled state
 * @param userContext0 User context word 0
 * @param userContext1 User context word 1
 * @param key Pointer to the byte array that holds the key to be wrapped
 * @param keyCode Pointer to the byte array that will hold the key code. Size of the array must be DRV_PUF_KEY_CODE_SIZE(keyLenBits).
 * @note The PUF must have been enrolled or started/reconstructed to run this command successfully.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 */
drv_puf_result_t DRV_PUF_WrapCmd(uint16_t keyLenBits,
        drv_puf_key_scope_t keyScopeStarted, drv_puf_key_scope_t keyScopeEnrolled,
        uint32_t userContext0, uint32_t userContext1, const uint8_t *key, uint8_t *keyCode);

/**
 * @ingroup puf
 * @brief Performs the Unwrap command.
 * @param keyCode Pointer to the byte array that holds the key code
 * @param keyDest Key destination
 * @param key Pointer to the byte array that will hold the key
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The PUF must have been enrolled or started/reconstructed to run this command successfully.
 */
drv_puf_result_t DRV_PUF_UnwrapCmd(const uint8_t *keyCode,
        drv_puf_data_destination_t keyDest, uint8_t *key);

/**
 * @ingroup puf
 * @brief Performs the Generate Random command.
 * @param rndDataLenBits Length of the random data in bits. Accepted values: Till 1024: Multiples of 64; and 2048, 3072, 4096.
 * @param rndDataDest Random data destination
 * @param rndData Pointer to the byte array that will hold the random data
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized and started to run this command successfully.
 */
drv_puf_result_t DRV_PUF_GenerateRandomCmd(uint16_t rndDataLenBits,
        drv_puf_data_destination_t rndDataDest, uint8_t *rndData);

/**
 * @ingroup puf
 * @brief Performs the Test Memory command.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The PUF must have been stopped or the driver must have been initialized and started to run this command successfully.
 */
drv_puf_result_t DRV_PUF_TestMemoryCmd(void);

/**
 * @ingroup puf
 * @brief Performs the Test PUF command.
 * @param pufScore Pointer to the byte that will hold the PUF score
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized and started to run this command successfully. No other command must have been run before.
 */
drv_puf_result_t DRV_PUF_TestPufCmd(uint8_t *pufScore);

/**
 * @ingroup puf
 * @brief Performs the Zeroize command.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The driver must have been initialized to run this operation successfully.
 */
drv_puf_result_t DRV_PUF_Zeroize(void);

/**
 * @ingroup puf
 * @brief Performs the BIST command.
 * @param None.
 * @return Result code of the operation. See @ref drv_puf_result_t enum for details.
 * @note The PUF must have been stopped or the driver must have been initialized and started to run this command successfully.
 */
drv_puf_result_t DRV_PUF_Bist(void);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
    }
#endif
// DOM-IGNORE-END

#endif // DRV_PUF_H
