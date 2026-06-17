/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_aes_local.h

  Summary:
    Crypto Framework Library interface file for local CAM AES definitions.

  Description:
    This header file contains the interface that provides local definitions
    for Microchip microcontrollers equipped with a Crypto Accelerator Module.
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

#ifndef CAM_AES_LOCAL_H
#define	CAM_AES_LOCAL_H

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "cam_dma_local.h"
#include "cam_local.h"

// *****************************************************************************
// *****************************************************************************
// Section: AES Common Data Types
// *****************************************************************************
// *****************************************************************************

// State context block sizes for AES.
#define AES_STATE_CONTEXT_SIZE        (32U)  // bytes
#define AES_SMALL_STATE_CONTEXT_SIZE  (16U)  // bytes
#define CMAC_STATE_CONTEXT_SIZE       (16U)  // bytes
#define STATE_CONTEXT_SIZE            (AES_STATE_CONTEXT_SIZE)

#define AES_KEY_128_SIZE        (16U)
#define AES_KEY_192_SIZE        (24U)
#define AES_KEY_256_SIZE        (32U)

#define AES_GCM_AUTHTAG_SIZE    (16U)

// Size of the lenA/lenC component in a LenALenC calculation.
#define AES_LENALENC_SIZE       (sizeof(uint64_t))

// This pattern indicates a context was intiialized.
#define AES_CONTEXT_INIT_PATTERN   (0xabaddeedUL)

/**
 * @brief Defines the data types specified to the DMA engine.
 */
typedef enum AES_DATATYPE
{
    PAYLOAD_DATATYPE = 0,
    HEADER_DATATYPE = 1,
} AES_DATATYPE;

/**
 * @brief Defines the AES context block.
 * @note If you add to this structure, you must verify the wrapper reserves
 *       sufficient space for the context block in the CCv4 context by
 *       updating the minimum size definition (since the upper layer cannot
 *       see this structure, it cannot know how big it is at compilation time).
 **/
typedef struct AES_CONTEXT {

    // If not the init pattern, then this context has not been initialized.
    uint32_t        initialized;

    // Key to be used for this operation.
    uint8_t *       key;

    // Configuration data for the DMA's AES operation.
    // This must persist until the operation is complete.
    AES_CONFIG      config;

    uint32_t        keyLength: 7; // Key length in bytes.
    uint32_t        op       : 1; // Operation (encrypt = 0: decrypt = 1).
    uint32_t        mode     : 4; // AES mode shift.
    uint32_t        blockSize: 6; // Size of the AES mode's data block.
    uint32_t        stateSize: 6; // Size of the AES mode's state data.
    uint32_t        left     : 6; // Remaining data in descriptor chain to process.

    // Total input data added to this operation.
    uint32_t        dataLength;

    // Amount of Additional header data (such as AAD) included in this operation.
    // Generally applicable to AES-GCM mode.
    uint32_t        headerLength;

    // DMA context state data.
    DMA_CONTEXT     dma;

    // Context state data (partial MAC).
    // Placing this at the end allows for slight memory optimization for CMAC contexts.
    uint8_t         state[STATE_CONTEXT_SIZE];

} AES_CONTEXT;

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif	/* CAM_AES_LOCAL_H */
