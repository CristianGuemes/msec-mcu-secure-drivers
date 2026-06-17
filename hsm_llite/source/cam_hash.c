/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_hash.c

  Summary:
    Crypto Framework Library interface file for CAM Hashing.

  Description:
    This source file contains the interface that make up the CAM Hashing hardware
    driver for Microchip microcontrollers equipped with a Crypto Accelerator Module.
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

/* MISRA C:2012 Rules 8.4, 11.4 and 11.5 Deviation Justification
 *
 * ID      Category   Description
 * 8.4     Required   A compatible declaration shall be visible when an object or
 *                    function with external linkage is defined
 * 11.4    Advisory   A conversion should not be performed between a pointer to
 *                    object and an integer type
 * 11.5    Advisory   A conversion should not be performed from pointer to void
 *                    into pointer to object
 *
 * Justification:
 * - Rule 8.4: All externally visible CAM/HSM driver APIs (e.g., DRV_CRYPTO_*
 *   functions) are declared in their corresponding public header files
 *   (such as cam_aes.h, cam_ecdh.h, cam_ecdsa.h, cam_hash.h). Some MISRA analysis
 *   tools may not fully parse these external headers, resulting in false positives
 *   where it appears that compatible declarations are missing. The implementation
 *   is compliant, as each externally linked function has a matching prototype
 *   defined in the associated header file.
 *
 * - Rule 11.4: Pointer arithmetic using explicit casts to uintptr_t is employed
 *   to safely calculate context buffer offsets and memory size requirements needed
 *   for hardware abstraction and descriptor management. This is performed in strict
 *   accordance with C standard and does not result in pointer value misuse or data loss.
 *   All such conversions are documented and isolated to necessary locations.
 *
 * - Rule 11.5: CAM driver APIs are designed as common interface functions that
 *   accept generic (void*) pointers to support abstract driver contexts and
 *   upper-layer framework integration. The actual object type (e.g., AES_CONTEXT,
 *   ECDH_CONTEXT, ECDSA_CONTEXT, HASH_CONTEXT) is defined later and must be cast
 *   within the implementation. This usage is intentional, safe, and necessary to
 *   maintain a consistent hardware abstraction layer.
 *
 * Scope:
 * This justification applies to all functions with external linkage, all
 * instances of (void*) pointer casts, and all pointer/integer conversions within
 * this source file.
 */


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>
#include <string.h>
#include "../inc/cam_hash.h"
#include "cam_hash_local.h"
#include "../inc/cam_device.h"
#if defined(GENERIC_TARGET_CAM_05346)
    #include "sec_cam_05346_v2_07A0000_local.h"
#elif defined(GENERIC_TARGET_CAM_06048)
    #include "sec_cam_06048_v3_07A0000_local.h"
#elif defined(GENERIC_TARGET_HSM_LITE_04777)
    #include "sec_hsm_lite_04777_v1_4F000000_local.h"
#else
    #error "Unsupported device for Hash: please define GENERIC_TARGET_CAM_05346, GENERIC_TARGET_CAM_06048, or GENERIC_TARGET_HSM_LITE_04777"
#endif
// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

/**
 * @brief Calculate the number of 'invalid' bytes at the end of hash data to pad
 *        to a 4 byte boundary.
 * @param dataLength The length of the data.
 * @return Number of 'invalid' bytes.
 */
static inline uint32_t lDRV_CRYPTO_HASH_GetInvalidByteCount(uint32_t dataLength)
{
    uint32_t invalidByteCount = dataLength & (HASH_INVALID_BYTES - 1UL);
    if (invalidByteCount != 0UL)
    {
        invalidByteCount = (HASH_INVALID_BYTES - invalidByteCount);
    }

    return invalidByteCount;
}

/**
 * @brief Determine if the HASH context is initialized.
 * @param hashState The HASH context for this operation.
 * @return HASH_NO_ERROR if initialized, HASH_CONTEXT_ERROR / HASH_INITIALIZE_ERROR
 *         if not initialized.
 */
static inline HASH_ERROR lDRV_CRYPTO_HASH_IsInitialized(HASH_CONTEXT *hashState)
{
    register HASH_CONTEXT *localState = hashState;
    HASH_ERROR status;

    if (localState == NULL)
    {
        status = HASH_CONTEXT_ERROR;
    }
    else if ((localState->initialized != HASH_CONTEXT_INIT_PATTERN) ||
        (DRV_CRYPTO_DMA_IsValidContext(&localState->dma) != CAM_DMA_NO_ERROR))
    {
        status = HASH_INITIALIZE_ERROR;
    }
    else
    {
        status = HASH_NO_ERROR;
    }

    return status;
}

/**
 * @brief Set up the context and HASH engine hardware.
 * @param hashState The state context to initialize.
 * @param useOpData When set, the operations data space is to be initialized.
 * @return HASH_NO_ERROR if successful,
 *         HASH_INITIALIZE_ERROR / HASH_INVALID_MODE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *       need to be verified here.
 **/
static HASH_ERROR lDRV_CRYPTO_HASH_SetupEngine(HASH_CONTEXT *hashState, bool useOpData)
{
    register HASH_CONTEXT *localState = hashState;
    HASH_ERROR status = HASH_NO_ERROR;
    bool modeHasBeenValidated = false;

    #if defined(GENERIC_TARGET_CAM_06048)

    // The CAM 06048 uses the SHA3 capacity value to define the mode. There is overlap between a few SHA3/SHAKE capacity values used for configuration.
    // When using SHAKE, the SHAKE enable bit must be set.
    if(localState->shakeConfig == (uint32_t) SHAKE_ENABLE)
    {
        switch(localState->mode)
        {
            case MODE_SHAKE128:
            {
                localState->blockSize = SHAKE128_BLOCK_SIZE;
                localState->stateSize = SHAKE_STATE_CONTEXT_SIZE;
                localState->configSha3.MODE = CAPACITY_256;
                localState->configSha3.DIGEST_LENGTH = localState->digestSize;
                localState->configSha3.SHAKE_ENABLE = SHAKE_ENABLE;
            }
            break;

            case MODE_SHAKE256:
            {
                localState->blockSize = SHAKE256_BLOCK_SIZE;
                localState->stateSize = SHAKE_STATE_CONTEXT_SIZE;
                localState->configSha3.MODE = CAPACITY_512;
                localState->configSha3.DIGEST_LENGTH = localState->digestSize;
                localState->configSha3.SHAKE_ENABLE = SHAKE_ENABLE;
            }
            break;
            
            default:
                status = HASH_INVALID_MODE_ERROR;
            break;
        }

        modeHasBeenValidated = true;
    }

    #endif

    if(modeHasBeenValidated == false)
    {
        switch(localState->mode)
        {   
            case MODE_SHA1:
                localState->blockSize = SHA1_BLOCK_SIZE;
                localState->stateSize = SHA1_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA1_DIGEST_SIZE;
                localState->encLengthSize = SHA1_ENCLEN_SIZE;
            break;

            case MODE_SHA224:
                localState->blockSize = SHA224_BLOCK_SIZE;
                localState->stateSize = SHA224_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA224_DIGEST_SIZE;
                localState->encLengthSize = SHA224_ENCLEN_SIZE;
            break;

            case MODE_SHA256:
                localState->blockSize = SHA256_BLOCK_SIZE;
                localState->stateSize = SHA256_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA256_DIGEST_SIZE;
                localState->encLengthSize = SHA256_ENCLEN_SIZE;
            break;

            case MODE_SHA384:
                localState->blockSize = SHA384_BLOCK_SIZE;
                localState->stateSize = SHA384_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA384_DIGEST_SIZE;
                localState->encLengthSize = SHA384_ENCLEN_SIZE;
            break;

            case MODE_SHA512:
                localState->blockSize = SHA512_BLOCK_SIZE;
                localState->stateSize = SHA512_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA512_DIGEST_SIZE;
                localState->encLengthSize = SHA512_ENCLEN_SIZE;
            break;

            #if defined(GENERIC_TARGET_CAM_06048)

            case MODE_SHA3_224:
                localState->blockSize = SHA3_224_BLOCK_SIZE;
                localState->stateSize = SHA3_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA3_224_DIGEST_SIZE;
                localState->configSha3.MODE = CAPACITY_448;
                localState->configSha3.SHAKE_ENABLE = SHAKE_DISABLE;
            break;

            case MODE_SHA3_256:
                localState->blockSize = SHA3_256_BLOCK_SIZE;
                localState->stateSize = SHA3_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA3_256_DIGEST_SIZE;
                localState->configSha3.MODE = CAPACITY_512;
                localState->configSha3.SHAKE_ENABLE = SHAKE_DISABLE;
            break;

            case MODE_SHA3_384:
                localState->blockSize = SHA3_384_BLOCK_SIZE;
                localState->stateSize = SHA3_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA3_384_DIGEST_SIZE;
                localState->configSha3.MODE = CAPACITY_768;
                localState->configSha3.SHAKE_ENABLE = SHAKE_DISABLE;
            break;

            case MODE_SHA3_512:
                localState->blockSize = SHA3_512_BLOCK_SIZE;
                localState->stateSize = SHA3_STATE_CONTEXT_SIZE;
                localState->digestSize = SHA3_512_DIGEST_SIZE;
                localState->configSha3.MODE = CAPACITY_1024;
                localState->configSha3.SHAKE_ENABLE = SHAKE_DISABLE;
            break;

            #endif

            default:
                status = HASH_INVALID_MODE_ERROR;
            break;
        }
    }
    
    if (status == HASH_NO_ERROR)
    {
        if (useOpData == true)
        {
            // Set pointers to working data.
            localState->cache = localState->opData;
            localState->state   = &localState->cache[localState->blockSize];
            localState->padding = &localState->state[localState->stateSize];
        }

        if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA512))
        {
            // Set the configuration for this operation.
            localState->config.MODE = (uint32_t)(1UL << localState->mode);
            localState->config.HMAC = localState->hmac;
            localState->config.PADDING = HW_PADDING;
            localState->config.FINAL = DIGEST;
        }

        #if defined(GENERIC_TARGET_CAM_06048)

        else if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
        {
            // Mode, digest length, and SHAKE enabled bits have been set in the above switch statement.
            localState->configSha3.HMAC = localState->hmac;
            localState->configSha3.PADDING = HW_PADDING;
            localState->configSha3.SAVE_CONTEXT = SAVE_CONTEXT_OFF;
        }
        else
        {
            /* MISRA */
        }

        #endif

        localState->initialized = HASH_CONTEXT_INIT_PATTERN;
    }

    return status;
}

/**
 * @brief Execute the common setup steps for a HASH operation.
 * @param hashState The state context to initialize.
 * @param mode The hash algorithm mode.
 * @param useOpData When set, the operations data space is to be initialized.
 * @return HASH_NO_ERROR if successful, HASH_INVALID_MODE_ERROR / HASH_INITIALIZE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *        need to be verified here.
 **/
static HASH_ERROR lDRV_CRYPTO_HASH_Initialize(HASH_CONTEXT *hashState, HASHCON_MODE mode, bool useOpData)
{
    register HASH_CONTEXT *localState = hashState;
    HASH_ERROR hashStatus = HASH_INITIALIZE_ERROR;
    CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

    localState->mode = mode;
    localState->hmac = HMAC_OFF;
    localState->partialHash = 0UL;
    localState->left = 0;
    localState->blockSize = 0;
    localState->stateSize = 0;

    DRV_CRYPTO_DMA_Enable();

    // Reset the DMA, initialize the descriptors, and set up the config/key/IV descriptor chain.
    dmaStatus = DRV_CRYPTO_DMA_Reset();

    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        // Default to using SHA1/SHA2 engine. If using CAM_06048 device and using SHA3/SHAKE mode, adjust to use SHA3 engine.
        uint32_t engine = TAG_ENGINE_HASH;

        #if defined(GENERIC_TARGET_CAM_06048)

        if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
        {
            engine = TAG_ENGINE_HASH_SHA3;
        }

        #endif

        dmaStatus = DRV_CRYPTO_DMA_Initialize(&localState->dma, CAM_DMA_MODE_DESCRIPTOR, engine);
    }

    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        hashStatus = lDRV_CRYPTO_HASH_SetupEngine(localState, useOpData);
    }

    return hashStatus;
}

/**
 * @brief Save the state of a DMA operation by generating a partial hash.
 * @param hashState The state context to use.
 * @return HASH_NO_ERROR is successful, HASH_STATE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *        need to be verified here.
 **/
static HASH_ERROR lDRV_CRYPTO_HASH_Save(HASH_CONTEXT *hashState)
{
    register HASH_CONTEXT *localState = hashState;
    HASH_ERROR hashStatus = HASH_STATE_ERROR;
    CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

    /* Change the hash engine 'final' state to 'state' to return the state.
     * No hardware padding is used for state. */
    localState->partialHash = 1UL;

    if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA512))
    {
        localState->config.FINAL = STATE;
        localState->config.PADDING = SW_PADDING;
    }

    #if defined(GENERIC_TARGET_CAM_06048)

    else if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
    {
        localState->configSha3.SAVE_CONTEXT = SAVE_CONTEXT_ON;
        localState->configSha3.PADDING = SW_PADDING;
    }

    #endif

    else
    {
        /* MISRA */
    }

    // Save the state into the operation's state cache.
    dmaStatus = DRV_CRYPTO_DMA_AddResultDescriptor(&localState->dma, localState->state, localState->stateSize);
    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        dmaStatus = DRV_CRYPTO_DMA_ExecuteDescriptors(&localState->dma);
    }

    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        dmaStatus = DRV_CRYPTO_DMA_WaitForCompletion(&localState->dma);
    }

    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        hashStatus = HASH_NO_ERROR;
    }

    return hashStatus;
}

/**
 * @brief Resume a HASH operation by setting up a new DMA operation and reloading
 *         the operation's state.
 * @param hashState The state context to use.
 * @return HASH_NO_ERROR if successful, HASH_STATE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *        need to be verified here.
 **/
static HASH_ERROR lDRV_CRYPTO_HASH_Resume(HASH_CONTEXT *hashState)
{
    register HASH_CONTEXT *localState = hashState;
    HASH_ERROR hashStatus = HASH_NO_ERROR;
    CAM_DMA_ERROR dmaStatus = CAM_DMA_GENERAL_ERROR;

    /* Reinitialize just the DMA's descriptors.  The DMA hardware is already initialized
     * and the hash context has current state. */
    (void)DRV_CRYPTO_DMA_ResetDescriptors(&localState->dma);

    // Default to using SHA1/SHA2 config. If using CAM_06048 device and using SHA3/SHAKE mode, adjust pointer to SHA3 config.
    const uint32_t* hashConfig = (uint32_t *) &localState->config;

    /* Once a save/resume operation is executed, we can no longer allow
     * the hardware to pad the data.  Padding must be performed manually
     * when the digest is calculated. */
    if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA512))
    {
       localState->config.PADDING = SW_PADDING;
    }

    #if defined(GENERIC_TARGET_CAM_06048)

    else if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
    {
       localState->configSha3.PADDING = SW_PADDING;
       hashConfig = (uint32_t *) &localState->configSha3;
    }

    #endif

    else
    {
        /* MISRA */
    }

    // Install the config descriptor before any data descriptor.
    dmaStatus = DRV_CRYPTO_DMA_AddConfigDescriptor(&localState->dma, hashConfig);

    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        // Restore the previous hash state.
        dmaStatus = DRV_CRYPTO_DMA_AddHeaderDescriptor(&localState->dma, localState->state, localState->stateSize);
    }

    if (dmaStatus != CAM_DMA_NO_ERROR)
    {
        hashStatus = HASH_STATE_ERROR;
    }

    return hashStatus;
}

/**
 * @brief Calculate the padding data to complete a hash operation after save/resume.
 * @param hashState The state context to use.
 * @return HASH_NO_ERROR.
 **/
static HASH_ERROR lDRV_CRYPTO_HASH_AddPaddingData(HASH_CONTEXT *hashState)
{
    register HASH_CONTEXT *localState = hashState;
    HASH_ERROR hashStatus = HASH_EXECUTE_ERROR;
    CAM_DMA_ERROR dmaStatus;
    uint32_t left;
    uint8_t *pad = localState->padding;
    uint32_t dataLength = localState->dataLength;
    uint32_t padSize;

    /* Calculate the amount of required padding data.
     * If the remaining space in the block will not hold the encoded length + pad indicator,
     * then the pad must be increased by a full block size to hold the encoded length. */
    left = localState->blockSize - (dataLength & (localState->blockSize - 1UL));
    padSize = left;
    if (left <= localState->encLengthSize)
    {
        padSize += localState->blockSize;
    }

    /* Clear the padding space, since it must be all 0s except for the pad indicator
     * and encoded length. */
    (void)memset(pad, 0, padSize);

    // The first byte's MSB (0x80) indicates this is the start of pad data.
    *pad = 0x80;

    // Encode the length at the end of the padding data.
    pad = &(localState->padding[padSize - 1UL]);
    *pad = (dataLength & 0x1FUL) << 3UL;
    dataLength >>= 5UL;
    pad--;

    for (uint32_t i = 0; i < (padSize - 2UL); i++)
    {
        *pad = (uint8_t)(dataLength & 0xFFUL);
        dataLength >>= 8UL;
        pad--;
    }

    // Insert a data descriptor with the padding data.
    dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&localState->dma, localState->padding, padSize);
    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        hashStatus = HASH_NO_ERROR;
    }

    return hashStatus;
}


#if defined(GENERIC_TARGET_CAM_06048)

/**
 * @brief Calculate the SHA3 padding data to complete a hash operation after save/resume.
 * @param hashState The state context to use.
 * @return HASH_NO_ERROR.
 **/
static HASH_ERROR lDRV_CRYPTO_HASH_AddPaddingDataSha3(HASH_CONTEXT *hashState)
{
    register HASH_CONTEXT *localState = hashState;
    HASH_ERROR hashStatus = HASH_EXECUTE_ERROR;
    CAM_DMA_ERROR dmaStatus;
    uint8_t *pad = localState->padding;
    uint32_t dataLength = localState->dataLength;

    /* The SHA3 "sponge" construction equates "rate" to "block size" as it is the number
     * of bytes used during each round of processing.
     */
    uint32_t rate = localState->blockSize;

    uint32_t used = dataLength % rate;
    uint32_t padSize = rate - used;

    // Clear the padding space, since it must be all 0s except for the domain separator and final byte indicator.
    (void) memset(pad, 0, padSize);

    if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHA3_512))
    {
        // The domain separator (0x06) indicates this is the start of pad data for SHA3.
        *pad = 0x06;
    }
    else
    {
        // The domain separator (0x1F) indicates this is the start of pad data for SHAKE.
        *pad = 0x1F;
    }

    // The final byte's MSB (0x80) indicates this is the end of pad data.
    pad[padSize - 1U] |= 0x80;

    // Insert a data descriptor with the padding data.
    dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&localState->dma, localState->padding, padSize);
    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        hashStatus = HASH_NO_ERROR;
    }

    return hashStatus;
}

#endif

/**
 * @brief Finalize the hash data when there is data left over in the cache.
 * @param hashState The state context to use.
 * @return HASH_NO_ERROR on success, HASH_EXECUTE_ERROR on failure.
 **/
static HASH_ERROR lDRV_CRYPTO_HASH_FinalizeData(HASH_CONTEXT *hashState)
{
    register HASH_CONTEXT *localState = hashState;
    HASH_ERROR hashStatus = HASH_EXECUTE_ERROR;
    CAM_DMA_ERROR dmaStatus;

    uint32_t paddingType = SW_PADDING;

    if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA512))
    {
        paddingType = localState->config.PADDING;
    }

    #if defined(GENERIC_TARGET_CAM_06048)

    else if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
    {
        paddingType = localState->configSha3.PADDING;
    }

    #endif

    else
    {
        /* MISRA */
    }

    if (paddingType == (uint32_t) SW_PADDING)
    {
        /* If software padding is required, calculate it and add it to the chain.
         * This is a 'raw' (unaligned) data descriptor because there is padding data. */
        dmaStatus = DRV_CRYPTO_DMA_AddRawDataDescriptor(&localState->dma, localState->cache, localState->left);
        if (dmaStatus == CAM_DMA_NO_ERROR)
        {
            if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA512))
            {
                hashStatus = lDRV_CRYPTO_HASH_AddPaddingData(localState);
            }

            #if defined(GENERIC_TARGET_CAM_06048)

            else if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
            {
                hashStatus = lDRV_CRYPTO_HASH_AddPaddingDataSha3(localState);
            }

            #endif

            else
            {
                /* MISRA */
            }
        }
    }
    else
    {
        /* If hardware padding is still in use (total data is less than a block size),
         * then an aligned data descriptor is added and padded with with 'invalid bytes'
         * to pad to a 32-bit size. */
        dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&localState->dma, localState->cache, localState->left);
        if (dmaStatus == CAM_DMA_NO_ERROR)
        {
            uint32_t invalidByteCount = lDRV_CRYPTO_HASH_GetInvalidByteCount(localState->left);

            dmaStatus = DRV_CRYPTO_DMA_SetInvalidBytes(&localState->dma, invalidByteCount, DMA_REALIGN);
            if (dmaStatus == CAM_DMA_NO_ERROR)
            {
                hashStatus = HASH_NO_ERROR;
            }
        }
    }

    return hashStatus;
}

/**
 * @brief Complete a HASH context by clearing the initialized key.
 * @param hashState The state context to use.
 * @return HASH_NO_ERROR.
 **/
static inline HASH_ERROR lDRV_CRYPTO_HASH_Complete(HASH_CONTEXT *hashState)
{
    if (hashState != NULL)
    {
        hashState->initialized = 0UL;
    }

    return HASH_NO_ERROR;
}

/**
 * @brief Process an incoming data buffer into chunks that are provided to
 *        DMA to generate a partial hash value, which is used to seed the
 *        new outstanding descriptor chain that is created.
 * @param hashState The state context to use.
 * @param data The input data buffer.
 * @param dataLength The length of the data buffer.
 * @return HASH_NO_ERROR if successful, HASH_UPDATE_ERROR / HASH_STATE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *       need to be verified here.
 **/
static HASH_ERROR lDRV_CRYPTO_HASH_ProcessInputData(HASH_CONTEXT *hashState, void * data, uint32_t dataLength)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    register uint8_t *chunk = (uint8_t *)data;
    register HASH_CONTEXT *localState = hashState;
    uint32_t remaining = dataLength;
    /* cppcheck-suppress misra-c2012-11.4*/
    bool aligned = ((uint32_t)chunk & 0x3UL) == 0UL;
    HASH_ERROR hashStatus = HASH_NO_ERROR;

    do
    {
        // Add data until the cache is full or there's no more data.
        register uint32_t dataToCache = min(remaining, (localState->blockSize - localState->left));
        bool direct = aligned & (bool)(dataToCache == localState->blockSize);

        /* If the data to process is aligned to a 4 byte boundary and equal to a block size,
         * then the memcpy is not needed.  Data can be processed directly. */
        if (direct == false)
        {
            (void)memcpy(&localState->cache[localState->left], chunk, dataToCache);
        }

        // If the cache is full, hash it and save state.
        localState->left += dataToCache;
        if (localState->left == localState->blockSize)
        {
            CAM_DMA_ERROR dmaStatus;

            /* If a partial hash is in progress, 'resume' by restoring the
             * partial hash.  This sets up the config descriptor and adds the
             * partial hash state descriptor. */
            if (localState->partialHash != 0UL)
            {
                hashStatus = lDRV_CRYPTO_HASH_Resume(localState);
            }
            else
            {
                // Default to using SHA1/SHA2 config. If using CAM_06048 device and using SHA3/SHAKE mode, adjust pointer to SHA3 config.
                const uint32_t* hashConfig = (uint32_t *) &hashState->config;

                #if defined(GENERIC_TARGET_CAM_06048)

                if(IS_EQUAL_TO_OR_BETWEEN_VALUES(localState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
                {
                    hashConfig = (uint32_t *) &hashState->configSha3;
                }

                #endif

                // Install the config descriptor before any data descriptor.
                dmaStatus = DRV_CRYPTO_DMA_AddConfigDescriptor(&hashState->dma, hashConfig);

                if (dmaStatus != CAM_DMA_NO_ERROR)
                {
                    hashStatus = HASH_UPDATE_ERROR;
                }
            }

            if (hashStatus == HASH_NO_ERROR)
            {
                /* If adding a complete block, point to the block data directly.
                 * Otherwise, use the cache. */
                const uint8_t *dataPtr = (direct == true) ? chunk : localState->cache;

                // Add the data descriptor, then generate the partial hash.
                dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&localState->dma, dataPtr, localState->blockSize);
                if (dmaStatus == CAM_DMA_NO_ERROR)
                {
                    hashStatus = lDRV_CRYPTO_HASH_Save(localState);

                    // The data in the cache has been processed.
                    localState->left = 0;
                }
                else
                {
                    hashStatus = HASH_UPDATE_ERROR;
                }
            }
        }

        // Stop on save/resume errors.
        if (hashStatus != HASH_NO_ERROR)
        {
            break;
        }

        // Move to the next chunk.
        remaining -= dataToCache;
        chunk = &chunk[dataToCache];
        localState->dataLength += dataToCache;

    } while (remaining > 0UL);

    return hashStatus;
}

/**
 * @brief This helper function adds data to the ongoing hash operation.
 * @param hashState A pointer to the context block that contains the hash context data.
 * @param data The data to add to the operation.
 * @param dataLength The length of the data.
 * @retval HASH_NO_ERROR          Success.
 * @retval HASH_INITIALIZE_ERROR  Failure during hash initialization.
 * @retval HASH_STATE_ERROR       Invalid or unexpected hash state.
 * @retval HASH_UPDATE_ERROR      Failure while updating the hash.
 * @note The caller verifies the hashState is valid, so it does not
 *         need to be checked here.
 */
static HASH_ERROR lDRV_CRYPTO_HASH_Update(HASH_CONTEXT *hashState, uint8_t *data, uint32_t dataLength)
{
    HASH_ERROR hashStatus = HASH_NO_ERROR;

    // NULL data of 0 length is allowed.  No descriptor will be inserted.
    if ((data != NULL) && (dataLength > 0UL))
    {
        hashStatus = lDRV_CRYPTO_HASH_ProcessInputData(hashState, data, dataLength);
    }

    /* On failure, end the HASH operation in process.
     * This simplifies starting a new HASH operation. */
    if (hashStatus != HASH_NO_ERROR)
    {
        (void)lDRV_CRYPTO_HASH_Complete(hashState);
    }

    return hashStatus;
}

/**
 * @brief This helper function finalizes the hash operation and generates the digest.
 * @param hashState A pointer to the context block that contains the hash context data.
 * @param digest Buffer to contain the digest.
 * @param digestLength The size of the digest buffer.
 * @retval HASH_NO_ERROR          Success.
 * @retval HASH_INITIALIZE_ERROR  Failure during hash initialization.
 * @retval HASH_STATE_ERROR       Invalid or unexpected hash state.
 * @retval HASH_EXECUTE_ERROR     Failure during hash execution.
 * @note The caller verifies the hashState is valid, so it does not
 *         need to be checked here.
 */
static HASH_ERROR lDRV_CRYPTO_HASH_Final(HASH_CONTEXT *hashState, const uint8_t *digest, uint32_t digestLength)
{
    HASH_ERROR hashStatus = HASH_NO_ERROR;

    if ((digest == NULL) || (digestLength == 0UL))
    {
        // A digest buffer is required.
        hashStatus = HASH_EXECUTE_ERROR;
    }

    if (hashStatus == HASH_NO_ERROR)
    {
        CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

        /* If a partial hash is in progress, 'resume' by restoring the
         * partial hash.  This sets up the config descriptor and adds the
         * partial hash state descriptor. */
        if (hashState->partialHash != 0UL)
        {
            hashStatus = lDRV_CRYPTO_HASH_Resume(hashState);
        }
        else
        {
            // Default to using SHA1/SHA2 config. If using CAM_06048 device and using SHA3/SHAKE mode, adjust pointer to SHA3 config.
            const uint32_t* hashConfig = (uint32_t *) &hashState->config;

            #if defined(GENERIC_TARGET_CAM_06048)

            if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
            {
                hashConfig = (uint32_t *) &hashState->configSha3;
            }

            #endif

            // Install the config descriptor before any data descriptor.
            dmaStatus = DRV_CRYPTO_DMA_AddConfigDescriptor(&hashState->dma, hashConfig);

            if (dmaStatus != CAM_DMA_NO_ERROR)
            {
                hashStatus = HASH_UPDATE_ERROR;
            }
        }

        if (hashStatus == HASH_NO_ERROR)
        {
            // Indicate this is the final round and to generate the digest.
            if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA512))
            {
                hashState->config.FINAL = DIGEST;
            }

            #if defined(GENERIC_TARGET_CAM_06048)

            else if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
            {
                hashState->configSha3.SAVE_CONTEXT = SAVE_CONTEXT_OFF;
            }
            else
            {
                /* MISRA */
            }

            #endif

            if (hashState->dataLength == 0UL)
            {
                /* An empty hash is being calculated.  An 'empty' descriptor must be
                 * added to finish the chain, which will be ignored by the hardware. */
                dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&hashState->dma, hashState->cache, 0UL);
                if (dmaStatus == CAM_DMA_NO_ERROR)
                {
                    dmaStatus = DRV_CRYPTO_DMA_SetInvalidBytes(&hashState->dma, HASH_INVALID_BYTES, DMA_REALIGN);
                }

                if (dmaStatus != CAM_DMA_NO_ERROR)
                {
                    hashStatus = HASH_EXECUTE_ERROR;
                }
            }
            else
            {
                // Add the data remaining in the cache to the descriptor chain.
                hashStatus = lDRV_CRYPTO_HASH_FinalizeData(hashState);
            }
        }

        if (hashStatus == HASH_NO_ERROR)
        {
            // Fill out the rest of the descriptor chain and execute.
            if (dmaStatus == CAM_DMA_NO_ERROR)
            {
                dmaStatus = DRV_CRYPTO_DMA_AddResultDescriptor(&hashState->dma, digest, digestLength);
            }

            if (dmaStatus == CAM_DMA_NO_ERROR)
            {
                dmaStatus = DRV_CRYPTO_DMA_ExecuteDescriptors(&hashState->dma);
            }

            if (dmaStatus == CAM_DMA_NO_ERROR)
            {
                dmaStatus = DRV_CRYPTO_DMA_WaitForCompletion(&hashState->dma);
            }

            if (dmaStatus != CAM_DMA_NO_ERROR)
            {
                hashStatus = HASH_EXECUTE_ERROR;
            }
        }
    }

    // Regardless of status, the hash is complete because it was finalized.
    (void)lDRV_CRYPTO_HASH_Complete(hashState);

    return hashStatus;
}

/**
 * @brief This helper function performs a complete hash operation and generates a digest.
 * @param hashState A pointer to the context block that contains the hash context data.
 * @param mode The HASHCON_MODE to use.
 * @param data The data to add to the operation.
 * @param dataLength The length of the data.
 * @param digest Buffer to contain the digest.
 * @param digestLength The size of the digest buffer.
 * @retval HASH_NO_ERROR          Success.
 * @retval HASH_INITIALIZE_ERROR  Failure during hash initialization.
 * @retval HASH_STATE_ERROR       Invalid or unexpected hash state.
 * @retval HASH_EXECUTE_ERROR     Failure during hash execution.
 * @note The caller verifies the hashState is valid, so it does not
 *         need to be checked here.
 */
static HASH_ERROR lDRV_CRYPTO_HASH_Digest(HASH_CONTEXT *hashState, HASHCON_MODE mode,
    const uint8_t *data, uint32_t dataLength,
    const uint8_t * digest, uint32_t digestLength)
{
    HASH_ERROR hashStatus = HASH_NO_ERROR;

    if ((digest == NULL) || (digestLength == 0UL))
    {
        // A digest buffer is required.
        hashStatus = HASH_EXECUTE_ERROR;
    }
    else
    {
        // Initialize, with the operations data space unused.
        hashStatus = lDRV_CRYPTO_HASH_Initialize(hashState, mode, false);
    }

    if (hashStatus == HASH_NO_ERROR)
    {
        CAM_DMA_ERROR dmaStatus = CAM_DMA_GENERAL_ERROR;

        // Default to using SHA1/SHA2 config. If using CAM_06048 device and using SHA3 mode, adjust pointer to SHA3 config.
        const uint32_t* hashConfig = (uint32_t *) &hashState->config;

        #if defined(GENERIC_TARGET_CAM_06048)

        if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHA3_224, (uint32_t) MODE_SHAKE256))
        {
            hashConfig = (uint32_t *) &hashState->configSha3;
        }

        #endif

        // Install the config descriptor before any data descriptor.
        dmaStatus = DRV_CRYPTO_DMA_AddConfigDescriptor(&hashState->dma, hashConfig);

        if (dmaStatus != CAM_DMA_NO_ERROR)
        {
            hashStatus = HASH_UPDATE_ERROR;
        }

        // If data is provided, submit as a single data block for hashing.
        if ((hashStatus == HASH_NO_ERROR) && (data != NULL) && (dataLength > 0UL))
        {
            dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&hashState->dma, data, dataLength);
            if (dmaStatus != CAM_DMA_NO_ERROR)
            {
                hashStatus = HASH_UPDATE_ERROR;
            }
        }

        if (hashStatus == HASH_NO_ERROR)
        {
            uint32_t invalidByteCount = HASH_INVALID_BYTES;

            // Indicate this is the final round and to generate the digest.
            if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA512))
            {
                ((HASH_CONFIG*) hashConfig)->FINAL = DIGEST;
            }

            #if defined(GENERIC_TARGET_CAM_06048)

            else
            {
                ((HASH_CONFIG_SHA3*) hashConfig)->SAVE_CONTEXT = SAVE_CONTEXT_OFF;
            }

            #endif

            if (dataLength == 0UL)
            {
                uint32_t dummyData = 0UL;

                /* An empty hash is being calculated.  An 'empty' descriptor must be
                 * added to finish the chain, which will be ignored by the hardware. */
                dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&hashState->dma, &dummyData, 0UL);
                if (dmaStatus != CAM_DMA_NO_ERROR)
                {
                    hashStatus = HASH_EXECUTE_ERROR;
                }
            }
            else
            {
                // Calculate the invalid bytes (number of bytes to round to a 4-byte boundary).
                invalidByteCount = lDRV_CRYPTO_HASH_GetInvalidByteCount(dataLength);
            }

            if (hashStatus == HASH_NO_ERROR)
            {
                dmaStatus = DRV_CRYPTO_DMA_SetInvalidBytes(&hashState->dma, invalidByteCount, DMA_REALIGN);

                // Fill out the rest of the descriptor chain and execute.
                if (dmaStatus == CAM_DMA_NO_ERROR)
                {
                    dmaStatus = DRV_CRYPTO_DMA_AddResultDescriptor(&hashState->dma, digest, digestLength);
                }

                if (dmaStatus == CAM_DMA_NO_ERROR)
                {
                    dmaStatus = DRV_CRYPTO_DMA_ExecuteDescriptors(&hashState->dma);
                }

                if (dmaStatus == CAM_DMA_NO_ERROR)
                {
                    dmaStatus = DRV_CRYPTO_DMA_WaitForCompletion(&hashState->dma);
                }

                if (dmaStatus != CAM_DMA_NO_ERROR)
                {
                    hashStatus = HASH_EXECUTE_ERROR;
                }
            }
        }
    }

    return hashStatus;
}

// *****************************************************************************
// *****************************************************************************
// Section: SHA Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

void DRV_CRYPTO_HASH_IsrHelper(void)
{
    CAMINTSTATCLR.val = 0x3F;
}

HASH_ERROR DRV_CRYPTO_HASH_Initialize(void *contextData, HASHCON_MODE mode)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = HASH_NO_ERROR;

    if (hashState == NULL)
    {
        hashStatus = HASH_CONTEXT_ERROR;
    }
    else
    {
        #if defined(GENERIC_TARGET_CAM_06048)

        hashState->shakeConfig = SHAKE_DISABLE;

        #endif

        hashStatus = lDRV_CRYPTO_HASH_Initialize(hashState, mode, true);
    }

    return hashStatus;
}

HASH_ERROR DRV_CRYPTO_HASH_Update(void *contextData, uint8_t *data, uint32_t dataLength)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = lDRV_CRYPTO_HASH_IsInitialized(hashState);

    #if defined(GENERIC_TARGET_CAM_06048)

    if(hashStatus == HASH_NO_ERROR)
    {
        if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA3_512))
        {
            hashStatus = lDRV_CRYPTO_HASH_Update(hashState, data, dataLength);
        }
        else
        {
            hashStatus = HASH_CONTEXT_MODE_MISMATCH_ERROR;
        }
    }

    #else

    if(hashStatus == HASH_NO_ERROR)
    {
        hashStatus = lDRV_CRYPTO_HASH_Update(hashState, data, dataLength);
    }

    #endif

    return hashStatus;
}

HASH_ERROR DRV_CRYPTO_HASH_Final(void *contextData, const uint8_t *digest, uint32_t digestLength)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = lDRV_CRYPTO_HASH_IsInitialized(hashState);

    #if defined(GENERIC_TARGET_CAM_06048)

    if(hashStatus == HASH_NO_ERROR)
    {
        if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHA1, (uint32_t) MODE_SHA3_512))
        {
            hashStatus = lDRV_CRYPTO_HASH_Final(hashState, digest, digestLength);
        }
        else
        {
            hashStatus = HASH_CONTEXT_MODE_MISMATCH_ERROR;
        }
    }

    #else

    if(hashStatus == HASH_NO_ERROR)
    {
        hashStatus = lDRV_CRYPTO_HASH_Final(hashState, digest, digestLength);
    }

    #endif

    return hashStatus;
}

HASH_ERROR DRV_CRYPTO_HASH_Digest(void *contextData, HASHCON_MODE mode,
    const uint8_t *data, uint32_t dataLength,
    const uint8_t *digest, uint32_t digestLength)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = HASH_NO_ERROR;

    if (hashState == NULL)
    {
        hashStatus = HASH_CONTEXT_ERROR;
    }
    else
    {
        #if defined(GENERIC_TARGET_CAM_06048)

        hashState->shakeConfig = SHAKE_DISABLE;

        #endif

        hashStatus = lDRV_CRYPTO_HASH_Digest(hashState, mode, data, dataLength, digest, digestLength);
    }

    return hashStatus;   
}

#if defined(GENERIC_TARGET_CAM_06048)

HASH_ERROR DRV_CRYPTO_HASH_SHAKE_Initialize(void *contextData, HASHCON_MODE mode, uint16_t digestLength)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = HASH_NO_ERROR;

    if (hashState == NULL)
    {
        hashStatus = HASH_CONTEXT_ERROR;
    }
    else
    {
        hashState->digestSize = digestLength;
        hashState->shakeConfig = SHAKE_ENABLE;

        hashStatus = lDRV_CRYPTO_HASH_Initialize(hashState, mode, true);
    }

    return hashStatus;
}

HASH_ERROR DRV_CRYPTO_HASH_SHAKE_Update(void *contextData, uint8_t *data, uint32_t dataLength)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = lDRV_CRYPTO_HASH_IsInitialized(hashState);

    if(hashStatus == HASH_NO_ERROR)
    {
        if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHAKE128, (uint32_t) MODE_SHAKE256))
        {
            hashStatus = lDRV_CRYPTO_HASH_Update(hashState, data, dataLength);
        }
        else
        {
            hashStatus = HASH_CONTEXT_MODE_MISMATCH_ERROR;
        }
    }

    return hashStatus;
}

HASH_ERROR DRV_CRYPTO_HASH_SHAKE_Final(void *contextData, const uint8_t *digest)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = lDRV_CRYPTO_HASH_IsInitialized(hashState);

    if(hashStatus == HASH_NO_ERROR)
    {
        if(IS_EQUAL_TO_OR_BETWEEN_VALUES(hashState->mode, (uint32_t) MODE_SHAKE128, (uint32_t) MODE_SHAKE256))
        {
            hashStatus = lDRV_CRYPTO_HASH_Final(hashState, digest, hashState->digestSize);
        }
        else
        {
            hashStatus = HASH_CONTEXT_MODE_MISMATCH_ERROR;
        }
    }

    return hashStatus;
}

HASH_ERROR DRV_CRYPTO_HASH_SHAKE_Digest(void *contextData, HASHCON_MODE mode,
    const uint8_t *data, uint32_t dataLength,
    const uint8_t *digest, uint16_t digestLength)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = HASH_NO_ERROR;

    if (hashState == NULL)
    {
        hashStatus = HASH_CONTEXT_ERROR;
    }
    else
    {
        hashState->digestSize = digestLength;
        hashState->shakeConfig = SHAKE_ENABLE;

        hashStatus = lDRV_CRYPTO_HASH_Digest(hashState, mode, data, dataLength, digest, digestLength);
    }

    return hashStatus;  
}

#endif

HASH_ERROR DRV_CRYPTO_HASH_IsActive(void *contextData, HASH_ERROR *active)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    HASH_ERROR hashStatus = lDRV_CRYPTO_HASH_IsInitialized(hashState);

    if (hashStatus == HASH_NO_ERROR)
    {
        *active = HASH_OPERATION_IS_ACTIVE;
    }
    else
    {
        *active = HASH_OPERATION_IS_NOT_ACTIVE;
    }

    return hashStatus;
}

uint32_t DRV_CRYPTO_HASH_GetContextSize(void *contextData)
{
    /* cppcheck-suppress misra-c2012-11.5 */
    HASH_CONTEXT *hashState = (HASH_CONTEXT *)contextData;
    uint32_t contextSize = sizeof(HASH_CONTEXT);

    if (hashState != NULL)
    {
        // Start with the minimum space before the working data.
        /* cppcheck-suppress misra-c2012-11.4*/
        contextSize = (uint32_t)hashState->opData - (uint32_t)hashState;

        // Add in the cache, state and pad sizes for this operation's mode.
        contextSize += hashState->blockSize;
        contextSize += hashState->stateSize;
        contextSize += (hashState->blockSize + hashState->encLengthSize);
    }

    return contextSize;
}