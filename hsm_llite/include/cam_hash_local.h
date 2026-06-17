/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_hash_local.h

  Summary:
    Crypto Framework Library interface file for local CAM HASH definitions.

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

#ifndef CAM_HASH_LOCAL_H
#define	CAM_HASH_LOCAL_H

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "cam_dma_local.h"
#include "cam_local.h"

// *****************************************************************************
// *****************************************************************************
// Section: HASH Common Data Types
// *****************************************************************************
// *****************************************************************************

// Maximum size of any 'invalid' bytes in the final data descriptor.
#define HASH_INVALID_BYTES  (4U)

#if defined(GENERIC_TARGET_CAM_06048)

// Maximum hash state size.
#define HASH_MAX_STATE_SIZE (SHAKE_STATE_CONTEXT_SIZE)

// Maximum hash block size.
#define HASH_MAX_BLOCK_SIZE (SHAKE128_BLOCK_SIZE)

#else

// Maximum hash state size.
#define HASH_MAX_STATE_SIZE (SHA512_STATE_CONTEXT_SIZE)

// Maximum hash block size.
#define HASH_MAX_BLOCK_SIZE (SHA512_BLOCK_SIZE)

#endif

// Maximum hash block padding size.  The extra bytes hold the pad indicator.
#define HASH_MAX_ENCLEN_SIZE ((uint32_t)SHA512_ENCLEN_SIZE + sizeof(uint32_t))

/* Maximum size of the operation's Working data area, This area contains the
 * data cache, padding, and state data. */
#define HASH_MAX_WORKING_AREA_SIZE ((uint32_t)HASH_MAX_BLOCK_SIZE + \
                                    (uint32_t)HASH_MAX_STATE_SIZE + \
                                    ((uint32_t)HASH_MAX_BLOCK_SIZE + (uint32_t)HASH_MAX_ENCLEN_SIZE))

// This pattern indicates a context was initialized.
#define HASH_CONTEXT_INIT_PATTERN   (0xfeedfaceUL)

/**
 * @brief Defines block sizes for HASH modes.
 */
typedef enum HASHCON_BLOCK_SIZE
{
    SHA1_BLOCK_SIZE = 64,
    SHA224_BLOCK_SIZE = 64,
    SHA256_BLOCK_SIZE = 64,
    SHA384_BLOCK_SIZE = 128,
    SHA512_BLOCK_SIZE = 128,

    #if defined(GENERIC_TARGET_CAM_06048)

    SHA3_224_BLOCK_SIZE = 144,
    SHA3_256_BLOCK_SIZE = 136,
    SHA3_384_BLOCK_SIZE = 104,
    SHA3_512_BLOCK_SIZE = 72,
    SHAKE128_BLOCK_SIZE = 168,
    SHAKE256_BLOCK_SIZE = 136

    #endif

} HASHCON_BLOCK_SIZE;

/**
 * @brief Defines state context sizes for HASH modes.
 */
typedef enum HASHCON_STATE_SIZE
{
    SHA1_STATE_CONTEXT_SIZE = 20,
    SHA224_STATE_CONTEXT_SIZE = 32,
    SHA256_STATE_CONTEXT_SIZE = 32,
    SHA384_STATE_CONTEXT_SIZE = 64,
    SHA512_STATE_CONTEXT_SIZE = 64,

    #if defined(GENERIC_TARGET_CAM_06048)

    SHA3_STATE_CONTEXT_SIZE = 200,  // SHA3-224/256/384/512
    SHAKE_STATE_CONTEXT_SIZE = 200, // SHAKE128/256

    #endif

} HASHCON_STATE_SIZE;

/**
 * @brief Defines digest sizes for HASH modes.
 */
typedef enum HASHCON_DIGEST_SIZE
{
    SHA1_DIGEST_SIZE = 20,
    SHA224_DIGEST_SIZE = 28,
    SHA256_DIGEST_SIZE = 32,
    SHA384_DIGEST_SIZE = 48,
    SHA512_DIGEST_SIZE = 64,

    #if defined(GENERIC_TARGET_CAM_06048)

    SHA3_224_DIGEST_SIZE = 28,
    SHA3_256_DIGEST_SIZE = 32,
    SHA3_384_DIGEST_SIZE = 48,
    SHA3_512_DIGEST_SIZE = 64

    #endif

} HASHCON_DIGEST_SIZE;

/**
 * @brief Defines encoded length sizes for SHA1/SHA2 HASH modes.
 */
typedef enum HASHCON_ENCLEN_SIZE
{
    SHA1_ENCLEN_SIZE = 8,
    SHA224_ENCLEN_SIZE = 8,
    SHA256_ENCLEN_SIZE = 8,
    SHA384_ENCLEN_SIZE = 16,
    SHA512_ENCLEN_SIZE = 16

} HASHCON_ENCLEN_SIZE;

/**
 * @brief Defines execution state for SHA1/SHA2 HASH operations.
 **/
typedef enum HASHCON_FINAL
{
    STATE = 0,  // Saved the state of the partial hash operation
    DIGEST = 1  // Compute the digest.
} HASHCON_FINAL;

#if defined(GENERIC_TARGET_CAM_06048)

/**
 * @brief Defines context save state for SHA3/SHAKE HASH operations.
 *        The BA418 SHA3 engine uses inverted polarity compared to the BA413 SHA1/SHA2
 *        engine.
 **/
typedef enum HASHCON_SAVE_CONTEXT
{
    SAVE_CONTEXT_OFF = 0,  // Output is the digest
    SAVE_CONTEXT_ON  = 1   // Saved the state of the partial hash operation
} HASHCON_SHA3_SAVE_CONTEXT;

#endif

/**
 * @brief Defines the padding mode to use for HASH oeprations.
 **/
typedef enum HASHCON_PADDING
{
    SW_PADDING = 0, // Execute a software pad operation
    HW_PADDING = 1  // Allow the hardware to calculate padding.
} HASHCON_PADDING;

/**
 * @brief Defines whether HMAC is enabled for this HASH operation.
 **/
typedef enum HASHCON_HMAC
{
    HMAC_OFF = 0,
    HMAC_ON = 1
} HASHCON_HMAC;

#if defined(GENERIC_TARGET_CAM_06048)

/**
 * @brief Defines the algorithm capacity of this SHA3/SHAKE HASH operation.
 **/
typedef enum HASHCON_CAPACITY {
    CAPACITY_256 = 3,   // SHAKE128
    CAPACITY_448 = 6,   // SHA3-224
    CAPACITY_512 = 7,   // SHA3-256/SHAKE256
    CAPACITY_768 = 11,  // SHA3-384
    CAPACITY_1024 = 15  // SHA3-512
} HASHCON_CAPACITY;

/**
 * @brief Defines whether SHAKE is enabled for this HASH operation.
 **/
typedef enum HASHCON_SHAKE
{
    SHAKE_DISABLE = 0,
    SHAKE_ENABLE = 1
} HASHCON_SHAKE;

#endif

/**
 * @brief Defines the HASH context block.  The data space for this context is
 *        allocated by callers of the functions in this file.
 * @note If you add to this structure, you must verify the wrapper reserves
 *       sufficient space for the context block in the CCv4 context by
 *       updating the minimum size definition (since the upper layer cannot
 *       see this structure, it cannot know how big it is at compilation time).
 **/
typedef struct HASH_CONTEXT {

    // If not the init pattern, then this context has not been initialized.
    uint32_t         initialized;

    // Configuration data for the SHA1/SHA2 HASH operation.
    // This must persist until the operation is complete.
    HASH_CONFIG      config;

    #if defined(GENERIC_TARGET_CAM_06048)

    // Configuration data for the SHA3/SHAKE HASH operation.
    HASH_CONFIG_SHA3 configSha3;

    #endif

    uint32_t         mode          : 4;    // Mode shift.
    uint32_t         hmac          : 1;    // Hash HMAC is to be generated.
    uint32_t         partialHash   : 1;    // If set, a partial hash is being generated.
    uint32_t                       : 2;    // Unused for future needs.
    uint32_t         blockSize     : 8;    // Size of the hash mode's data block.
    uint32_t         stateSize     : 8;    // Size of the hash mode's state data.
    uint32_t         left          : 8;    // Remaining data in cache to process.

    uint32_t         digestSize    : 16;   // Size of the digest for this operation.
    uint32_t         encLengthSize : 6;    // Size of the encoded length data. (SHA1/SHA2 only)

    #if defined(GENERIC_TARGET_CAM_06048)

    uint32_t         shakeConfig   : 1;    // If set, this context is initialized for SHAKE.
    uint32_t                       : 9;    // Unused for future needs.

    #else

    uint32_t                       : 10;   // Unused for future needs.
    
    #endif

    // Holds the total number of bytes hashed.
    uint32_t         dataLength;

    // The DMA context for this operation.
    DMA_CONTEXT      dma;

    /* Hash data cache, used to submit complete blocks.
     * This points to a location in the working data area. */
    uint8_t          *cache;

    /* Context state data (partial hash).
     * This points to a location in the working data area. */
    uint8_t          *state;

    /* Hash pad cache, used to pad after a partial-hash calculation.
     * A full block is needed in case a full block of padding must be added.
     * The added padding space holds to encoded length.
     * This points to a location in the working data area. */
    uint8_t          *padding;

    /* Working data area, to contain the data cache, padding, and state data.
     * Using a working data block allows potential reduction in working space
     * for hash algorithms that use smaller block and state sizes.
     * Each respective use will set a data pointer. */
    uint8_t          opData[HASH_MAX_WORKING_AREA_SIZE];

} HASH_CONTEXT;

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif	/* CAM_HASH_LOCAL_H */
