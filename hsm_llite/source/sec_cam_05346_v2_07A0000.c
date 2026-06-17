/**************************************************************************
  CAM Register Definition Source

  Company:
    Microchip Technology Inc.

  File Name:
    sec_cam_05346_v2_07A0000.c

  Summary:
    CAM Register Definition Header File.

  Description:
    This file contains the register definitions required for interactions with the CAM.

  File created on 2026-03-26t10:36:26

  DFP Source for starting module address: dsPIC33AK-MP_DFP
  DFP Version: 1.3.185
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

#include <stdint.h>
#include "sec_cam_05346_v2_07A0000_local.h"

volatile uint32_t CAMFETCHL __attribute__((persistent, OBFUSCATE, far, address(0x7A0000)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMFETCHHBITS CAMFETCHH __attribute__((persistent, OBFUSCATE, far, address(0x7A0004)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMFETCHLENBITS CAMFETCHLEN __attribute__((persistent, OBFUSCATE, far, address(0x7A0008)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMFETCHTAGBITS CAMFETCHTAG __attribute__((persistent, OBFUSCATE, far, address(0x7A000C)));

volatile uint32_t CAMPUSHL __attribute__((persistent, OBFUSCATE, far, address(0x7A0010)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPUSHHBITS CAMPUSHH __attribute__((persistent, OBFUSCATE, far, address(0x7A0014)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPUSHLENBITS CAMPUSHLEN __attribute__((persistent, OBFUSCATE, far, address(0x7A0018)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTENBITS CAMINTEN __attribute__((persistent, OBFUSCATE, far, address(0x7A001C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTENSETBITS CAMINTENSET __attribute__((persistent, OBFUSCATE, far, address(0x7A0020)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTENCLRBITS CAMINTENCLR __attribute__((persistent, OBFUSCATE, far, address(0x7A0024)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTSTATRAWBITS CAMINTSTATRAW __attribute__((persistent, OBFUSCATE, far, address(0x7A0028)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTSTATBITS CAMINTSTAT __attribute__((persistent, OBFUSCATE, far, address(0x7A002C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTSTATCLRBITS CAMINTSTATCLR __attribute__((persistent, OBFUSCATE, far, address(0x7A0030)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMCONFIGBITS CAMCONFIG __attribute__((persistent, OBFUSCATE, far, address(0x7A0034)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMSTRTBITS CAMSTRT __attribute__((persistent, OBFUSCATE, far, address(0x7A0038)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMSTATBITS CAMSTAT __attribute__((persistent, OBFUSCATE, far, address(0x7A003C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMENGINCBITS CAMENGINC __attribute__((persistent, OBFUSCATE, far, address(0x7A0400)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMBA411CON1BITS CAMBA411CON1 __attribute__((persistent, OBFUSCATE, far, address(0x7A0404)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMBA411CON2BITS CAMBA411CON2 __attribute__((persistent, OBFUSCATE, far, address(0x7A0408)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMBA413CONBITS CAMBA413CON __attribute__((persistent, OBFUSCATE, far, address(0x7A040C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMBA418CONBITS CAMBA418CON __attribute__((persistent, OBFUSCATE, far, address(0x7A0410)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMTRNGCONBITS CAMTRNGCON __attribute__((persistent, OBFUSCATE, far, address(0x7A1000)));

volatile uint32_t CAMFIFOLVL __attribute__((persistent, OBFUSCATE, far, address(0x7A1004)));

volatile uint32_t CAMFIFOTHRESH __attribute__((persistent, OBFUSCATE, far, address(0x7A1008)));

volatile uint32_t CAMFIFODEP __attribute__((persistent, OBFUSCATE, far, address(0x7A100C)));

volatile uint32_t CAMKEY0 __attribute__((persistent, OBFUSCATE, far, address(0x7A1010)));

volatile uint32_t CAMKEY1 __attribute__((persistent, OBFUSCATE, far, address(0x7A1014)));

volatile uint32_t CAMKEY2 __attribute__((persistent, OBFUSCATE, far, address(0x7A1018)));

volatile uint32_t CAMKEY3 __attribute__((persistent, OBFUSCATE, far, address(0x7A101C)));

volatile uint32_t CAMTSTDAT __attribute__((persistent, OBFUSCATE, far, address(0x7A1020)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMREPTHRESHBITS CAMREPTHRESH __attribute__((persistent, OBFUSCATE, far, address(0x7A1024)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPROPTHRESHBITS CAMPROPTHRESH __attribute__((persistent, OBFUSCATE, far, address(0x7A1028)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMTRNGSTATBITS CAMTRNGSTAT __attribute__((persistent, OBFUSCATE, far, address(0x7A1030)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINITWAITBITS CAMINITWAIT __attribute__((persistent, OBFUSCATE, far, address(0x7A1034)));

volatile uint32_t CAMDISOSC0 __attribute__((persistent, OBFUSCATE, far, address(0x7A1038)));

volatile uint32_t CAMDISOSC1 __attribute__((persistent, OBFUSCATE, far, address(0x7A103C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMSWOFFTMRBITS CAMSWOFFTMR __attribute__((persistent, OBFUSCATE, far, address(0x7A1040)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMCLKDIVBITS CAMCLKDIV __attribute__((persistent, OBFUSCATE, far, address(0x7A1044)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMHWCFGBITS CAMHWCFG __attribute__((persistent, OBFUSCATE, far, address(0x7A1058)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKPTRBITS CAMPKPTR __attribute__((persistent, OBFUSCATE, far, address(0x7A2000)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKCMDBITS CAMPKCMD __attribute__((persistent, OBFUSCATE, far, address(0x7A2004)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKCONBITS CAMPKCON __attribute__((persistent, OBFUSCATE, far, address(0x7A2008)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKSTATBITS CAMPKSTAT __attribute__((persistent, OBFUSCATE, far, address(0x7A200C)));

volatile uint32_t CAMPKTMR __attribute__((persistent, OBFUSCATE, far, address(0x7A2014)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKCONHWBITS CAMPKCONHW __attribute__((persistent, OBFUSCATE, far, address(0x7A2018)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKOPSZBITS CAMPKOPSZ __attribute__((persistent, OBFUSCATE, far, address(0x7A201C)));

volatile uint32_t CAMPKMEMOFF __attribute__((persistent, OBFUSCATE, far, address(0x7A2020)));

volatile uint32_t CAMPKMICOFF __attribute__((persistent, OBFUSCATE, far, address(0x7A2024)));

#ifndef CAMCON
/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMCONBITS CAMCON __attribute__((persistent, OBFUSCATE, far, address(0x7A8000)));
#endif // CAMCON

/**
 End of File
*/
