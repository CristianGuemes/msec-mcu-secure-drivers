/**************************************************************************
CAM Register Definition Header

  Company:
    Microchip Technology Inc.

  File Name:
    sec_hsm_lite_04777_v1_4F000000_local.h

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

/* MISRA C:2012 Rule 19.2 and 21.1 Deviation Justification
 *
 * ID      Category   Description
 * 19.2    Advisory   The union keyword should not be used
 * 21.1    Required   Macro names shall not be the same as keywords (or reserved identifiers)
 *
 * Justification:
 * - Rule 19.2: These register definition files use unions in the associated header
 *   structures to accurately map hardware registers and bitfields as
 *   defined in the device’s DFP. This is required for correct low-level
 *   access to individual bits as well as the full register value.
 *   The use of unions in this context is intentional, hardware-driven,
 *   and does not introduce ambiguity or undefined behavior.
 *
 * - Rule 21.1: The macro __attribute__ is intentionally defined for compatibility with
 *   non-GCC/non-XC compilers, allowing vendor attribute annotations to be safely ignored.
 *   This usage is restricted to cross-platform headers and is required for third-party integration
 *   with generated register maps. The use of a reserved identifier is necessary due to external code
 *   requirements and is tightly scoped to this header.
 *
 * Scope:
 * This justification applies to all register definition files that
 * interact directly with CAM/HSM hardware registers and all
 * instances of reserved macro definitions that support cross-compiler compatibility.
 */

/* MISRA C:2012 Rule 21.1 Deviation Justification
 *
 * ID      Category   Description
 * 21.1    Required   define and undef directives shall not be used on a
 *                    reserved identifier or reserved macro name
 *
 * Justification:
 * When unit testing the CAM source on the PC, redefining `attribute`
 * allows to quickly disable XC compiler specific settings and allow the
 * build by GCC.
 *
 * Scope:
 * This justification applies to all redefined macros via a check for XC compiler.
 */

#ifndef SEC_HSM_LITE_04777_V1_4F000000_LOCAL_H
#define SEC_HSM_LITE_04777_V1_4F000000_LOCAL_H

#ifdef __cplusplus  // Provide C++ Compatibility
    extern "C" {
#endif

#include <stdint.h>

#define PIC32CM_HSM_PKE_MICROCODE_ROM_ADDR      (0x0400DFE0UL)
#define CAMCON_VAL_INIT         0x2U


#ifndef __XC__
  /* cppcheck-suppress misra-c2012-21.1 */
  #define __attribute__(x)
#endif

extern const uint32_t* const pkeMicrocode;
extern volatile uint32_t CAMFETCHL;


/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMFETCHHBITS
{
    uint32_t val;
    struct {
      uint32_t STOP:1;
      uint32_t :1;
      uint32_t NEXT:30;
    } bits;
} CAMFETCHHBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMFETCHHBITS CAMFETCHH;


/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMFETCHLENBITS
{
    uint32_t val;
    struct {
      uint32_t LEN:28;
      uint32_t CNSTADDR:1;
      uint32_t REALIGN:1;
      uint32_t DISCARD:1;
      uint32_t INTEN:1;
    } bits;
} CAMFETCHLENBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMFETCHLENBITS CAMFETCHLEN;


/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMFETCHTAGBITS
{
    uint32_t val;
    struct {
      uint32_t ENGSEL:4;
      uint32_t DATCON:1;
      uint32_t DLASTCLAST:1;
      uint32_t DTYPE:2;
      uint32_t INVLDBYTOFFSTRT:6;
      uint32_t :18;
    } bits;
} CAMFETCHTAGBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMFETCHTAGBITS CAMFETCHTAG;

extern volatile uint32_t CAMPUSHL;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPUSHHBITS
{
    uint32_t val;
    struct {
      uint32_t STOP:1;
      uint32_t :1;
      uint32_t NEXT:30;
    } bits;
} CAMPUSHHBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPUSHHBITS CAMPUSHH;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPUSHLENBITS
{
    uint32_t val;
    struct {
      uint32_t LEN:28;
      uint32_t CNSTADDR:1;
      uint32_t REALIGN:1;
      uint32_t DISCARD:1;
      uint32_t INTEN:1;
    } bits;
} CAMPUSHLENBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPUSHLENBITS CAMPUSHLEN;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMINTENBITS
{
    uint32_t val;
    struct {
      uint32_t FEOBINTEN:1;
      uint32_t FSTOPINTEN:1;
      uint32_t FERRINTEN:1;
      uint32_t PEOBINTEN:1;
      uint32_t PSTOPINTEN:1;
      uint32_t PERRINTEN:1;
      uint32_t :26;
    } bits;
} CAMINTENBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMINTENBITS CAMINTEN;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMINTENSETBITS
{
    uint32_t val;
    struct {
      uint32_t FEOBINTENSET:1;
      uint32_t FSTOPINTENSET:1;
      uint32_t FERRINTENSET:1;
      uint32_t PEOBINTENSET:1;
      uint32_t PSTOPINTENSET:1;
      uint32_t PERRINTENSET:1;
      uint32_t :26;
    } bits;
} CAMINTENSETBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMINTENSETBITS CAMINTENSET;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMINTENCLRBITS
{
    uint32_t val;
    struct {
      uint32_t FEOBINTENCLR:1;
      uint32_t FSTOPINTENCLR:1;
      uint32_t FERRINTENCLR:1;
      uint32_t PEOBINTENCLR:1;
      uint32_t PSTOPINTENCLR:1;
      uint32_t PERRINTENCLR:1;
      uint32_t :26;
    } bits;
} CAMINTENCLRBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMINTENCLRBITS CAMINTENCLR;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMINTSTATRAWBITS
{
    uint32_t val;
    struct {
      uint32_t FEOBINTSTATR:1;
      uint32_t FSTOPINTSTATR:1;
      uint32_t FERRINTSTATR:1;
      uint32_t PEOBINTSTATR:1;
      uint32_t PSTOPINTSTATR:1;
      uint32_t PERRINTSTATR:1;
      uint32_t :26;
    } bits;
} CAMINTSTATRAWBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMINTSTATRAWBITS CAMINTSTATRAW;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMINTSTATBITS
{
    uint32_t val;
    struct {
      uint32_t FEOBINTSTAT:1;
      uint32_t FSTOPINTSTAT:1;
      uint32_t FERRINTSTAT:1;
      uint32_t PEOBINTSTAT:1;
      uint32_t PSTOPINTSTAT:1;
      uint32_t PERRINTSTAT:1;
      uint32_t :26;
    } bits;
} CAMINTSTATBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMINTSTATBITS CAMINTSTAT;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMINTSTATCLRBITS
{
    uint32_t val;
    struct {
      uint32_t FEOBINTSTATC:1;
      uint32_t FSTOPINTSTATC:1;
      uint32_t FERRINTSTATC:1;
      uint32_t PEOBINTSTATC:1;
      uint32_t PSTOPINTSTATC:1;
      uint32_t PERRINTSTATC:1;
      uint32_t :26;
    } bits;
} CAMINTSTATCLRBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMINTSTATCLRBITS CAMINTSTATCLR;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMCONFIGBITS
{
    uint32_t val;
    struct {
      uint32_t FETCHSG:1;
      uint32_t PUSHSG:1;
      uint32_t FETCHSTOP:1;
      uint32_t PUSHSTOP:1;
      uint32_t SWRST:1;
      uint32_t :27;
    } bits;
} CAMCONFIGBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMCONFIGBITS CAMCONFIG;


/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMSTRTBITS
{
    uint32_t val;
    struct {
      uint32_t FETCHSTRT:1;
      uint32_t PUSHSTRT:1;
      uint32_t :30;
    } bits;
} CAMSTRTBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMSTRTBITS CAMSTRT;


/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMSTATBITS
{
    uint32_t val;
    struct {
      uint32_t FETCHBUSY:1;
      uint32_t PUSHBUSY:1;
      uint32_t :2;
      uint32_t FETCHNEMP:1;
      uint32_t PUSHWAIT:1;
      uint32_t SRSTBUSY:1;
      uint32_t :9;
      uint32_t NUMDAT:16;
    } bits;
} CAMSTATBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMSTATBITS CAMSTAT;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMENGINCBITS
{
    uint32_t val;
    struct {
      uint32_t BA411EN:1;
      uint32_t BA415EN:1;
      uint32_t BA416EN:1;
      uint32_t BA412EN:1;
      uint32_t BA413EN:1;
      uint32_t BA417EN:1;
      uint32_t BA418EN:1;
      uint32_t BA421EN:1;
      uint32_t BA419EN:1;
      uint32_t BA414EPEN:1;
      uint32_t BA431EN:1;
      uint32_t BA420EN:1;
      uint32_t BA423EN:1;
      uint32_t BA422EN:1;
      uint32_t :18;
    } bits;
} CAMENGINCBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMENGINCBITS CAMENGINC;


/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMBA411CON1BITS
{
    uint32_t val;
    struct {
      uint32_t AESPOSS:9;
      uint32_t :7;
      uint32_t CS:1;
      uint32_t MASK:1;
      uint32_t :6;
      uint32_t KEYSIZE:3;
      uint32_t CXSW:1;
      uint32_t GLITCHPROT:1;
      uint32_t :3;
    } bits;
} CAMBA411CON1BITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMBA411CON1BITS CAMBA411CON1;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMBA411CON2BITS
{
    uint32_t val;
    struct {
      uint32_t CTRSIZE:16;
      uint32_t :4;
      uint32_t EXT:4;
      uint32_t IKG:4;
      uint32_t :4;
    } bits;
} CAMBA411CON2BITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMBA411CON2BITS CAMBA411CON2;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMBA413CONBITS
{
    uint32_t val;
    struct {
      uint32_t HASHMASK:7;
      uint32_t :9;
      uint32_t HASHPAD:1;
      uint32_t HMACEN:1;
      uint32_t HASHVER:1;
      uint32_t :1;
      uint32_t EXT:4;
      uint32_t IKG:4;
      uint32_t :4;
    } bits;
} CAMBA413CONBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMBA413CONBITS CAMBA413CON;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMBA418CONBITS
{
    uint32_t val;
    struct {
      uint32_t SHA3EN:1;
      uint32_t :31;
    } bits;
} CAMBA418CONBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMBA418CONBITS CAMBA418CON;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMTRNGCONBITS
{
    uint32_t val;
    struct {
      uint32_t NDRNDEN:1;
      uint32_t LFSREN:1;
      uint32_t TESTEN:1;
      uint32_t CONDBYPASS:1;
      uint32_t INTENREP:1;
      uint32_t INTENPROP:1;
      uint32_t :1;
      uint32_t INTENFULL:1;
      uint32_t SOFTRST:1;
      uint32_t INTENPRE:1;
      uint32_t INTENALM:1;
      uint32_t FORCERUN:1;
      uint32_t HLTHTSTBYPASS:1;
      uint32_t AIS31BYPASS:1;
      uint32_t HLTHTSTSEL:1;
      uint32_t AIS31TSTSEL:1;
      uint32_t NB128BLOCK:4;
      uint32_t FIFOWRTSTRT:1;
      uint32_t :11;
    } bits;
} CAMTRNGCONBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMTRNGCONBITS CAMTRNGCON;

extern volatile uint32_t CAMFIFOLVL;

extern volatile uint32_t CAMFIFOTHRESH;

extern volatile uint32_t CAMFIFODEP;

extern volatile uint32_t CAMKEY0;

extern volatile uint32_t CAMKEY1;

extern volatile uint32_t CAMKEY2;

extern volatile uint32_t CAMKEY3;

extern volatile uint32_t CAMTSTDAT;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMREPTHRESHBITS
{
    uint32_t val;
    struct {
      uint32_t REPTHRESH:6;
      uint32_t :26;
    } bits;
} CAMREPTHRESHBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMREPTHRESHBITS CAMREPTHRESH;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPROPTHRESHBITS
{
    uint32_t val;
    struct {
      uint32_t PROPTHRESH:10;
      uint32_t :22;
    } bits;
} CAMPROPTHRESHBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPROPTHRESHBITS CAMPROPTHRESH;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMTRNGSTATBITS
{
    uint32_t val;
    struct {
      uint32_t TSTDATBSY:1;
      uint32_t STATE:3;
      uint32_t REPERR:1;
      uint32_t PROPERR:1;
      uint32_t :1;
      uint32_t FULLINT:1;
      uint32_t PREINT:1;
      uint32_t ALMINT:1;
      uint32_t STRTUPERR:1;
      uint32_t FIFOACCERR:1;
      uint32_t :20;
    } bits;
} CAMTRNGSTATBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMTRNGSTATBITS CAMTRNGSTAT;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMINITWAITBITS
{
    uint32_t val;
    struct {
      uint32_t INITWAITVAL:16;
      uint32_t :16;
    } bits;
} CAMINITWAITBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMINITWAITBITS CAMINITWAIT;

extern volatile uint32_t CAMDISOSC0;

extern volatile uint32_t CAMDISOSC1;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMSWOFFTMRBITS
{
    uint32_t val;
    struct {
      uint32_t SWOFFTMR:16;
      uint32_t :16;
    } bits;
} CAMSWOFFTMRBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMSWOFFTMRBITS CAMSWOFFTMR;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMCLKDIVBITS
{
    uint32_t val;
    struct {
      uint32_t CLKDIV:8;
      uint32_t :24;
    } bits;
} CAMCLKDIVBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMCLKDIVBITS CAMCLKDIV;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMHWCFGBITS
{
    uint32_t val;
    struct {
      uint32_t NUMRINGS:8;
      uint32_t AIS31:1;
      uint32_t AIS31FULL:1;
      uint32_t :22;
    } bits;
} CAMHWCFGBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMHWCFGBITS CAMHWCFG;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPKPTRBITS
{
      uint32_t val;
      struct {
      uint32_t OPPTRA:4;
      uint32_t :4;
      uint32_t OPPTRB:4;
      uint32_t :4;
      uint32_t OPPTRC:4;
      uint32_t :4;
      uint32_t OPPTRN:4;
      uint32_t :4;
    } bits;
} CAMPKPTRBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPKPTRBITS CAMPKPTR;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPKCMDBITS
{
    uint32_t val;
    struct {
      uint32_t OPTYPE:7;
      uint32_t FIELD:1;
      uint32_t OPSIZE:10;
      uint32_t :1;
      uint32_t RANDMOD:1;
      uint32_t SELCRV:4;
      uint32_t RANDKE:1;
      uint32_t RANDPROJ:1;
      uint32_t EDWARDS:1;
      uint32_t :1;
      uint32_t SWBYTES:1;
      uint32_t FLAGA:1;
      uint32_t FLAGB:1;
      uint32_t CALCR2:1;
    } bits;
} CAMPKCMDBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPKCMDBITS CAMPKCMD;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPKCONBITS
{
    uint32_t val;
    struct {
      uint32_t PKSTRT:1;
      uint32_t CLRIRQ:1;
      uint32_t :30;
    } bits;
} CAMPKCONBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPKCONBITS CAMPKCON;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPKSTATBITS
{
    uint32_t val;
    struct {
      uint32_t :4;
      uint32_t PTNOCRV:1;
      uint32_t INVLDMIC:1;
      uint32_t OORANG:1;
      uint32_t INVLDMOD:1;
      uint32_t NIMPL:1;
      uint32_t SIGNVLD:1;
      uint32_t PRMNVLD:1;
      uint32_t NINVERT:1;
      uint32_t COMP:1;
      uint32_t NQUADRES:1;
      uint32_t BADORDR:1;
      uint32_t :1;
      uint32_t PKBUSY:1;
      uint32_t INTSTAT:1;
      uint32_t :6;
      uint32_t FAILPTR:5;
      uint32_t :3;
    } bits;
} CAMPKSTATBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPKSTATBITS CAMPKSTAT;

extern volatile uint32_t CAMPKTMR;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPKCONHWBITS
{
    uint32_t val;
    struct {
      uint32_t MAXOPSIZE:12;
      uint32_t NBMULT:4;
      uint32_t PRMFIELD:1;
      uint32_t BINFIELD:1;
      uint32_t ECC:1;
      uint32_t :1;
      uint32_t P256:1;
      uint32_t P384:1;
      uint32_t P521:1;
      uint32_t P192:1;
      uint32_t CRV25519:1;
      uint32_t AHBMAS:1;
      uint32_t :3;
      uint32_t DISSMX:1;
      uint32_t DISCLRMEM:1;
      uint32_t DISCM:1;
    } bits;
} CAMPKCONHWBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPKCONHWBITS CAMPKCONHW;

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMPKOPSZBITS
{
    uint32_t val;
    struct {
      uint32_t OPSIZE:13;
      uint32_t :19;
    } bits;
} CAMPKOPSZBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMPKOPSZBITS CAMPKOPSZ;

extern volatile uint32_t CAMPKMEMOFF;

extern volatile uint32_t CAMPKMICOFF;

#ifndef CAMCON

/* cppcheck-suppress misra-c2012-19.2 */
typedef union tagCAMCONBITS
{
    uint32_t val;
    struct {
      uint32_t SWRST:1;
      uint32_t ON:1;
      uint32_t PRIV:1;
      uint32_t :3;
      uint32_t RUNSTDBY:1;      
      uint32_t :9;
      uint32_t :16;
    } bits;
} CAMCONBITS;

/* cppcheck-suppress misra-c2012-19.2 */
extern volatile CAMCONBITS CAMCON;
#endif // CAMCON

#ifdef __cplusplus  // Provide C++ Compatibility
    }
#endif

#endif // SEC_HSM_LITE_04777_V1_4F000000_LOCAL_H

/**
 End of File
*/


