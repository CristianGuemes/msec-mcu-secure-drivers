/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_dma.c

  Summary:
    Crypto Framework Library interface file for CAM DMA.

  Description:
    This source file contains the interface that make up the CAM DMA hardware
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

/* MISRA C:2012 Rule 11.4 Deviation Justification
 *
 * ID      Category   Description
 * 11.4    Advisory   A conversion should not be performed between a pointer to
 *                    object and an integer type
 *
 * Justification:
 * Conversions between object pointers (e.g., DMA descriptor arrays) and integer
 * types (uint32_t) are required to provide memory-mapped register interfaces
 * with the physical base addresses of descriptor chains. The hardware requires
 * these addresses in integer form, and this pattern is common in device drivers
 * for memory-mapped peripherals. The conversions are safe because the values
 * are not dereferenced as integers; they are consumed only by the DMA engine.
 *
 * Scope:
 * This justification applies to all instances in this source file where an
 * object pointer is cast to an integer type for programming CAM DMA registers.
 */

/* MISRA C:2012 Rules 11.5 Deviation Justification
 *
 * ID      Category   Description
 * 11.5    Advisory   A conversion should not be performed from pointer to void
 *                    into pointer to object
 *
 * Justification:
 * Rule 11.5: CAM driver APIs are designed as common interface functions that
 * accept generic (void*) pointers to support abstract driver contexts and
 * upper-layer framework integration. The actual object type (e.g., AES_CONTEXT,
 * ECDH_CONTEXT, ECDSA_CONTEXT, HASH_CONTEXT) is defined later and must be cast
 * within the implementation. This usage is intentional, safe, and necessary to
 * maintain a consistent hardware abstraction layer.
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
#include "cam_dma_local.h"
#include "../inc/cam_device.h"

#if defined(UNIT_TESTING)
    #include "../scripts/testing/mocks/cam_hw_mock.h"
#elif defined(GENERIC_TARGET_CAM_05346)
    #include "sec_cam_05346_v2_07A0000_local.h"
#elif defined(GENERIC_TARGET_CAM_06048)
    #include "sec_cam_06048_v3_07A0000_local.h"
#elif defined(GENERIC_TARGET_HSM_LITE_04777)
    #include "sec_hsm_lite_04777_v1_4F000000_local.h"
#else
    #error "Unsupported device for DMA: please define GENERIC_TARGET_CAM_05346, GENERIC_TARGET_CAM_06048, or GENERIC_TARGET_HSM_LITE_04777"
#endif

#if defined(UNIT_TESTING)
/* cppcheck-suppress misra-c2012-8.4 */
bool g_dmaIsBusy = false;
#else
static bool g_dmaIsBusy = false;
#endif
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

/**
 * @brief Verify the engine is supported.
 * @param engine The TAG_ENGINE_SELECTION engine value.
 * @return CAM_DMA_NO_ERROR if valid, CAM_DMA_TAG_UNIMPLEMENTED_ENGINE_SELECTED if unsupported.
 */
static inline CAM_DMA_ERROR lDRV_CRYPTO_DMA_EngineIsSupported(TAG_ENGINE_SELECTION engine)
{
    CAM_DMA_ERROR status = CAM_DMA_TAG_UNIMPLEMENTED_ENGINE_SELECTED;

    switch (engine)
    {
        case TAG_ENGINE_HASH:
        case TAG_ENGINE_AES:

        #if defined(GENERIC_TARGET_CAM_06048)

        case TAG_ENGINE_HASH_SHA3:

        #endif
        {
            status = CAM_DMA_NO_ERROR;
        }
        break;

        default:
            break;
    }

    return status;
}

/**
 *  @brief Finalize the descriptor chain.
 *  @param descriptor The start of the descriptor chain to finalize.
 *  @param count The number of descriptors in the chain.
 **/
static void lDRV_CRYPTO_FinalizeDescriptors(DMA_DESCRIPTOR *descriptor, uint32_t count)
{
    register DMA_DESCRIPTOR *current = descriptor;
    register DMA_DESCRIPTOR *last = current;

    // Set the next pointers for each descriptor.
    for (uint32_t i = 0U; i < count; i++)
        {
            descriptor[i].next = &descriptor[i + 1U];
            last = &descriptor[i];
        }

    // The last descriptor in the chain gets the 'last' tag flag,
    // and the 'next' is set to the 'stop' bit (bit 0).
    last->tag |= DMA_TAG_LAST(TAG_IS_LAST);
    /* cppcheck-suppress misra-c2012-11.4 */
    last->next = DMA_LAST_DESCRIPTOR;
    last->size |= DMA_REALIGN;
}

/**
 *  @brief Common add-descriptor function.
 *  @param descriptor The descriptor entry to populate.
 *  @param data The data buffer for the descriptor.
 *  @param dataLength The length of the data buffer.
 *  @param tag The DMA tag to apply for this descriptor.
 *  @param align The alignment flag to set for this descriptor.
 **/
static inline void lDRV_CRYPTO_DMA_AddDescriptor(DMA_DESCRIPTOR *descriptor,
        const void * const data, const uint32_t dataLength, const uint32_t tag, uint32_t align)
{
    register DMA_DESCRIPTOR *localDescriptor = descriptor;

    /* cppcheck-suppress misra-c2012-11.5 */
    localDescriptor->address = (volatile uint32_t const *)data;
    localDescriptor->size = (dataLength & ADDRESS_LENGTH_MASK) | align;
    localDescriptor->tag = tag;
}


/**
 * @brief Add a header descriptor with or without alignment.
 * @param contextData The DMA context.
 * @param data The header data to add.
 * @param dataLength The length of the header data.
 * @param align The alignment flag to apply.
 * @note Validity of the context is assumed by the caller.
 */
static CAM_DMA_ERROR lDRV_CRYPTO_DMA_AddHeaderDescriptor(DMA_CONTEXT *contextData, const void * const data, const uint32_t dataLength, uint32_t align)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if ((data == NULL) || (dataLength == 0UL))
    {
        /* Empty data is allowed, and means no descriptor is required... though this
         * doesn't need to be called if it is empty. */
        status = CAM_DMA_NO_ERROR;
    }
    else if (dma->inOffset >= DMA_IN_DESCRIPTOR_COUNT)
    {
        status = CAM_DMA_ALL_DESCRIPTORS_USED;
    }
    else
    {
        register DMA_DESCRIPTOR *descriptor = &dma->inDescriptors[dma->inOffset];
        uint32_t tag;

        if (ENGINE_IS_AES(dma->engine))
        {
            tag = (uint32_t)dma->engine | DMA_DATATYPE_TAG(TAG_AES_HEADER);
        }
        else
        {
            tag = (uint32_t)dma->engine | DMA_DATATYPE_TAG(TAG_HASH_IV) | DMA_TAG_LAST(TAG_IS_LAST);
        }

        lDRV_CRYPTO_DMA_AddDescriptor(descriptor, data, dataLength, tag, align);
        dma->inOffset++;
    }

    return status;
}


// *****************************************************************************
// *****************************************************************************
// Section: CAM DMA Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

CAM_DMA_ERROR DRV_CRYPTO_DMA_Initialize(DMA_CONTEXT *contextData, CAM_DMA_MODE mode, TAG_ENGINE_SELECTION engine)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;

    if (contextData == NULL)
    {
        status = CAM_DMA_CONTEXT_ERROR;
    }
    /* cppcheck-suppress misra-c2012-11.4 */
    else if (0UL != ((uint32_t)contextData & 0x3UL))
    {
        // Data must be 4-byte aligned.
        status = CAM_DMA_CONTEXT_ALIGNMENT_ERROR;
    }
    else
    {

    }

    if (status == CAM_DMA_NO_ERROR)
    {
        register DMA_CONTEXT *dma = contextData;

        /* If the shared resource is busy, an operation is either in progress (direct mode)
        * or actively running (descriptor mode).  The DMA needs to either complete or be
        * reset to proceed. */
        if (g_dmaIsBusy == true)
        {
            status = CAM_DMA_TRY_AGAIN_LATER;
        }

        if (status == CAM_DMA_NO_ERROR)
        {
            // Clear the signature, in case the initialization does not succeed.
            dma->signature = 0UL;

            switch (mode)
            {
                case CAM_DMA_MODE_DIRECT:
                {
                    /* Reset the DMA before use.
                     * Direct mode makes multiple changes to the DMA engine as it proceeds. */
                    (void) DRV_CRYPTO_DMA_Reset();

                    // Put the device in direct-mode.
                    CAMCONFIG.bits.FETCHSG = 0;
                    CAMCONFIG.bits.PUSHSG = 0;

                    /* This global state flag gates the single DMA resource.
                     * For direct mode operations, the DMA resource is considered to be
                     * 'in use' since multiple calls modify the DMA engine's registers. */
                    g_dmaIsBusy = true;
                }
                break;

                case CAM_DMA_MODE_DESCRIPTOR:
                {
                    // Reset descriptor array offsets.
                    dma->inOffset = 0;
                    dma->outOffset = 0;

                    /* Put the device in descriptor mode.
                     * Descriptor mode does not consume the DMA resource until it executes. */
                    CAMCONFIG.bits.FETCHSG = 1;
                    CAMCONFIG.bits.PUSHSG = 1;
                }
                break;

                default:
                {
                    status = CAM_DMA_INVALID_MODE;
                }
                break;
            }
        }

        if (status == CAM_DMA_NO_ERROR)
        {
            status = lDRV_CRYPTO_DMA_EngineIsSupported(engine);
            if (status == CAM_DMA_NO_ERROR)
            {
                dma->engine = engine;
                dma->state = DMA_BUSY;
            }
        }

        if (status == CAM_DMA_NO_ERROR)
        {
            // Set the signature to identify this as a DMA context.
            dma->signature = DMA_CONTEXT_INIT_PATTERN;
        }
    }

    return status;
}

void DRV_CRYPTO_DMA_Enable(void)
{
    CAMCON.bits.ON = 1;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_Reset(void)
{
    CAM_DMA_ERROR status = CAM_DMA_RESET_FAIL_ERROR;
    int32_t timeout = DMA_TIMEOUT_LIMIT;

    // Issue the reset.
    CAMCONFIG.bits.SWRST = 1;
    CAMCONFIG.bits.SWRST = 0;

    while ((CAMSTAT.bits.SRSTBUSY != 0U) && (timeout > 0L))
    {
        timeout--;
    };

    if (!CAMSTAT.bits.SRSTBUSY)
    {
        status = CAM_DMA_NO_ERROR;
    }

    // Regardless of success, the DMA was reset (or tried to be), so the shared resource is available.
    g_dmaIsBusy = false;

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_SetTagData(DMA_CONTEXT * const contextData, TAG_LAST last, TAG_DATATYPE dataType, uint8_t invalidByteCount)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register const DMA_CONTEXT *dma = contextData;

    if ((g_dmaIsBusy == false) || (dma->state != DMA_BUSY))
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        TAG_ENGINE_SELECTION engine = dma->engine;

        if (((ENGINE_IS_HASH(engine)) && ((uint8_t) dataType >= (uint8_t) HASH_DATATYPE_LIMIT)) ||
            ((ENGINE_IS_AES(engine)) && ((uint8_t) dataType >= (uint8_t) AES_DATATYPE_LIMIT)))
        {
            status = CAM_DMA_TAG_INVALID_DATATYPE;
        }
        else
        {
            CAMFETCHTAG.val = 0;

            CAMFETCHTAG.bits.ENGSEL = engine;
            CAMFETCHTAG.bits.DLASTCLAST = last;
            CAMFETCHTAG.bits.DATCON =  0;
            CAMFETCHTAG.bits.DTYPE = dataType;
            CAMFETCHTAG.bits.INVLDBYTOFFSTRT = invalidByteCount;
        }
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_SetTagConfig(DMA_CONTEXT * const contextData, TAG_LAST last, uint32_t offset)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register const DMA_CONTEXT *dma = contextData;

    if ((g_dmaIsBusy == false) || (dma->state != DMA_BUSY))
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        TAG_ENGINE_SELECTION engine = dma->engine;

        CAMFETCHTAG.val = 0;
        CAMFETCHTAG.bits.ENGSEL = engine;
        CAMFETCHTAG.bits.DLASTCLAST = last;

        if (ENGINE_IS_AES(engine))
        {
            switch (offset)
            {
                case CONFIG_OFFSET:
                case AES_KEY_OFFSET:
                case AES_IV_OFFSET:
                case AES_MASK_OFFSET:
                    CAMFETCHTAG.val |= (offset << (uint32_t)DMA_CONFIG_TAG_OFFSET_SHIFT);
                    break;
                default:
                    status = CAM_DMA_TAG_INVALID_OFFSET;
                    break;
            }
        }

        CAMFETCHTAG.bits.DATCON = 1;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_SetFetchAddress(DMA_CONTEXT * const contextData, uint32_t address)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register const DMA_CONTEXT *dma = contextData;

    if ((g_dmaIsBusy == false) || (dma->state != DMA_BUSY))
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        CAMCONFIG.bits.PUSHSG = 0;
        CAMFETCHL = address;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_SetFetchLength(DMA_CONTEXT * const contextData, uint32_t length)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register const DMA_CONTEXT *dma = contextData;

    if ((g_dmaIsBusy == false) || (dma->state != DMA_BUSY))
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        if (length > (uint32_t) MAX_ADDRESS_LENGTH)
        {
            status = CAM_DMA_DATA_LENGTH_BEYOND_LIMIT;
        }
        else
        {
            CAMFETCHLEN.bits.LEN = length & (uint32_t) ADDRESS_LENGTH_MASK;
            CAMFETCHLEN.bits.REALIGN = 1;
        }
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_SetPushAddress(DMA_CONTEXT * const contextData, uint32_t address)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register const DMA_CONTEXT *dma = contextData;

    if ((g_dmaIsBusy == false) || (dma->state != DMA_BUSY))
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        CAMCONFIG.bits.PUSHSG = 0;
        CAMPUSHL = address;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_SetPushLength(DMA_CONTEXT * const contextData, uint32_t length)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register const DMA_CONTEXT *dma = contextData;

    if ((g_dmaIsBusy == false) || (dma->state != DMA_BUSY))
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        if (length > (uint32_t) MAX_ADDRESS_LENGTH)
        {
            status = CAM_DMA_DATA_LENGTH_BEYOND_LIMIT;
        }
        else
        {
            CAMPUSHLEN.bits.LEN = length & (uint32_t) ADDRESS_LENGTH_MASK;
        }
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_StartFetch(DMA_CONTEXT * const contextData)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if ((g_dmaIsBusy == false) || (dma->state != DMA_BUSY))
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        int32_t timeout = DMA_TIMEOUT_LIMIT;

        // The DMA operation is running.
        dma->state = DMA_RUNNING;
        CAMSTRT.bits.FETCHSTRT = 1;

        while ((CAMSTAT.bits.FETCHBUSY) && (timeout > 0L))
        {
            timeout--;
        }

        if (CAMSTAT.bits.FETCHBUSY != 0U)
        {
            (void) DRV_CRYPTO_DMA_Reset();
            status = CAM_DMA_FETCH_TIMEOUT_ERROR;
        }

        // The DMA operation is done.
        dma->state = DMA_IDLE;
        g_dmaIsBusy = false;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_StartPush(DMA_CONTEXT * const contextData)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if ((g_dmaIsBusy == false) || (dma->state != DMA_BUSY))
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        int32_t timeout = DMA_TIMEOUT_LIMIT;

        // The DMA operation is running.
        dma->state = DMA_RUNNING;
        CAMSTRT.bits.PUSHSTRT = 1;

        while ((CAMSTAT.bits.PUSHBUSY != 0U) && (timeout > 0L))
        {
            timeout--;
        }

        if (CAMSTAT.bits.PUSHBUSY != 0U)
        {
            (void) DRV_CRYPTO_DMA_Reset();
            status = CAM_DMA_PUSH_TIMEOUT_ERROR;
        }

        // The DMA operation is done.
        dma->state = DMA_IDLE;
        g_dmaIsBusy = false;
    }

    return status;
}

void DRV_CRYPTO_DMA_ResetDescriptors(DMA_CONTEXT * const contextData)
{
    register DMA_CONTEXT *dma = contextData;

    dma->state = DMA_BUSY;
    dma->inOffset = 0;
    dma->outOffset = 0;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddConfigDescriptor(DMA_CONTEXT * const contextData, const uint32_t * const config)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if (dma->inOffset != 0UL)
    {
        // The config descriptor must be the first one in the chain.
        status = CAM_DMA_INIT_ERROR;
    }
    else if (config == NULL)
    {
        status = CAM_DMA_INIT_ERROR;
    }
    else
    {
        register DMA_DESCRIPTOR *descriptor = &dma->inDescriptors[0];
        uint32_t tag = DMA_DATCON_TAG | dma->engine | DMA_CONFIG_TAG(CONFIG_OFFSET);

        lDRV_CRYPTO_DMA_AddDescriptor(descriptor, config, sizeof(uint32_t), tag, DMA_REALIGN);
        dma->inOffset++;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddKeyDescriptor(DMA_CONTEXT * const contextData, AES_KEY_NUMBER keyNumber, const void * const key, const uint32_t keyLength)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if (dma->inOffset >= DMA_IN_DESCRIPTOR_COUNT)
    {
        status = CAM_DMA_ALL_DESCRIPTORS_USED;
    }
    else if ((key == NULL) || (keyLength == 0UL))
    {
        status = CAM_DMA_INIT_ERROR;
    }
    else
    {
        register DMA_DESCRIPTOR *descriptor = &dma->inDescriptors[dma->inOffset];
        uint32_t tag;

        if (ENGINE_IS_AES(dma->engine))
        {
        tag = DMA_DATCON_TAG | dma->engine | DMA_CONFIG_TAG((keyNumber == KEY1) ? AES_KEY_OFFSET : AES_KEY2_OFFSET);
        }
        else
        {
            tag = (uint32_t)dma->engine | DMA_DATATYPE_TAG(TAG_HASH_HMAC_KEY) | DMA_TAG_LAST(TAG_IS_LAST);
        }

        lDRV_CRYPTO_DMA_AddDescriptor(descriptor, key, keyLength, tag, DMA_REALIGN);
        dma->inOffset++;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddIvDescriptor(DMA_CONTEXT * const contextData, const void * const iv, const uint32_t ivLength)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if ((iv == NULL) || (ivLength == 0UL))
    {
        // An empty initialization vector is allowed, and means no descriptor is required.
        status = CAM_DMA_NO_ERROR;
    }
    else if (!ENGINE_IS_AES(dma->engine))
    {
        // This descriptor only applies to AES modes.
        status = CAM_DMA_TAG_INVALID_FOR_THIS_ENGINE;
    }
    else if (dma->inOffset >= DMA_IN_DESCRIPTOR_COUNT)
    {
        status = CAM_DMA_ALL_DESCRIPTORS_USED;
    }
    else
    {
        register DMA_DESCRIPTOR *descriptor = &dma->inDescriptors[dma->inOffset];
        uint32_t tag = DMA_DATCON_TAG | dma->engine | DMA_CONFIG_TAG(AES_IV_OFFSET);

        lDRV_CRYPTO_DMA_AddDescriptor(descriptor, iv, ivLength, tag, DMA_REALIGN);
        dma->inOffset++;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddRawHeaderDescriptor(DMA_CONTEXT * const contextData, const void *data, const uint32_t dataLength)
{
    CAM_DMA_ERROR status = lDRV_CRYPTO_DMA_AddHeaderDescriptor(contextData, data, dataLength, 0UL);

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddHeaderDescriptor(DMA_CONTEXT * const contextData, const void *data, const uint32_t dataLength)
{
    CAM_DMA_ERROR status = lDRV_CRYPTO_DMA_AddHeaderDescriptor(contextData, data, dataLength, DMA_REALIGN);

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddRawDataDescriptor(DMA_CONTEXT * const contextData, const void * const data, const uint32_t dataLength)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if (dma->inOffset >= DMA_IN_DESCRIPTOR_COUNT)
    {
        status = CAM_DMA_ALL_DESCRIPTORS_USED;
    }
    else
    {
        register DMA_DESCRIPTOR *descriptor = &dma->inDescriptors[dma->inOffset];
        uint32_t tag = dma->engine;

        lDRV_CRYPTO_DMA_AddDescriptor(descriptor, data, dataLength, tag, 0UL);
        dma->inOffset++;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddDataDescriptor(DMA_CONTEXT * const contextData, const void * const data, const uint32_t dataLength)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if (dma->inOffset >= DMA_IN_DESCRIPTOR_COUNT)
    {
        status = CAM_DMA_ALL_DESCRIPTORS_USED;
    }
    else
    {
        register DMA_DESCRIPTOR *descriptor = &dma->inDescriptors[dma->inOffset];
        uint32_t tag = dma->engine;

        lDRV_CRYPTO_DMA_AddDescriptor(descriptor, data, dataLength, tag, DMA_REALIGN);
        dma->inOffset++;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_SetIgnoreBytes(DMA_CONTEXT * const contextData, const uint32_t ignoreByteCount)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        /* Set the last input descriptor's tag data to contain the number of bytes to ignore
            * and increase the size to align to the block size. */
        register DMA_DESCRIPTOR *last = &dma->inDescriptors[dma->inOffset - 1U];

        last->tag |= DMA_TAG_IGNOREBYTES(ignoreByteCount);
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_SetInvalidBytes(DMA_CONTEXT * const contextData, const uint32_t invalidByteCount, const uint32_t align)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else
    {
        /* Set the last input descriptor's tag data to contain the number of bytes to ignore
        * and increase the size to align to the block size. */
        register DMA_DESCRIPTOR *last = &dma->inDescriptors[dma->inOffset - 1U];

        last->tag |= DMA_TAG_IGNOREBYTES(invalidByteCount);
        last->size = (last->size + invalidByteCount) | align;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddDiscardDescriptor(DMA_CONTEXT * const contextData, const uint32_t discardLength)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if (dma->outOffset >= DMA_IN_DESCRIPTOR_COUNT)
    {
        status = CAM_DMA_ALL_DESCRIPTORS_USED;
    }
    else if (discardLength == 0UL)
    {
        status = CAM_DMA_INIT_ERROR;
    }
    else if (!ENGINE_IS_AES(dma->engine))
    {
        // Discard descriptors are only valid for AES engines.
        status = CAM_DMA_TAG_INVALID_FOR_THIS_ENGINE;
    }
    else
    {
        register DMA_DESCRIPTOR *descriptor = &dma->outDescriptors[dma->outOffset];

        descriptor->size = discardLength | DMA_DISCARD | DMA_REALIGN;
        descriptor->address = NULL;
        descriptor->tag = 0UL;

        dma->outOffset++;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_AddResultDescriptor(DMA_CONTEXT * const contextData, const void * const resultData, const uint32_t resultDataLength)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if (dma->outOffset >= DMA_IN_DESCRIPTOR_COUNT)
    {
        status = CAM_DMA_ALL_DESCRIPTORS_USED;
    }
    else if ((resultData == NULL) || (resultDataLength == 0UL))
    {
        status = CAM_DMA_INIT_ERROR;
    }
    else
    {
        register DMA_DESCRIPTOR *descriptor = &dma->outDescriptors[dma->outOffset];
        uint32_t tag = 0;

        lDRV_CRYPTO_DMA_AddDescriptor(descriptor, resultData, resultDataLength, tag, 0);
        dma->outOffset++;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_ExecuteDescriptors(DMA_CONTEXT * const contextData)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;

    if (g_dmaIsBusy == true)
    {
        // If the DMA resource is busy, the descriptor operation cannot execute.
        // Try again later to see if the resource is available.
        status = CAM_DMA_TRY_AGAIN_LATER;
    }
    else if (dma->state != DMA_BUSY)
    {
        status = CAM_DMA_BUSY_ERROR;
    }
    else if (dma->outOffset == 0UL)
    {
        /* inOffset will never be 0 (because there is always a config descriptor).
         * outOffset should not be 0 since either state is being saved or
         * a finalization is occurring. */
        status = CAM_DMA_EXECUTE_ERROR;
    }
    else
    {
        // The DMA operation is running.
        g_dmaIsBusy = true;
        dma->state = DMA_RUNNING;

        lDRV_CRYPTO_FinalizeDescriptors(dma->inDescriptors, dma->inOffset);
        lDRV_CRYPTO_FinalizeDescriptors(dma->outDescriptors, dma->outOffset);

        // Reset the DMA before use.
        (void) DRV_CRYPTO_DMA_Reset();

        // Put the device in descriptor mode.
        CAMCONFIG.bits.FETCHSG = 1;
        CAMCONFIG.bits.PUSHSG = 1;

        // Set addresses to point to the descriptor chain.
        /* cppcheck-suppress misra-c2012-11.4 */
        CAMFETCHL = (uint32_t)(uintptr_t)dma->inDescriptors;
        /* cppcheck-suppress misra-c2012-11.4 */
        CAMPUSHL = (uint32_t)(uintptr_t)dma->outDescriptors;

        /* Start the operation.
         * By ORing in the fetch/push as a value, this becomes an atomic operation. */
        CAMSTRT.val |= (DMA_FETCHSTRT | DMA_PUSHSTRT);
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_GetExecuteStatus(DMA_CONTEXT * const contextData)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register const DMA_CONTEXT *dma = contextData;
    register uint32_t state = dma->state;

    if (state == DMA_IDLE)
    {
        // DMA must be busy.
        status = CAM_DMA_INIT_ERROR;
    }
    else if (dma->state == DMA_RUNNING)
    {
        if ((CAMSTAT.val & DMA_BUSY_MASK) != 0UL)
        {
            status = CAM_DMA_BUSY_ERROR;
        }
        else if ((CAMINTSTATRAW.val & DMA_INT_ERR_MASK) != 0UL)
        {
            status = CAM_DMA_EXECUTE_ERROR;
        }
        else
        {
            status = CAM_DMA_NO_ERROR;
        }
    }
    else
    {
        status = CAM_DMA_RUNNING_ERROR;
    }

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_WaitForCompletion(DMA_CONTEXT * const contextData)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;
    uint32_t state = dma->state;

    if (state == DMA_IDLE)
    {
        // DMA must be busy.
        status = CAM_DMA_INIT_ERROR;
    }
    else if (state == DMA_RUNNING)
    {
        // Wait until not busy.
        register int32_t timeout = DMA_TIMEOUT_LIMIT;

        // To save time polling, check the status of registers directly.
        do
        {
            if ((CAMSTAT.val & DMA_BUSY_MASK) != 0UL)
            {
                status = CAM_DMA_BUSY_ERROR;
            }
            else if ((CAMINTSTATRAW.val & DMA_INT_ERR_MASK) != 0UL)
            {
                status = CAM_DMA_EXECUTE_ERROR;
            }
            else
            {
                status = CAM_DMA_NO_ERROR;
            }

            timeout--;

        } while ((status == CAM_DMA_BUSY_ERROR) && (timeout > 0L));
    }
    else
    {
        status = CAM_DMA_RUNNING_ERROR;
    }

    if (status != CAM_DMA_NO_ERROR)
    {
        // On any error, reset the DMA engine.
        (void) DRV_CRYPTO_DMA_Reset();
    }

    // Regardless, return to idle state.
    g_dmaIsBusy = false;
    dma->state = DMA_IDLE;

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_Complete(DMA_CONTEXT * const contextData)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;
    register DMA_CONTEXT *dma = contextData;
    register uint32_t state = dma->state;

    if (state == DMA_IDLE)
    {
        // DMA must be busy.
        status = CAM_DMA_INIT_ERROR;
    }
    else if (state != DMA_RUNNING)
    {
        // DMA must be running.
        status = CAM_DMA_RUNNING_ERROR;
    }
    else
    {
        // Satisfy MISRA
    }

    // Regardless, return to idle state.
    dma->state = DMA_IDLE;
    g_dmaIsBusy = false;

    return status;
}

CAM_DMA_ERROR DRV_CRYPTO_DMA_IsValidContext(const DMA_CONTEXT * const contextData)
{
    CAM_DMA_ERROR status = CAM_DMA_NO_ERROR;

    register const DMA_CONTEXT *dma = contextData;

    if (dma == NULL)
    {
        // Must exist!
        status = CAM_DMA_CONTEXT_ERROR;
    }
    /* cppcheck-suppress misra-c2012-11.4 */
    else if (0UL != ((uint32_t)dma & 0x3UL))
    {
        // Data must be 4-byte aligned.
        status = CAM_DMA_CONTEXT_ALIGNMENT_ERROR;
    }
    else if (dma->signature != DMA_CONTEXT_INIT_PATTERN)
    {
        // The signature value tells us this is a DMA context.
        status = CAM_DMA_CONTEXT_ERROR;
    }
    else
    {
    }

    return status;
}