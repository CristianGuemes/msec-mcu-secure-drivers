/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_trng_local.h
    
  Summary:
    Crypto Framework Library interface file for CAM TRNG.

  Description:
    This header file contains the interface that make up the CAM TRNG hardware 
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

#ifndef CAM_TRNG_LOCAL_H
#define	CAM_TRNG_LOCAL_H

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include "../inc/cam_version.h"

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

#define OFF_BIT 0U
#define ON_BIT  1U

#define SETUPKEY_MAX_SIZE   16U
#define WORD_SIZE  4U

#define KEY_SIZE 16U
#define TEST_RESULT_SIZE 4U
#define MAX_FIFO_LEVEL 8U
#define TEST_ENABLE_MASK 0x4U
#define CAM_ENABLE_MASK 0x8000U
#define CAM_TRNG_SETUP_MASK 0x630U
#define SAFE_READ_ADDRESS (uint32_t *)99U

#define FIFO_THRESHOLD 3U
#define NUMBER_128_BLOCKS 4U
#define COOLDOWN_PERIOD 0U
#define SAMPLING_PERIOD 10U

/*  
 *  This change is due do the difference in how the versions handle timing. For DOS-05346, the INITIAL_WAIT is the only timing control available. 
 *  With DOS-06048, the INITIAL_WAIT is not as important since it is not the only control, so it can be smaller. 
 */
#if defined(GENERIC_TARGET_CAM_05346) || defined(GENERIC_TARGET_HSM_LITE_04777)
#define INITIAL_WAIT 256U
#define GENERATION_TIMEOUT  512U
#elif defined(GENERIC_TARGET_CAM_06048)
#define INITIAL_WAIT 16U
#define GENERATION_TIMEOUT  4096U
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

typedef struct TRNG_DATA 
{
    uint8_t* data;
    uint32_t size;
} TRNG_DATA;

typedef struct TRNG_CTX
{
    bool conditioningBypass;
    bool fifoWriteStartup;  
    bool htestAfterCond;
    uint8_t nb128Block;
    uint8_t fifoThreshold;
    uint16_t offTmr;
    uint8_t clkDiv;
    uint16_t initialWait;
    TRNG_DATA key;
#ifdef GENERIC_TARGET_CAM_06048
    bool startupBypass; 
    bool pseudoMode;
    uint16_t coolDownPeriod;
    uint16_t samplingPeriod;
    uint8_t blendMode;
#endif
} TRNG_CTX;

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif // CAM_TRNG_LOCAL_H