/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_dma_local.h

  Summary:
    Crypto Framework Library interface file for CAM DMA.

  Description:
    This header file contains the interface that make up the CAM DMA hardware
    driver for Microchip microcontrollers equipped with a Crypto Accelerator Module.
**************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END

#ifndef CAM_DMA_LOCAL_H
#define	CAM_DMA_LOCAL_H

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdbool.h>
#include <stdint.h>

// *****************************************************************************
// *****************************************************************************
// Section: DMA Common Macros
// *****************************************************************************
// *****************************************************************************

#define IS_EQUAL_TO_OR_BETWEEN_VALUES(x, a, b) (((x) >= (a)) && ((x) <= (b)))

// *****************************************************************************
// *****************************************************************************
// Section: DMA Common Data Types
// *****************************************************************************
// *****************************************************************************

#define DMA_CONTEXT_INIT_PATTERN          (uint32_t)0xd3a3a31cUL

#define DMA_TIMEOUT_LIMIT        (int32_t)0xFFFFU

#define MAX_ADDRESS_LENGTH       (uint32_t)0xFFFFFFFU
#define ADDRESS_LENGTH_MASK      (uint32_t)0x0FFFFFFFU

#define CONFIG_OFFSET            (0x00U)  // Configuration mode offset

#define AES_KEY_OFFSET           (0x08U)
#define AES_IV_OFFSET            (0x28U)
#define AES_KEY2_OFFSET          (0x48U)  // For use of a second key with AES (XTS only).
#define AES_MASK_OFFSET          (0x68U)  // Initial value for LFSR (used for countermeasure)

#define HASH_DATATYPE_LIMIT      (5U)
#define AES_DATATYPE_LIMIT       (2U)

#define DMA_LAST_TAG_OFFSET_SHIFT       (5U)
#define DMA_DATATYPE_TAG_OFFSET_SHIFT   (6U)
#define DMA_CONFIG_TAG_OFFSET_SHIFT     (8U)
#define DMA_INVALID_BYTES_OFFSET_SHIFT  (8U)
#define DMA_REALIGN_OFFSET_SHIFT        (29U)
#define DMA_DISCARD_OFFSET_SHIFT        (30U)


#define DMA_CONFIG_TAG(offset)      ((uint32_t)(offset) << DMA_CONFIG_TAG_OFFSET_SHIFT)
#define DMA_DATATYPE_TAG(datatype)  ((uint32_t)(datatype) << DMA_DATATYPE_TAG_OFFSET_SHIFT)
#define DMA_DATCON_TAG              (1UL << 4)
#define DMA_TAG_LAST(last)          ((uint32_t)(last) << DMA_LAST_TAG_OFFSET_SHIFT)
#define DMA_TAG_IGNOREBYTES(size)   ((size) << DMA_INVALID_BYTES_OFFSET_SHIFT)

#define DMA_REALIGN                 (1UL << DMA_REALIGN_OFFSET_SHIFT)
#define DMA_DISCARD                 (1UL << DMA_DISCARD_OFFSET_SHIFT)

#define DMA_BUSY_MASK               (uint32_t)(0x3)   // FETCHBUSY | PUSHBUSY
#define DMA_INT_ERR_MASK            (uint32_t)(0x24)  // FERRINTSTATR | PERRINTSTATR

#define DMA_FETCHSTRT               (uint32_t)(1U << 0)
#define DMA_PUSHSTRT                (uint32_t)(1U << 1)

#define DMA_IS_NOT_BUSY             (0U)
#define DMA_IS_BUSY                 (1U)

#define DMA_IS_NOT_RUNNING          (0U)
#define DMA_IS_RUNNING              (1U)

// The maximum number of available hardware descriptors.
#define DMA_IN_DESCRIPTOR_COUNT     (7U)
#define DMA_OUT_DESCRIPTOR_COUNT    (5U)

// Tells the DMA engine to stop processing the chain (DMAFETCHBITS.STOP or DMAPUSHBITS.STOP).
#define DMA_LAST_DESCRIPTOR         ((DMA_DESCRIPTOR *)1UL)

// Defines DMA operations state.
#define DMA_IDLE    (0UL)
#define DMA_BUSY    (1UL)
#define DMA_RUNNING (2UL)

// Indicates this engine is used for AES operations.
#define ENGINE_IS_AES(engine) ((uint32_t)(engine) == (uint32_t)TAG_ENGINE_AES)
#define ENGINE_IS_HASH(engine) ((uint32_t)(engine) == (uint32_t)TAG_ENGINE_HASH)

/**
 * @brief DMA operations modes.
 */
typedef enum CAM_DMA_MODE
{
  CAM_DMA_MODE_DIRECT = 0,
  CAM_DMA_MODE_DESCRIPTOR = 1,
} CAM_DMA_MODE;

/**
 *  @brief DMA driver errors.
 **/
typedef enum CAM_DMA_ERROR
{
    CAM_DMA_NO_ERROR = 0,
    CAM_DMA_DATA_LENGTH_BEYOND_LIMIT = 1,
    CAM_DMA_TAG_INVALID_DATATYPE = 2,
    CAM_DMA_TAG_INVALID_OFFSET = 3,
    CAM_DMA_TAG_UNIMPLEMENTED_ENGINE_SELECTED = 4,
    CAM_DMA_TAG_INVALID_FOR_THIS_ENGINE = 5,
    CAM_DMA_FETCH_TIMEOUT_ERROR = 6,
    CAM_DMA_PUSH_TIMEOUT_ERROR = 7,
    CAM_DMA_RESET_FAIL_ERROR = 8,
    CAM_DMA_GENERAL_ERROR = 9,
    CAM_DMA_INIT_ERROR = 10,
    CAM_DMA_BUSY_ERROR = 11,
    CAM_DMA_RUNNING_ERROR = 12,
    CAM_DMA_EXECUTE_ERROR = 13,
    CAM_DMA_ALL_DESCRIPTORS_USED = 14,
    CAM_DMA_CONTEXT_ERROR = 15,
    CAM_DMA_CONTEXT_ALIGNMENT_ERROR = 16,
    CAM_DMA_INVALID_MODE = 17,
    CAM_DMA_TRY_AGAIN_LATER = 18
} CAM_DMA_ERROR;

/**
 *  @brief Select the DMA engine mode.
 **/
typedef enum TAG_ENGINE_SELECTION
{
    TAG_ENGINE_DEFAULT = 0,
    TAG_ENGINE_BYPASS = 0,
    TAG_ENGINE_AES = 1,
    TAG_ENGINE_HASH = 3,

    #if defined(GENERIC_TARGET_CAM_06048)

    TAG_ENGINE_HASH_SHA3 = 5,

    #endif
} TAG_ENGINE_SELECTION;

/**
 *  @brief Defines the 'last descriptor' tag setting.
 **/
typedef enum TAG_LAST
{
    TAG_NOT_LAST = 0,
    TAG_IS_LAST = 1,
} TAG_LAST;

/**
 *  @brief Defines the data type for the DMA to process.
 **/
typedef enum TAG_DATATYPE
{
    TAG_AES_PAYLOAD = 0,
    TAG_AES_HEADER = 1,
    TAG_HASH_MESSAGE = 0,
    TAG_HASH_IV = 1,
    TAG_HASH_HMAC_KEY = 2,
    TAG_HASH_REF_HASH = 3,
} TAG_DATATYPE;

// *****************************************************************************
// *****************************************************************************
// Section: Data Definitions
// *****************************************************************************
// *****************************************************************************

/**
 *  @brief The configuration data format for SHA1/SHA2 HASH operations.
 **/
typedef volatile struct HASH_CONFIG
{
    uint32_t MODE    : 7;
    uint32_t         : 1;
    uint32_t HMAC    : 1;
    uint32_t PADDING : 1;
    uint32_t FINAL   : 1;
    uint32_t         :21;
} HASH_CONFIG;


#if defined(GENERIC_TARGET_CAM_06048)

/**
 *  @brief The configuration data format for SHA3/SHAKE HASH operations.
 *  @note SAVECONTEXT in the BA418 SHA3 engine:
 *        The polarity is inverted compared to the BA413 FINAL bit.
 **/
typedef volatile struct HASH_CONFIG_SHA3 {
    uint32_t MODE           : 4;
    uint32_t SHAKE_ENABLE   : 1;
    uint32_t PADDING        : 1;
    uint32_t SAVE_CONTEXT   : 1;
    uint32_t                : 1;
    uint32_t DIGEST_LENGTH  : 16;
    uint32_t                : 5;
    uint32_t HMAC           : 1;
    uint32_t                : 2;
} HASH_CONFIG_SHA3;

#endif

/**
 *  @brief The configuration data format for AES operations.
 **/
typedef volatile struct AES_CONFIG
{
    uint32_t ENCDEC : 1;
    uint32_t        : 3;
    uint32_t CXLOAD : 1;
    uint32_t CXSAVE : 1;
    uint32_t        : 2;
    uint32_t MODE   : 9;
    uint32_t        : 15;
} AES_CONFIG;

/**
 * @brief The AES key to use.
 */
typedef enum AES_KEY_NUMBER
{
    KEY1 = 0,
    KEY2 = 1,
} AES_KEY_NUMBER;

/**
 *  @brief Defines a DMA descriptor.
 *  @note The structure of this descriptor is defined in the BA450_CryptoMaster datasheet.
 **/
 typedef struct DMA_DESCRIPTOR
{
    volatile uint32_t const *address;   // Address for input/output buffer.

    struct DMA_DESCRIPTOR *next;        // Pointer to the next descriptor.
                                        // The last in the chain will have the LSB set to
                                        // tell the hardware to 'stop'.

    uint32_t size;                      // Size of the data in bytes.

    uint32_t tag;                       // Hardware tag to apply.

} DMA_DESCRIPTOR;

/**
 *  @brief Defines DMA operations management data.
 **/
typedef struct DMA_CONTEXT
{
    // Identifies this as a DMA context block.
    uint32_t signature;

    // Current DMA descriptor array offsets.
    // This is used when the DMA is in 'descriptor' mode.
    uint32_t inOffset  : 8;
    uint32_t outOffset : 8;

    // The engine to use for this DMA operation.
    uint32_t engine    : 3;

    // DMA state (idle/busy/running).
    // "Busy" starts when the transaction is initialized, indicating a DMA
    // operation is being configured.
    // "Running" is set after "busy" and means the hardware is executing.
    uint32_t state     : 2;

    // DMA mode (direct/descriptor).
    //  0 = Direct mode (manipulate hardware directly)
    //  1 = Descriptor mode (use descriptor chaining)
    uint32_t mode      : 1;

    // Unused for future needs.
    uint32_t           : 10;

    // The descriptor data (address/size/etc).
    // This is used when the DMA is in 'descriptor' mode.
    DMA_DESCRIPTOR inDescriptors[DMA_IN_DESCRIPTOR_COUNT];
    DMA_DESCRIPTOR outDescriptors[DMA_OUT_DESCRIPTOR_COUNT];

} DMA_CONTEXT;

#define DMA_CONTEXT_SIZE  sizeof(DMA_CONTEXT)

// *****************************************************************************
// *****************************************************************************
// Section: CAM DMA Function Definitions
// *****************************************************************************
// *****************************************************************************

/**
 *  @brief Enable the DMA engine.
 **/
void DRV_CRYPTO_DMA_Enable(void);

/**
 *  @brief Reset the CAM DMA engine.
 *  @note This function must be called before DRV_CRYPTO_DMA_Initialize()
 *         or if there is an error during operation setup.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_Reset(void);

// *****************************************************************************
// Section: CAM DMA Direct Mode Operations
// *****************************************************************************

/**
 *  @brief Initialize the CAM DMA engine.
 *  @param contextData Pointer to the DMA context space.
 *  @param mode the DMA operations mode (direct/descriptor).
 *  @param engine The tag engine type.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 *  @note The context must be large enough to hold the data for the given mode.
 *        This function is called before any tag/address operations.
 *        This function should be called immediately after DRV_CRYPTO_DMA_Reset().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_Initialize(DMA_CONTEXT * const contextData, CAM_DMA_MODE mode, TAG_ENGINE_SELECTION engine);

/**
 *  @brief Set direct-mode DMA write ("fetch") engine ("tag") data operation parameters.
 *  @param contextData Pointer to the DMA context space.
 *  @param last When TAG_LAST, tells the engine this is the last buffer to be written.
 *  @param dataType The data type of an AES or HASH operation.
 *  @param invalidByteCount the number of invalid bytes (bytes to ignore) for padding to a block size.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 *  @note This must be preceded by a call to DRV_CRYPTO_DMA_Init().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_SetTagData(DMA_CONTEXT * const contextData, TAG_LAST last, TAG_DATATYPE dataType, uint8_t invalidByteCount);

/**
 *  @brief Set direct-mode DMA write ("fetch") engine ("tag") config operation parameters.
 *  @param contextData Pointer to the DMA context space.
 *  @param last When TAG_LAST, tells the engine this is the last buffer to be written.
 *  @param offset The offset in bytes for the configuration data.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 *  @note This must be preceded by a call to DRV_CRYPTO_DMA_Init().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_SetTagConfig(DMA_CONTEXT * const contextData, TAG_LAST last, uint32_t offset);

/**
 *  @brief Set the start address of a direct-mode DMA write ("fetch") operation.
 *  @param contextData Pointer to the DMA context space.
 *  @param address The uint32_t address of the buffer into which to write ("fetch") the data.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 *  @note This must be preceded by a call to DRV_CRYPTO_DMA_SetTagConfig() or DRV_CRYPTO_DMA_SetData().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_SetFetchAddress(DMA_CONTEXT * const contextData, uint32_t address);

/**
 *  @brief Set the size of the direct-mode DMA operation data to write ("fetch").
 *  @param contextData Pointer to the DMA context space.
 *  @param length The length of the data to write in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 *  @note This must be preceded by a call to DRV_CRYPTO_DMA_SetTagConfig() or DRV_CRYPTO_DMA_SetData().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_SetFetchLength(DMA_CONTEXT * const contextData, uint32_t length);

/**
 *  @brief Set the start address of a direct-mode DMA read ("push") operation.
 *  @param contextData Pointer to the DMA context space.
 *  @param address The uint32_t address of the buffer into which to read ("push") the data.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 *  @note This must be preceded by a call to DRV_CRYPTO_DMA_SetTagConfig() or DRV_CRYPTO_DMA_SetData().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_SetPushAddress(DMA_CONTEXT * const contextData, uint32_t address);

/**
 *  @brief Set the size of the direct-mode DMA operation data to read ("push").
 *  @param contextData Pointer to the DMA context space.
 *  @param length The length of the data to read in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 *  @note This must be preceded by a call to DRV_CRYPTO_DMA_SetTagConfig() or DRV_CRYPTO_DMA_SetData().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_SetPushLength(DMA_CONTEXT * const contextData, uint32_t length);

/**
 *  @brief Start a previously configured direct-mode DMA write ("fetch") operation.
 *  @param contextData Pointer to the DMA context space.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_StartFetch(DMA_CONTEXT * const contextData);

/**
 *  @brief Start a previously configured direct-mode DMA read ("push") operation.
 *  @param contextData Pointer to the DMA context space.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_StartPush(DMA_CONTEXT * const contextData);

// *****************************************************************************
// Section: CAM DMA Scatter Gather Mode Operations
// *****************************************************************************

/**
 *  @brief Reset the state of the descriptor control block.
 *  @param contextData Pointer to the DMA context space.
 *  @return None.
 *  @note This call is only to be used with extreme caution as it does not check
 *        DMA state, nor does it reinitialize the DMA engine.
 **/
void DRV_CRYPTO_DMA_ResetDescriptors(DMA_CONTEXT * const contextData);

/**
 *  @brief Add a configuration descriptor to set up the DMA engine.
 *  @param contextData Pointer to the DMA context space.
 *  @param config The contents of the configuration (CAMCONFIGBITS).
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddConfigDescriptor(DMA_CONTEXT * const contextData, const uint32_t * const config);

/**
 *  @brief Add a descriptor containing a key to install.
 *  @param contextData Pointer to the DMA context space.
 *  @param keyNumber The key number (1, 2) to install, e.g. XTS uses two key sections.
 *  @param key The key to install.
 *  @param keyLength The size of the key in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddKeyDescriptor(DMA_CONTEXT * const contextData, AES_KEY_NUMBER keyNumber, const void * const key, const uint32_t keyLength);

/**
 *  @brief Add a descriptor containing an initialization vector (IV) to install.
 *  @param contextData Pointer to the DMA context space.
 *  @param iv The IV to install.
 *  @param ivLength THe length of the IV in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddIvDescriptor(DMA_CONTEXT * const contextData, const void * const iv, const uint32_t ivLength);

/**
 *  @brief Add a descriptor containing additional authentication / header data, without any alignment.
 *  @param contextData Pointer to the DMA context space.
 *  @param data The header data to add.
 *  @param aadLength The length of the data in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddRawHeaderDescriptor(DMA_CONTEXT * const contextData, const void *data, const uint32_t dataLength);

/**
 *  @brief Add a descriptor containing additional authentication / header data.
 *  @param contextData Pointer to the DMA context space.
 *  @param data The header data to add.
 *  @param aadLength The length of the data in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddHeaderDescriptor(DMA_CONTEXT * const contextData, const void * const data, const uint32_t dataLength);

/**
 *  @brief Add a descriptor containing data to process, without any alignment.
 *  @param contextData Pointer to the DMA context space.
 *  @param data The data to process.
 *  @param dataLength The length of the data in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddRawDataDescriptor(DMA_CONTEXT * const contextData, const void * const data, const uint32_t dataLength);

/**
 *  @brief Add a descriptor containing data to process.
 *  @param contextData Pointer to the DMA context space.
 *  @param data The data to process.
 *  @param dataLength The length of the data in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddDataDescriptor(DMA_CONTEXT * const contextData, const void * const data, const uint32_t dataLength);

/**
 *  @brief Set the number of bytes to ignore for the current input data descriptor.
 *  @param contextData Pointer to the DMA context space.
 *  @param ignoreByteCount The number of bytes to ignore.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_SetIgnoreBytes(DMA_CONTEXT * const contextData, const uint32_t ignoreByteCount);

/**
 *  @brief Set the number of padding (invalid) bytes for the data.
 *  @param contextData Pointer to the DMA context space.
 *  @param invalidByteCount The size of the pad (number of invalid bytes).
 *  @param align The alignment flag to set for this descriptor.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_SetInvalidBytes(DMA_CONTEXT * const contextData, const uint32_t invalidByteCount, const uint32_t align);

/**
 *  @brief Add a descriptor indicating the engine is to 'discard' (ignore)
 *         the specified number of bytes in the output stream.
 *  @param contextData Pointer to the DMA context space.
 *  @param discardLength The number of bytes to discard from the output stream.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddDiscardDescriptor(DMA_CONTEXT * const contextData, const uint32_t discardLength);

/**
 *  @brief Add a descriptor to receive data from the engine.
 *  @param contextData Pointer to the DMA context space.
 *  @param resultData The result data buffer into which to store the data.
 *  @param resultDataLength The size of the data buffer in bytes.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_AddResultDescriptor(DMA_CONTEXT * const contextData, const void * const  resultData, const uint32_t resultDataLength);

/**
 *  @brief Start executing a scatter-gather DMA operation.
 *  @param contextData Pointer to the DMA context space.
 *  @return CAM_DMA_NO_ERROR on success, error on failure.
 *  @note If using a blocking operations mode, call DRV_CRYPTO_DMA_WaitForCompletion() to
 *        get the operation's result.  If using a nonblocking operations mode, call
 *        DRV_CRYPTO_DMA_GetExecuteStatus() until not busy, then call DRV_CRYPTO_DMA_Complete()
 *        to end the DMA operation.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_ExecuteDescriptors(DMA_CONTEXT * const contextData);

/**
 *  @brief Get the current DMA execution status.
 *  @param contextData Pointer to the DMA context space.
 *  @return CAM_DMA_NO_ERROR if complete, CAM_DMA_BUSY_ERROR if busy, other error if failed.
 *  @note If using a nonblocking oeprations mode, this is called after issuing a call to
 *        DRV_CRYPTO_DMA_ExecuteDescriptors().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_GetExecuteStatus(DMA_CONTEXT * const contextData);

/**
 *  @brief Wait for a DMA operation to complete.
 *  @param contextData Pointer to the DMA context space.
 *  @return CAM_DMA_NO_ERROR if successful, error on failure.
 *  @note If using a blocking oeprations mode, this is called after issuing a call to
 *        DRV_CRYPTO_DMA_ExecuteDescriptors().
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_WaitForCompletion(DMA_CONTEXT * const contextData);

/**
 *  @brief Mark the DMA operation complete.
 *  @param contextData Pointer to the DMA context space.
 *  @return CAM_DMA_NO_ERROR if successful, error on failure.
 *  @note This must be called if using DRV_CRYPTO_DMA_GetExecuteStatus() to get completion status
 *        in a nonblocking operation mode.
 **/
CAM_DMA_ERROR DRV_CRYPTO_DMA_Complete(DMA_CONTEXT * const contextData);

/**
 * @brief Verify the DMA context is valid.
 * @param contextData The DMA context.
 * @return CAM_DMA_NO_ERROR if valid, CAM_DMA_CONTEXT_ERROR/CAM_DMA_CONTEXT_ALIGNMENT_ERROR if not.
 */
CAM_DMA_ERROR DRV_CRYPTO_DMA_IsValidContext(const DMA_CONTEXT * const contextData);

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif	/* CAM_DMA_LOCAL_H */
