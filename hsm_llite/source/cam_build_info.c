/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_build_info.c

  Summary:
    CAM Hardware Driver Version.

  Description:
    This file contains the build information of the CAM Hardware Driver.
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

#include <stdint.h>

/** @cond INTERNAL **/
/**
 * @ingroup build_info
 * @def CAM_LIBRARY_IDENTIFIER
 * @brief This field represents a build identifier of the CAM library.
 */
static const uint64_t __attribute__((used)) CAM_LIBRARY_IDENTIFIER = 0x5F7E45AB823BCE84ULL;
/** @endcond **/
