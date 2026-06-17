/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_aes.c

  Summary:
    Crypto Framework Library interface file for CAM AES.

  Description:
    This source file contains the interface that make up the CAM AES hardware
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

/* MISRA C:2012 Rules 8.4 and 11.5 Deviation Justification
 *
 * ID      Category   Description
 * 8.4     Required   A compatible declaration shall be visible when an object or
 *                    function with external linkage is defined
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
 * - Rule 11.5: CAM driver APIs are designed as common interface functions that
 *   accept generic (void*) pointers to support abstract driver contexts and
 *   upper-layer framework integration. The actual object type (e.g., AES_CONTEXT,
 *   ECDH_CONTEXT, ECDSA_CONTEXT, HASH_CONTEXT) is defined later and must be cast
 *   within the implementation. This usage is intentional, safe, and necessary to
 *   maintain a consistent hardware abstraction layer.
 *
 * Scope:
 * This justification applies to all functions with external linkage and all
 * instances of (void*) pointer casts within this source file.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>
#include <string.h>
#include "../inc/cam_aes.h"
#include "cam_aes_local.h"
#include "../inc/cam_device.h"
#if defined(GENERIC_TARGET_CAM_05346)
    #include "sec_cam_05346_v2_07A0000_local.h"
#elif defined(GENERIC_TARGET_CAM_06048)
    #include "sec_cam_06048_v3_07A0000_local.h"
#elif defined(GENERIC_TARGET_HSM_LITE_04777)
    #include "sec_hsm_lite_04777_v1_4F000000_local.h"
#endif
// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

/**
 * @brief Determine if the AES context is initialized.
 * @param aesState The AES context for this operation.
 * @return AES_NO_ERROR if initialized, AES_CONTEXT_ERROR/AES_INITIALIZE_ERROR
 *         if not initialized.
 */
static inline AES_ERROR lDRV_CRYPTO_AES_IsInitialized(AES_CONTEXT *aesState)
{
    register AES_CONTEXT *localState = (AES_CONTEXT *)aesState;
    AES_ERROR status;

    if (localState == NULL)
    {
        status = AES_CONTEXT_ERROR;
    }
    else if ((localState->initialized != AES_CONTEXT_INIT_PATTERN) ||
        (DRV_CRYPTO_DMA_IsValidContext(&localState->dma) != CAM_DMA_NO_ERROR))
    {
        status = AES_INITIALIZE_ERROR;
    }
    else
    {
        status = AES_NO_ERROR;
    }

    return status;
}

/**
 * @brief Set up the DMA engine for an AES operation and initialize the first
 *        (configuration) descriptor.
 * @param aesState The AES context for this operation.
 * @return AES_NO_ERROR if successful,
 *         AES_INVALID_MODE_ERROR / AES_INITIALIZE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *       need to be verified here.
 **/
static AES_ERROR lDRV_CRYPTO_AES_SetupEngine(AES_CONTEXT *aesState)
{
    register AES_CONTEXT *localState = (AES_CONTEXT *)aesState;
    AES_ERROR status = AES_NO_ERROR;

    switch (localState->mode)
    {
        case MODE_ECB:
        case MODE_CTR:
        case MODE_XTS:
        case MODE_CBC:
        case MODE_CFB:
        case MODE_OFB:
            localState->blockSize = AES_BLOCK_SIZE;
            localState->stateSize = AES_SMALL_STATE_CONTEXT_SIZE;
        break;

        case MODE_GCM:
        case MODE_CCM:
            localState->blockSize = AES_BLOCK_SIZE;
            localState->stateSize = AES_STATE_CONTEXT_SIZE;
        break;

        case MODE_CMAC:
            localState->blockSize = CMAC_BLOCK_SIZE;
            localState->stateSize = CMAC_STATE_CONTEXT_SIZE;
        break;

        default:
            status = AES_INVALID_MODE_ERROR;
        break;
    }

    if (status == AES_NO_ERROR)
    {
        // Initialize the DMA context for this operation.
        CAM_DMA_ERROR dmaStatus = DRV_CRYPTO_DMA_Initialize(&localState->dma, CAM_DMA_MODE_DESCRIPTOR, TAG_ENGINE_AES);
        if (dmaStatus == CAM_DMA_NO_ERROR)
        {
            localState->config.MODE = (uint32_t)(1UL << localState->mode);
            localState->config.ENCDEC = localState->op;

            dmaStatus = DRV_CRYPTO_DMA_AddConfigDescriptor(&localState->dma, (uint32_t *)&localState->config);
        }

        if (dmaStatus != CAM_DMA_NO_ERROR)
        {
            status = AES_INITIALIZE_ERROR;
        }
    }

    return status;
}

/**
 * @brief Execute the common setup steps for an AES operaton.
 * @param aesState The AES context for this operation.
 * @param initVector The intialization vector (or state data) to load.
 * @param initVectorLength the length of the IV/state data.
 * @return AES_NO_ERROR is successful, AES_INITIALIZE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *       need to be verified here.
 **/
static AES_ERROR lDRV_CRYPTO_AES_Initialize(AES_CONTEXT *aesState, const uint8_t *initVector, uint32_t initVectorLength)
{
    register AES_CONTEXT *localState = (AES_CONTEXT *)aesState;
    AES_ERROR aesStatus = AES_INITIALIZE_ERROR;
    CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

    localState->left = 0;
    localState->blockSize = 0;
    localState->stateSize = 0;

    // Reset the DMA, initialize the descriptors, and set up the config/key/IV descriptor chain.
    dmaStatus = DRV_CRYPTO_DMA_Reset();
    if (dmaStatus == CAM_DMA_NO_ERROR)
    {
        aesStatus = lDRV_CRYPTO_AES_SetupEngine(localState);
    }

    if (aesStatus == AES_NO_ERROR)
    {
        if((AESCON_MODE) aesState->mode == MODE_XTS)
        {
            uint32_t xtsKeyLength = aesState->keyLength / 2U;

            dmaStatus = DRV_CRYPTO_DMA_AddKeyDescriptor(&localState->dma, KEY1, localState->key, xtsKeyLength);

            if(dmaStatus == CAM_DMA_NO_ERROR)
            {
                dmaStatus = DRV_CRYPTO_DMA_AddKeyDescriptor(&localState->dma,
                                            KEY2,
                                            &localState->key[xtsKeyLength],
                                            xtsKeyLength);

            }
        }
        else
        {
            dmaStatus = DRV_CRYPTO_DMA_AddKeyDescriptor(&localState->dma, KEY1, localState->key, localState->keyLength);
        }

        if (dmaStatus == CAM_DMA_NO_ERROR)
        {
            dmaStatus = DRV_CRYPTO_DMA_AddIvDescriptor(&localState->dma, initVector, initVectorLength);
        }

        if (dmaStatus == CAM_DMA_NO_ERROR)
        {
            aesStatus = AES_NO_ERROR;
            localState->initialized = AES_CONTEXT_INIT_PATTERN;
        }
    }

    return aesStatus;
}

/**
 * @brief Save the state of a DMA operation by generating a partial MAC.
 * @param aesState The AES context for this operation.
 * @return AES_NO_ERROR on success, AES_STATE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *       need to be verified here.
 **/
static AES_ERROR lDRV_CRYPTO_AES_Save(AES_CONTEXT *aesState)
{
    register AES_CONTEXT *localState = (AES_CONTEXT *)aesState;
    AES_ERROR aesStatus = AES_STATE_ERROR;
    CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

    /* Add the 'save-state' flag to the configuration data.
     * This tells the engine to save the partial MAC, which is restored during a resume
     * to continue the AES operation. */
    localState->config.CXSAVE = 1;

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
        aesStatus = AES_NO_ERROR;
    }

    return aesStatus;
}

/**
 * @brief Resume an AES operation by setting up a new DMA operation and reloading
 *         the operation's state.
 * @param aesState The AES context for this operation.
 * @return AES_NO_ERROR on success, AES_STATE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *       need to be verified here.
 **/
static AES_ERROR lDRV_CRYPTO_AES_Resume(AES_CONTEXT *aesState)
{
    register AES_CONTEXT *localState = (AES_CONTEXT *)aesState;
    AES_ERROR aesStatus = AES_STATE_ERROR;

    /* Reinitialize just the DMA's descriptors.  The DMA hardware is already initialized
     * and the hash context has current state. */
    (void)DRV_CRYPTO_DMA_ResetDescriptors(&localState->dma);

    /* Clear the 'save-state' and add the 'restore-state' flags.
     * This tells the DMA engine it should restore the state provided in the
     * initialization-vector (also state data) descriptor. */
    localState->config.CXSAVE = 0;
    localState->config.CXLOAD = 1;

    // Set up a new DMA operation.
    aesStatus = lDRV_CRYPTO_AES_Initialize(localState, localState->state, localState->stateSize);

    return aesStatus;
}

/**
 * @brief Insert the length value into an AES-GCM operation.
 * @param buf Buffer into which to insert the length data.
 * @param buflength Length to insert.
 * @note Caller guarantees the buffer can hold the length data.
 **/
inline static void lDRV_CRYPTO_AES_InsertLength(uint8_t *buf, uint32_t bufLength)
{
    register uint32_t localLength = bufLength;

    for (uint32_t i = 0UL; i < AES_LENALENC_SIZE; i++)
    {
        buf[7U - i] = localLength & 0xFFUL;
        localLength >>= 8UL;
    }
}

/**
 * @brief Complete a AES context by clearing the initialized key.
 * @return AES_NO_ERROR.
 * @note The caller protects the input parameters with checks, so they do not
 *       need to be verified here.
 **/
static AES_ERROR lDRV_CRYPTO_AES_Complete(AES_CONTEXT *aesState)
{
    register AES_CONTEXT *localState = (AES_CONTEXT *)aesState;

    if (localState != NULL)
    {
        localState->initialized = 0UL;
    }

    return AES_NO_ERROR;
}

/**
 * @brief Process an incoming data buffer into chunks that are provided to
 *        DMA to generate a partial MAC value, which is used to seed the
 *        new outstanding descriptor chain that is created.
 * @param aesState The AES context for this operation.
 * @param data The input data buffer.
 * @param dataLength The length of the data buffer.
 * @param blockSize the size of the AES block, used to break the data into chunks.
 * @return AES_NO_ERROR on success, AES_WRITE_ERROR on failure.
 * @note The caller protects the input parameters with checks, so they do not
 *       need to be verified here.
 **/
static AES_ERROR lDRV_CRYPTO_AES_ProcessInputData(AES_CONTEXT *aesState, void * data,
                                                  uint32_t dataLength, uint32_t blockSize)
{
    register AES_CONTEXT *localState = (AES_CONTEXT *)aesState;
    AES_ERROR aesStatus = AES_NO_ERROR;
    register uint32_t localLength = dataLength;

    // CMAC mode has to submit the data in chunks.
    if (localState->mode == (uint32_t)MODE_CMAC)
    {
        /* cppcheck-suppress misra-c2012-11.5 */
        uint8_t *chunk = (uint8_t *)data;
        uint32_t remaining = localLength;

        /* Process the input data in block size chunks.  This is required by the
         * hardware when in CMAC AES mode and DMA scatter-gather mode. */
        while (remaining > 0UL)
        {
            CAM_DMA_ERROR dmaStatus;
            register uint8_t chunkSize = (uint8_t)min(remaining, blockSize);

            dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&localState->dma, chunk, chunkSize);
            if(dmaStatus == CAM_DMA_NO_ERROR)
            {
                remaining -= chunkSize;
                chunk = &chunk[chunkSize];

                /* If there is more data than a chunk size's worth left over, execute a
                 * partial MAC calculation by saving and resuming state.  This creates a
                 * new DMA operation for the next chunk of data.  Any remaining left over
                 * data will be processed either by the next 'add call' or the 'execute' call. */
                if ((chunkSize == blockSize) && (remaining > 0UL))
                {
                    aesStatus = lDRV_CRYPTO_AES_Save(localState);
                    if(aesStatus == AES_NO_ERROR)
                    {
                        aesStatus = lDRV_CRYPTO_AES_Resume(localState);
                    }
                }
            }
            else
            {
                aesStatus = AES_WRITE_ERROR;
            }

            if (aesStatus == AES_NO_ERROR)
            {
                /* The left over data (ranging from 1 to block-size bytes) is tracked.  If
                 * more data is added to the AES operation, this must be a block-size chunk.
                 * If the operation is executed, then this will be the last bit of data to
                 * process and can be less than a block-size. */
                localState->left = chunkSize;
            }
            else
            {
                // Stop on save/resume error.
                break;
            }
        }
    }
    else
    {
        /* Other symmetric modes submit the data as one descriptor and indicate the
         * number of invalid bytes that pad to a block boundary. */
        CAM_DMA_ERROR dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&localState->dma, data, localLength);

        if (dmaStatus == CAM_DMA_NO_ERROR)
        {
            uint32_t mask = (blockSize - 1UL);
            uint32_t pad = ((localLength + mask) & ~mask) - localLength;

            dmaStatus = DRV_CRYPTO_DMA_SetInvalidBytes(&localState->dma, pad, DMA_REALIGN);
        }

        if (dmaStatus != CAM_DMA_NO_ERROR)
        {
            aesStatus = AES_WRITE_ERROR;
        }
    }

    return aesStatus;
}

// *****************************************************************************
// *****************************************************************************
// Section: AES Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

void DRV_CRYPTO_AES_IsrHelper(void)
{
    CAMINTSTATCLR.val = 0x3F;
}

AES_ERROR DRV_CRYPTO_AES_Initialize(void *contextData, AESCON_MODE mode, AESCON_OPERATION operation,
                                    void* key, uint32_t keyLength,
                                    const void *initVector, uint32_t initVectorLength)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = AES_NO_ERROR;

    if (aesState == NULL)
    {
        aesStatus = AES_CONTEXT_ERROR;
    }
    else
    {
        DRV_CRYPTO_DMA_Enable();

        /* Clear the AES context to restart an operation.
         * The state data does not need to be cleared. */
        (void)memset(aesState, 0, (sizeof(AES_CONTEXT) - STATE_CONTEXT_SIZE));

        // Set up the AES operation's context and initialize.
        /* cppcheck-suppress misra-c2012-11.5 */
        aesState->key = key;
        aesState->keyLength = keyLength;
        aesState->mode = mode;
        aesState->op = operation;

        aesStatus = lDRV_CRYPTO_AES_Initialize(aesState, initVector, initVectorLength);
    }
    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_SetOperation(void *contextData, AESCON_OPERATION operation)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = AES_NO_ERROR;

    if (aesState == NULL)
    {
        aesStatus = AES_CONTEXT_ERROR;
    }
    else if (operation > OP_DECRYPT)
    {
        aesStatus = AES_INITIALIZE_ERROR;
    }
    else
    {
        aesState->op = operation;
        aesState->config.ENCDEC = operation;
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_AddHeader(void *contextData, const void * headerData, uint32_t headerLength)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);

    if (aesStatus == AES_NO_ERROR)
    {
        CAM_DMA_ERROR dmaStatus = DRV_CRYPTO_DMA_AddHeaderDescriptor(&aesState->dma, headerData, headerLength);

        if ((dmaStatus == CAM_DMA_NO_ERROR) && (headerLength > 0UL))
        {
            uint32_t mask = (aesState->blockSize - 1UL);
            uint32_t pad = ((headerLength + mask) & ~mask) - headerLength;

            // Header data gets padded to a block boundary and then the padding is 'ignored' as invalid.
            dmaStatus = DRV_CRYPTO_DMA_SetInvalidBytes(&aesState->dma, pad, DMA_REALIGN);
        }

        if (dmaStatus != CAM_DMA_NO_ERROR)
        {
            aesStatus = AES_WRITE_ERROR;
        }
    }

    if (aesStatus == AES_NO_ERROR)
    {
        /* Accumulate the amount of header data added to the operation.
         * This may be used to finalize an operation prior to submitting. */
        aesState->headerLength += headerLength;
    }
    else
    {
        // On failure, end the AES operation in process. This simplifies starting a new AES operation.
        (void)lDRV_CRYPTO_AES_Complete(aesState);
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_AddTweakData(void *contextData, const void *tweakData, uint32_t tweakLength)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);

    if (aesStatus == AES_NO_ERROR)
    {
        if ((tweakData != NULL) && (tweakLength > 0UL))
        {
            CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

            dmaStatus = DRV_CRYPTO_DMA_AddIvDescriptor(&aesState->dma, tweakData, tweakLength);

            if (dmaStatus != CAM_DMA_NO_ERROR)
            {
                aesStatus = AES_WRITE_ERROR;
            }
        }
    }

    if(aesStatus != AES_NO_ERROR)
    {
        (void)lDRV_CRYPTO_AES_Complete(aesState);
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_AddRawHeader(void *contextData, const void *headerData, uint32_t headerLength, uint32_t align)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = AES_NO_ERROR;

    if (aesState == NULL)
    {
        aesStatus = AES_CONTEXT_ERROR;
    }
    else
    {
        CAM_DMA_ERROR dmaStatus;

        /* This is currently used as a flag 0/1, where 0 = do not align.
         * Using a mask versus a boolean allows for future API expansion.*/
        if (align == AES_HEADER_DO_NOT_ALIGN)
        {
            dmaStatus = DRV_CRYPTO_DMA_AddRawHeaderDescriptor(&aesState->dma, headerData, headerLength);
        }
        else
        {
            dmaStatus = DRV_CRYPTO_DMA_AddHeaderDescriptor(&aesState->dma, headerData, headerLength);
        }

        if(dmaStatus != CAM_DMA_NO_ERROR)
        {
            aesStatus = AES_WRITE_ERROR;
        }
    }

    if (aesStatus == AES_NO_ERROR)
    {
        /* Accumulate the amount of header data added to the operation.
         * This may be used to finalize an operation prior to submitting. */
        aesState->headerLength += headerLength;
    }
    else
    {
        // On failure, end the AES operation in process. This simplifies starting a new AES operation.
        (void)lDRV_CRYPTO_AES_Complete(aesState);
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_AddInputData(void *contextData, void * data, uint32_t dataLength)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);
    register uint32_t localLength = dataLength;

    if (aesStatus == AES_NO_ERROR)
    {
        if ((data != NULL) && (localLength > 0UL))
        {
            /* See if there is left over data from a previous add operation that
             * needs to be processed before adding more data, and if that data is
             * the configured block size.  If it is, update the operation's state
             * (by generating the partial MAC) and then add this new data to the
             * new DMA operation that will be created.
             *
             * If the left over data is not aligned to the block size, then no more
             * data can be added as it will mess up the calculation (calculation is
             * only performed in block size chunks). */
            if (aesState->left > 0UL)
            {
                if (aesState->left != aesState->blockSize)
                {
                    aesStatus = AES_STATE_ERROR;
                }
                else
                {
                    /* Generate the partial MAC of the leftover data before adding new data
                     * by executing a 'save/resume' sequence.  This creates a new DMA operation
                     * for this new data. */
                    aesStatus = lDRV_CRYPTO_AES_Save(aesState);
                    if (aesStatus == AES_NO_ERROR)
                    {
                        aesStatus = lDRV_CRYPTO_AES_Resume(aesState);
                    }
                }
            }

            // The new input data can now be processed...
            if (aesStatus == AES_NO_ERROR)
            {
                aesStatus = lDRV_CRYPTO_AES_ProcessInputData(aesState, data, localLength, aesState->blockSize);
            }
        }

        // No data is not considered an error.  No data descriptor is inserted.
    }

    if (aesStatus == AES_NO_ERROR)
    {
        /* Accumulate the amount of input data added to the operation.
         * This may be used to finalize an operation prior to submitting. */
        aesState->dataLength += localLength;
    }
    else
    {
        // On failure, end the AES operation in process. This simplifies starting a new AES operation.
        (void)lDRV_CRYPTO_AES_Complete(aesState);
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_AddOutputData(void *contextData, const void * data, uint32_t dataLength)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);

    if (aesStatus == AES_NO_ERROR)
    {
        register uint32_t localLength = dataLength;

        if ((data != NULL) && (localLength > 0UL))
        {
            CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

            dmaStatus = DRV_CRYPTO_DMA_AddResultDescriptor(&aesState->dma, data, localLength);

            if (dmaStatus != CAM_DMA_NO_ERROR)
            {
                aesStatus = AES_WRITE_ERROR;
            }
        }

        // No data is not considered an error.  No output descriptor is inserted.
    }

    // On failure, end the AES operation in process. This simplifies starting a new AES operation.
    if(aesStatus != AES_NO_ERROR)
    {
        (void)lDRV_CRYPTO_AES_Complete(aesState);
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_IgnoreData(void *contextData, uint32_t ignoreLength)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = AES_NO_ERROR;

    if (aesState == NULL)
    {
        aesStatus = AES_CONTEXT_ERROR;
    }
    else
    {
        if (ignoreLength > 0UL)
        {
            CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

            dmaStatus = DRV_CRYPTO_DMA_SetIgnoreBytes(&aesState->dma, ignoreLength);

            if (dmaStatus != CAM_DMA_NO_ERROR)
            {
                aesStatus = AES_WRITE_ERROR;
            }
        }
    }

    // On failure, end the AES operation in process. This simplifies starting a new AES operation.
    if(aesStatus != AES_NO_ERROR)
    {
        (void)lDRV_CRYPTO_AES_Complete(aesState);
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_DiscardData(void *contextData, uint32_t discardLength)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);

    if (aesStatus == AES_NO_ERROR)
    {
        register uint32_t localLength = discardLength;

        if (localLength > 0UL)
        {
            CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

            dmaStatus = DRV_CRYPTO_DMA_AddDiscardDescriptor(&aesState->dma, localLength);

            if (dmaStatus != CAM_DMA_NO_ERROR)
            {
                aesStatus = AES_WRITE_ERROR;
            }
        }
    }

    // On failure, end the AES operation in process. This simplifies starting a new AES operation.
    if (aesStatus != AES_NO_ERROR)
    {
        (void)lDRV_CRYPTO_AES_Complete(aesState);
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_AddLenALenC(void *contextData)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);

    if (aesStatus == AES_NO_ERROR)
    {
        CAM_DMA_ERROR dmaStatus;

        // Clear the state cache to use as the source for the lenA/lenC data.
        (void)memset(aesState->state, 0, aesState->stateSize);

        /* Calculate lenA and lenC based on the amount of authentication data and
         * input data provided to the operation. */
        uint32_t lenA = (aesState->headerLength << 3UL);
        uint32_t lenC = (aesState->dataLength << 3UL);
        register uint8_t *buf = aesState->state;

        // Start in the first half of the buffer to calculate lenA.
        lDRV_CRYPTO_AES_InsertLength(buf, lenA);

        // Move to the second half of the buffer to calculate lenC.
        buf = &buf[8U];
        lDRV_CRYPTO_AES_InsertLength(buf, lenC);

        dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&aesState->dma, aesState->state, AES_GCM_AUTHTAG_SIZE);
        if (dmaStatus != CAM_DMA_NO_ERROR)
        {
            aesStatus = AES_WRITE_ERROR;
        }
    }

    // On failure, end the AES operation in process. This simplifies starting a new AES operation.
    if(aesStatus != AES_NO_ERROR)
    {
        (void)lDRV_CRYPTO_AES_Complete(aesState);
    }

    return aesStatus;
}

AES_ERROR DRV_CRYPTO_AES_Execute(void *contextData)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);

    if (aesStatus == AES_NO_ERROR)
    {
        CAM_DMA_ERROR dmaStatus = CAM_DMA_NO_ERROR;

        /* If there is no data in CMAC mode, then an 'empty' descriptor must be
         * added to end the chain. */
        if ((aesState->mode == (uint32_t)MODE_CMAC) && (aesState->dataLength == 0UL))
        {
            dmaStatus = DRV_CRYPTO_DMA_AddDataDescriptor(&aesState->dma, aesState->state, 0);

            if (dmaStatus == CAM_DMA_NO_ERROR)
            {
                dmaStatus = DRV_CRYPTO_DMA_SetInvalidBytes(&aesState->dma, aesState->blockSize, DMA_REALIGN);
            }

            if (dmaStatus != CAM_DMA_NO_ERROR)
            {
                aesStatus = AES_EXECUTE_ERROR;
            }
        }
        else
        {
            /* When any left over data is not equal in size to the configured block
             * size, then it must be padded with 'invalid bytes' to reach the block size. */
            if ((aesState->left > 0UL) && (aesState->left != aesState->blockSize))
            {
                dmaStatus = DRV_CRYPTO_DMA_SetInvalidBytes(&aesState->dma, (aesState->blockSize - aesState->left), DMA_REALIGN);
            }
        }

        if (dmaStatus == CAM_DMA_NO_ERROR)
        {
            dmaStatus = DRV_CRYPTO_DMA_ExecuteDescriptors(&aesState->dma);
        }

        if (dmaStatus == CAM_DMA_NO_ERROR)
        {
            dmaStatus = DRV_CRYPTO_DMA_WaitForCompletion(&aesState->dma);
        }

        if (dmaStatus != CAM_DMA_NO_ERROR)
        {
            aesStatus = AES_EXECUTE_ERROR;
        }
    }

    // End the AES operation.
    (void)lDRV_CRYPTO_AES_Complete(aesState);

    return aesStatus;
}


AES_ERROR DRV_CRYPTO_AES_IsActive(void *contextData, AES_ERROR *active)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);

    if (aesStatus == AES_NO_ERROR)
    {
        *active = AES_OPERATION_IS_ACTIVE;
    }
    else
    {
        *active = AES_OPERATION_IS_NOT_ACTIVE;
    }

    return aesStatus;
}

uint32_t DRV_CRYPTO_AES_GetContextSize(void *contextData)
{
    /* cppcheck-suppress misra-c2012-11.5; This is a false positive. */
    register AES_CONTEXT *aesState = (AES_CONTEXT *)contextData;
    AES_ERROR aesStatus = lDRV_CRYPTO_AES_IsInitialized(aesState);
    uint32_t contextSize = (sizeof(AES_CONTEXT) - STATE_CONTEXT_SIZE);

    if (aesStatus == AES_NO_ERROR)
    {
        switch (aesState->mode)
        {
            // CMAC modes use a smaller state context size.
            case MODE_CMAC:
                contextSize += CMAC_STATE_CONTEXT_SIZE;
            break;

            // All other AES modes use the larger state context size.
            default:
                contextSize += AES_STATE_CONTEXT_SIZE;
            break;
        }
    }
    else
    {
        // Default to the largest possible context size.
        contextSize += STATE_CONTEXT_SIZE;
    }

    return contextSize;
}
