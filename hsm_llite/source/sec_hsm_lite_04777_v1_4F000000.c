/**************************************************************************
CAM Register Definition Source

  Company:
    Microchip Technology Inc.

  File Name:
    sec_hsm_lite_04777_v1_4F000000.c

  Summary:
    CAM Register definition header file

  Description:
    This file contains all the register definitions required for interactions with the CAM.

  File created on 2025-01-28t09:26:32

  DFP Source for starting module address: PIC32CM-SG00_DFP
  DFP Version: 0.1.289
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
#include "sec_hsm_lite_04777_v1_4F000000_local.h"

volatile uint32_t CAMFETCHL __attribute__((persistent, address(0x4f010000)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMFETCHHBITS CAMFETCHH __attribute__((persistent, address(0x4f010004)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMFETCHLENBITS CAMFETCHLEN __attribute__((persistent, address(0x4f010008)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMFETCHTAGBITS CAMFETCHTAG __attribute__((persistent, address(0x4f01000C)));

volatile uint32_t CAMPUSHL __attribute__((persistent, address(0x4f010010)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPUSHHBITS CAMPUSHH __attribute__((persistent, address(0x4f010014)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPUSHLENBITS CAMPUSHLEN __attribute__((persistent, address(0x4f010018)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTENBITS CAMINTEN __attribute__((persistent, address(0x4f01001C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTENSETBITS CAMINTENSET __attribute__((persistent, address(0x4f010020)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTENCLRBITS CAMINTENCLR __attribute__((persistent, address(0x4f010024)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTSTATRAWBITS CAMINTSTATRAW __attribute__((persistent, address(0x4f010028)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTSTATBITS CAMINTSTAT __attribute__((persistent, address(0x4f01002C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINTSTATCLRBITS CAMINTSTATCLR __attribute__((persistent, address(0x4f010030)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMCONFIGBITS CAMCONFIG __attribute__((persistent, address(0x4f010034)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMSTRTBITS CAMSTRT __attribute__((persistent, address(0x4f010038)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMSTATBITS CAMSTAT __attribute__((persistent, address(0x4f01003C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMENGINCBITS CAMENGINC __attribute__((persistent, address(0x4f010400)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMBA411CON1BITS CAMBA411CON1 __attribute__((persistent, address(0x4f010404)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMBA411CON2BITS CAMBA411CON2 __attribute__((persistent, address(0x4f010408)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMBA413CONBITS CAMBA413CON __attribute__((persistent, address(0x4f01040C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMBA418CONBITS CAMBA418CON __attribute__((persistent, address(0x4f010410)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMTRNGCONBITS CAMTRNGCON __attribute__((persistent, address(0x4f011000)));

volatile uint32_t CAMFIFOLVL __attribute__((persistent, address(0x4f011004)));

volatile uint32_t CAMFIFOTHRESH __attribute__((persistent, address(0x4f011008)));

volatile uint32_t CAMFIFODEP __attribute__((persistent, address(0x4f01100C)));

volatile uint32_t CAMKEY0 __attribute__((persistent, address(0x4f011010)));

volatile uint32_t CAMKEY1 __attribute__((persistent, address(0x4f011014)));

volatile uint32_t CAMKEY2 __attribute__((persistent, address(0x4f011018)));

volatile uint32_t CAMKEY3 __attribute__((persistent, address(0x4f01101C)));

volatile uint32_t CAMTSTDAT __attribute__((persistent, address(0x4f011020)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMREPTHRESHBITS CAMREPTHRESH __attribute__((persistent, address(0x4f011024)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPROPTHRESHBITS CAMPROPTHRESH __attribute__((persistent, address(0x4f011028)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMTRNGSTATBITS CAMTRNGSTAT __attribute__((persistent, address(0x4f011030)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMINITWAITBITS CAMINITWAIT __attribute__((persistent, address(0x4f011034)));

volatile uint32_t CAMDISOSC0 __attribute__((persistent, address(0x4f011038)));

volatile uint32_t CAMDISOSC1 __attribute__((persistent, address(0x4f01103C)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMSWOFFTMRBITS CAMSWOFFTMR __attribute__((persistent, address(0x4f011040)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMCLKDIVBITS CAMCLKDIV __attribute__((persistent, address(0x4f011044)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMHWCFGBITS CAMHWCFG __attribute__((persistent, address(0x4f011058)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKPTRBITS CAMPKPTR __attribute__((persistent, address(0x4f012000)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKCMDBITS CAMPKCMD __attribute__((persistent, address(0x4f012004)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKCONBITS CAMPKCON __attribute__((persistent, address(0x4f012008)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKSTATBITS CAMPKSTAT __attribute__((persistent, address(0x4f01200C)));

volatile uint32_t CAMPKTMR __attribute__((persistent, address(0x4f012014)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKCONHWBITS CAMPKCONHW __attribute__((persistent, address(0x4f012018)));

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMPKOPSZBITS CAMPKOPSZ __attribute__((persistent, address(0x4f01201C)));

volatile uint32_t CAMPKMEMOFF __attribute__((persistent, address(0x4f012020)));

volatile uint32_t CAMPKMICOFF __attribute__((persistent, address(0x4f012024)));


#ifndef CAMCON

/* cppcheck-suppress misra-c2012-19.2; false positive */
volatile CAMCONBITS CAMCON __attribute__((persistent, address(0x4F000000)));
#endif //CAMCON
/**
End of File
*/