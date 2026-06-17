/**************************************************************************
  Crypto Framework Library Source

  Company:
    Microchip Technology Inc.

  File Name:
    cam_trng.c

  Summary:
    Crypto Framework Library interface file for CAM TRNG.

  Description:
    This source file contains the interface that make up the CAM TRNG hardware
    driver for Microchip microcontrollers equipped with a Crypto Accelerator Module.
**************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2026, Microchip Technology Inc., and its subsidiaries. All rights reserved.

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

/* MISRA C:2012 Rules 8.4 and 11.4 Deviation Justification
 *
 * ID      Category   Description
 * 8.4     Required   A compatible declaration shall be visible when an object or
 *                    function with external linkage is defined
 * 11.4    Advisory   A conversion should not be performed between a pointer
 *                    to object and an integer type
 *
 * Justification:
 * - Rule 8.4: All externally visible CAM/HSM driver APIs (e.g., DRV_CRYPTO_*
 *   functions) are declared in their corresponding public header files
 *   (such as cam_trng.h). Some MISRA analysis tools may not fully parse these
 *   external headers, resulting in false positives where it appears that
 *   compatible declarations are missing. The implementation is compliant, as
 *   each externally linked function has a matching prototype defined in the
 *   associated header file.
 *
 * - Rule 11.4: The TRNG driver accesses hardware registers and memory-mapped
 *   I/O locations by casting integer address constants to object pointers
 *   (e.g., (volatile uint32_t *)0x7A1080U). These conversions are intentional,
 *   controlled, and required to interface with device-specific hardware. No
 *   unsafe pointer arithmetic is performed.
 *
 * Scope:
 * This justification applies to all functions with external linkage and all
 * instances of integer ↔ pointer conversions within this source file.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "../inc/cam_trng.h"
#include "cam_trng_local.h"
#include "../inc/cam_device.h"

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

#if defined(UNIT_TESTING)
    #define REPEAT_WRAPPER ((void)0)
#elif defined(__XCDSC__)
    /* cppcheck-suppress misra-c2012-20.10 */
    #define REPEAT_WRAPPER do { __asm__ volatile("REPEAT #10 \n NOP"); } while(0)
#else
    /* cppcheck-suppress misra-c2012-20.10 */
    #define REPEAT_WRAPPER do { __asm__ volatile("NOP \n NOP \n NOP \n NOP \n NOP \n NOP \n NOP \n NOP \n NOP \n NOP \n NOP"); } while(0)
#endif

#if defined(UNIT_TESTING)
    #include "../scripts/testing/mocks/cam_hw_mock.h"
    #define TRNG_READ_ADDRESS CAMTRNG_READ_ADDR_MOCK
#elif defined(GENERIC_TARGET_CAM_05346)
    #define TRNG_READ_ADDRESS (*(volatile uint32_t *)0x7A1080U)
    #include "sec_cam_05346_v2_07A0000_local.h"
#elif defined(GENERIC_TARGET_CAM_06048)
    #define TRNG_READ_ADDRESS (*(volatile uint32_t *)0x7A1080U)
    #include "sec_cam_06048_v3_07A0000_local.h"
#elif defined(GENERIC_TARGET_HSM_LITE_04777)
    #define TRNG_READ_ADDRESS (*(volatile uint32_t *)0x4F011080UL)
    #include "sec_hsm_lite_04777_v1_4F000000_local.h"
#else
    #error "No TRNG_READ_ADDRESS mapping: define UNIT_TESTING or supported device macro."
#endif
// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

/*
 *  @brief Determines the exact error by reading the error bits.
 *  @return Returns the exact error from the multiple options from the TRNG_ERROR enum.
*/
static TRNG_ERROR lDRV_CRYPTO_TRNG_CheckError(void)
{
    TRNG_ERROR errorCode;
    if (CAMTRNGSTAT.bits.STRTUPERR != 0U)
    {
        errorCode = TRNG_EXEC_STARTUP;
    }
    else if (CAMTRNGSTAT.bits.PROPERR != 0U)
    {
        errorCode = TRNG_EXEC_PROP;
    }
    else if (CAMTRNGSTAT.bits.REPERR != 0U)
    {
        errorCode = TRNG_EXEC_REP;
    }
#if defined(GENERIC_TARGET_CAM_05346) || defined(GENERIC_TARGET_HSM_LITE_04777)
    else if (CAMTRNGSTAT.bits.FIFOACCERR != 0U)
    {
        errorCode = TRNG_EXEC_FIFO;
    }
#endif
#ifdef GENERIC_TARGET_CAM_06048
    else if (CAMTRNGSTAT.bits.CONDTOOSLOW != 0U)
    {
        errorCode = TRNG_EXEC_CONDITIONING_TOO_SLOW;
    }
    else if (CAMTRNGSTAT.bits.PROPTSTFAILPSH != 0U)
    {
        errorCode = TRNG_EXEC_PROPORTION_TEST_FAIL_PER_SHARE;
    }
    else if (CAMTRNGSTAT.bits.REPTSTFAILPSH != 0U)
    {
        errorCode = TRNG_EXEC_REPETITION_TEST_FAIL_PER_SHARE;
    }
    else if (CAMTRNGSTAT.bits.HLTTSTFAIL != 0U)
    {
        errorCode = TRNG_EXEC_HEALTH_TEST_FAIL;
    }
#endif
    else
    {
        errorCode = TRNG_NO_ERROR;
    }
    return errorCode;
}

#if defined(GENERIC_TARGET_CAM_05346) || defined(GENERIC_TARGET_HSM_LITE_04777)
/*
 *  @brief Stores the input values and sets two registers.
 *  @param writeStartup Enables (true)/Disables (false) writing values to the FIFO during start-up. Always set to false in our implementation.
 *  @param htestAfterCond Use the output after (true)/before (false) conditioning for the health test module input. Always set to false in our implementation.
 *  @param conditioningBypass The conditioning function is bypassed (true) or used (false). Always set to false in our implementation.
 *  @param nb128Block Number of 128-bit blocks used in AES-CBCMAC post-processing bits. Always set to 4 in our implementation.
 *  @return none
 *  @note The write to CAMCON enables the CAM. The first write to CAMTRNGCON zeroes the register. The second write enables the built-in tests.
*/
static void lDRV_CRYPTO_TRNG_SetupEngine(bool writeStartup, bool htestAfterCond, bool conditioningBypass, uint8_t nb128Block) 
{
    CAMTRNGCON.val &= TEST_ENABLE_MASK;
    CAMTRNGCON.bits.FIFOWRTSTRT = writeStartup;
    CAMTRNGCON.bits.HLTHTSTSEL = htestAfterCond;
    CAMTRNGCON.bits.CONDBYPASS = conditioningBypass;

    if (0x10U > nb128Block)
    {
        CAMTRNGCON.bits.NB128BLOCK = nb128Block;
    }

    CAMTRNGCON.val |= CAM_TRNG_SETUP_MASK;
}
#endif
#ifdef GENERIC_TARGET_CAM_06048
/*
 *  @brief Stores the input values and sets two registers.
 *  @param writeStartup Enables (true)/Disables (false) writing values to the FIFO during start-up. Always set to false in our implementation.
 *  @param htestAfterCond Use the output after (true)/before (false) conditioning for the health test module input. Always set to false in our implementation.
 *  @param conditioningBypass The conditioning function is bypassed (true) or used (false). Always set to false in our implementation.
 *  @param nb128Block Number of 128-bit blocks used in AES-CBCMAC post-processing bits. Always set to 4 in our implementation.
 *  @param blendMode Mode used for the compression of values.
 *  @return none
 *  @note The write to CAMCON enables the CAM. The first write to CAMTRNGCON zeroes the register. The second write enables the built-in tests.
*/
static void lDRV_CRYPTO_TRNG_SetupEngine(bool writeStartup, bool htestAfterCond, bool conditioningBypass, uint8_t nb128Block, uint8_t blendMode) 
{
    if (blendMode > (uint8_t)BLEND_VON_NEUMANN_DEBIASING) {
        CAMTRNGCON.bits.BLENDMODE = BLEND_VON_NEUMANN_DEBIASING;
    }
    else
    {
        CAMTRNGCON.bits.BLENDMODE = blendMode;
    }
    
    CAMTRNGCON.val &= TEST_ENABLE_MASK;
    CAMTRNGCON.bits.LFSREN = OFF_BIT;
    CAMTRNGCON.bits.FIFOWRTSTRT = writeStartup;
    CAMTRNGCON.bits.IGHLTTSTFAIL = htestAfterCond;
    CAMTRNGCON.bits.CONDBYPASS = conditioningBypass;
    
    if (0x10U > nb128Block)
    {
        CAMTRNGCON.bits.NB128BLOCK = nb128Block;
    }
    
    CAMTRNGCON.bits.INTENFULL = ON_BIT;
    CAMTRNGCON.bits.INTENHLTFAIL = ON_BIT;
}
#endif

/*
 *  @brief Resets the CAM module by toggling the SOFTRST bit.
 *  @return none
 *  @note Toggling the SOFTRST bit resets the continuous test, conditioning function, and FIFO.
*/
static void lDRV_CRYPTO_TRNG_Reset(void)
{
    CAMTRNGCON.bits.SOFTRST = ON_BIT;
    CAMTRNGCON.bits.SOFTRST = OFF_BIT;
    REPEAT_WRAPPER;
}

/*
 *  @brief Setter for the initial wait value.
 *  @param waitValue Number of clock cycles to wait before reading data from the noise source.
 *  @return none
*/
static void lDRV_CRYPTO_TRNG_SetInitialWait(uint16_t waitValue)
{
    CAMINITWAIT.bits.INITWAITVAL = waitValue;
}

/*
 *  @brief If the FIFO threshold is under four, set the corresponding register to the threshold.
 *  @param fifoThreshold Value for if the FIFO contains that many values, the module leaves the idle state to refill the FIFO.
 *  @return none
 *  @note The FIFO threshold is always 3, meaning that if there is 3 or less 4-bit words, then the FIFO will be refilled.
*/
static void lDRV_CRYPTO_TRNG_SetFifoThreshold(uint8_t fifoThreshold)
{
    if (WORD_SIZE >= fifoThreshold)
    {
        CAMFIFOTHRESH = fifoThreshold;
    }
}

#ifdef GENERIC_TARGET_CAM_06048
/*
 *  @brief Sets the frequency at which values are read from the FIFO.
 *  @param samplingRate Number of clock cycles between samples.
 *  @return none
*/
static void lDRV_CRYPTO_TRNG_SetSamplingRate(uint16_t samplingRate) 
{
    CAMCLKDIV.bits.CLKDIV = samplingRate;
}

/*
 *  @brief Sets the amount of time alloted to the cooldown sequence.
 *  @param coolDownPeriod Number of clock cycles in the cooldown sequence.
 *  @return none
*/
static void lDRV_CRYPTO_TRNG_SetCooldownPeriod(uint16_t coolDownPeriod)
{
    CAMTCLDWN.bits.TCLDWN = coolDownPeriod;
}
#endif

/*
 *  @brief Takes four 8-bit words and combines them ito a 32-bit value.
 *  @param data Array of 8-bit words.
 *  @returns The 32-bit value that is a combination of the input 8-bit words.
 *  @note This function is used in the lDRV_CRYPTO_TRNG_SetupKey() function to turn the 8-bit array of the generated key into a 32-bit value to store in the key register.
*/
static uint32_t lDRV_CRYPTO_TRNG_CombineData(uint8_t* data)
{
    uint32_t output = 0;
    output |= (uint32_t) data[0];
    output |= (uint32_t) data[1] << 8;
    output |= (uint32_t) data[2] << 16;
    output |= (uint32_t) data[3] << 24;
    return output;
}

/*
 *  @brief Takes the generated key and stores it in the CAMKEY0 register.
 *  @param data The generated key from the first part of lDRV_CRYPTO_TRNG_SetKey().
 *  @param size The size of the key.
 *  @return none
*/
static void lDRV_CRYPTO_TRNG_SetupKey(uint8_t* data, uint8_t size)
{   
    volatile uint32_t* ptr = &CAMKEY0;
    uint8_t* localData = data;

    uint8_t sizeCopy = size;
    if (sizeCopy > SETUPKEY_MAX_SIZE)
    {
        sizeCopy = SETUPKEY_MAX_SIZE;
    }

    for (uint32_t index = 0; index < sizeCopy; index += WORD_SIZE)
    {
        *ptr = lDRV_CRYPTO_TRNG_CombineData(localData);
        ptr++;
        localData = &localData[WORD_SIZE];
    }
}

/*
 *  @brief Splits a 32-bit word into 4 8-bit words.
 *  @param value 32-bit word to be split.
 *  @param destination Array of 8-bit words that are the split values of the 32-bit value.
 *  @param size Number of 8-bit words contained in the 32-bit value.
 *  @return none
 *  @note This function is used in DRV_CRYPTO_TRNG_ReadData() to convert the 32-bit generated values into 8-bit values to store in the buffer.
*/
static void lDRV_CRYPTO_TRNG_SplitData(uint32_t value,  uint8_t* destination)
{
    destination[0] = value & 0xFFU;
    destination[1] = (value >> 8U) & 0xFFU;
    destination[2] = (value >> 16U) & 0xFFU;
    destination[3] = (value >> 24U) & 0xFFU;
}

/*
 *  @brief Generates a key and stores it for later use.
 *  @param trngData Context struct to store relevant information.
 *  @return Returns TRNG_NO_ERROR on success. Returns one of the TRNG_FAILED_TO_GENERATE / TRNG_EXEC_FIFO / TRNG_EXEC_STARTUP
 *          TRNG_EXEC_PROP / TRNG_EXEC_REP on fail.
 *  @note This function calls the DRV_CRYPTO_TRNG_ReadData() function to generate a random key for the driver to use.
*/
static TRNG_ERROR lDRV_CRYPTO_TRNG_SetKey(TRNG_CTX* trngData)
{
    TRNG_ERROR errorCode = TRNG_NO_ERROR;
    if (CAMTRNGCON.bits.TESTEN == 0U)
    {
        errorCode = DRV_CRYPTO_TRNG_ReadData(trngData->key.data, trngData->key.size);
    }
    
    if (TRNG_NO_ERROR != errorCode)
    {
        errorCode = TRNG_KEY_SETUP;
    }
    else
    {
        lDRV_CRYPTO_TRNG_SetupKey(trngData->key.data, trngData->key.size);
    }

    return errorCode;
}

/*
 *  @brief Configures the TRNG context struct that will be used throughout generation.
 *  @param trngData TRNG context struct that holds settings like the initial wait and FIFO threshold.
 *  @return none
 *  @note This function also can configure the context struct for the conditioning test.
*/
static void lDRV_CRYPTO_TRNG_SetTrngData(TRNG_CTX* trngData)
{
    static uint8_t conditioningTestKey[16] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
    
    trngData->key.size = KEY_SIZE;
    trngData->conditioningBypass = OFF_BIT;    
    trngData->fifoThreshold = FIFO_THRESHOLD;         
    trngData->fifoWriteStartup = OFF_BIT;          
    trngData->htestAfterCond = OFF_BIT;
    trngData->nb128Block = NUMBER_128_BLOCKS;
    trngData->initialWait = INITIAL_WAIT;
#ifdef GENERIC_TARGET_CAM_06048
    trngData->coolDownPeriod = COOLDOWN_PERIOD;
    trngData->samplingPeriod = SAMPLING_PERIOD;
    trngData->blendMode = BLEND_VON_NEUMANN_DEBIASING;
#endif
    if (CAMTRNGCON.bits.TESTEN == ON_BIT)
    {
        trngData->key.data = conditioningTestKey;
        trngData->fifoWriteStartup = ON_BIT;
        trngData->htestAfterCond = ON_BIT;
        trngData->initialWait = OFF_BIT;
    }
    else
    {
        static uint8_t normalOperationKey[16] = {0};
        trngData->key.data = normalOperationKey;
    }
}

/*
 *  @brief Sets the values of the CAMTRNGCON, CAMINITWAIT, and CAMFIFOTHRESH registers based on the TRNG context struct.
 *  @param trngData TRNG context struct that holds settings like the initial wait and FIFO threshold.
 *  @return none
*/
static void lDRV_CRYPTO_TRNG_SetRegisters(const TRNG_CTX* trngData)
{
#if defined(GENERIC_TARGET_CAM_05346) || defined(GENERIC_TARGET_HSM_LITE_04777)
    lDRV_CRYPTO_TRNG_SetupEngine(trngData->fifoWriteStartup, 
            trngData->htestAfterCond, trngData->conditioningBypass, 
            trngData->nb128Block);
#endif
#ifdef GENERIC_TARGET_CAM_06048
    lDRV_CRYPTO_TRNG_SetupEngine(trngData->fifoWriteStartup, 
        trngData->htestAfterCond, trngData->conditioningBypass, 
        trngData->nb128Block, trngData->blendMode);
    lDRV_CRYPTO_TRNG_SetSamplingRate(trngData->samplingPeriod);
    lDRV_CRYPTO_TRNG_SetCooldownPeriod(trngData->coolDownPeriod);
#endif
    lDRV_CRYPTO_TRNG_SetInitialWait(trngData->initialWait);
    lDRV_CRYPTO_TRNG_SetFifoThreshold(trngData->fifoThreshold);
}

/*
 *  @brief Sends input data through the CAMTSTDAT register for testing.
 *  @param data Data array that will be passed through the CAMTSTDAT register.
 *  @return none
 *  @note This function splits each 32-bit word into two 16-bit words before sending through the register.
*/
static void lDRV_CRYPTO_TRNG_SendTestData(const uint32_t* data)
{
#if defined(GENERIC_TARGET_CAM_05346) || defined(GENERIC_TARGET_HSM_LITE_04777)
    for (uint8_t i = 0; i < (uint8_t)16; i++) 
    {
        while(CAMTRNGSTAT.bits.TSTDATBSY == ON_BIT)
        {
            //empty
        }
        CAMTSTDAT = data[i];
    }
#endif
#ifdef GENERIC_TARGET_CAM_06048
    for (uint8_t i = 0; i < (uint8_t)16; i++) 
    {
        CAMTSTDAT = (data[i] & 0xFFFFU) << 16;
        while (CAMTRNGSTAT.bits.TSTDATBSY == ON_BIT)
        {
            //empty
        }
        CAMTSTDAT = data[i] & 0xFFFF0000U;
        while (CAMTRNGSTAT.bits.TSTDATBSY == ON_BIT)
        {
            //empty
        }
    }
#endif
}

/*
 *  @brief Reads the given read address and returns the value stored there.
 *  @return Returns the value stored in the TRNG_READ_ADDRESS.
 *  @note This function is only used in the conditioning test.
*/
#if defined(UNIT_TESTING)
extern uint32_t* lDRV_CRYPTO_TRNG_RawRead(void);
#else
static uint32_t* lDRV_CRYPTO_TRNG_RawRead(void)
{
    /* cppcheck-suppress misra-c2012-11.4 */
    uint32_t* read = (uint32_t*) &TRNG_READ_ADDRESS;
    return read;
}
#endif

/*
 *  @brief Runs the TRNG conditioning test to verify the module.
 *  @return Returns TRNG_NO_ERROR on success. Returns TRNG_COND_TEST_FAIL on fail. 
 *  @note This function additionally calls the driver setup function.
*/
static TRNG_ERROR lDRV_CRYPTO_TRNG_ConditioningTest(void) 
{
    TRNG_ERROR errorCode = TRNG_NO_ERROR;
    static const uint32_t conditioningTestResult[4] = {0xA1CAF13FU, 0x09AC1F68U, 0x30CA0E12U, 0xA7E18675U};
    static const uint32_t conditioningTestData[16] = {0xE1BCC06BU, 0x9199452AU, 0x1A7434E1U, 0x25199E7FU,
                                                      0x578A2DAEU, 0x9CAC031EU, 0xAC6FB79EU, 0x518EAF45U,
                                                      0x461CC830U, 0x11E45CA3U, 0x19C1FBE5U, 0xEF520A1AU,
                                                      0x45249FF6U, 0x179B4FDFU, 0x7B412BADU, 0x10376CE6U};
    lDRV_CRYPTO_TRNG_Reset();

    uint32_t testResult[4] = {0};

    CAMTRNGCON.bits.TESTEN = ON_BIT;

    TRNG_CTX trngCondTestData;

    lDRV_CRYPTO_TRNG_SetTrngData(&trngCondTestData);

    lDRV_CRYPTO_TRNG_SetRegisters(&trngCondTestData);
    
    errorCode = lDRV_CRYPTO_TRNG_SetKey(&trngCondTestData);

    CAMTRNGCON.bits.NDRNDEN = ON_BIT;

    lDRV_CRYPTO_TRNG_SendTestData(conditioningTestData);

    for (int i = 0; i < 4 ; i++)
    {
        testResult[i] = *lDRV_CRYPTO_TRNG_RawRead();
        REPEAT_WRAPPER;
    }
    
    for (uint8_t i = 0; i < TEST_RESULT_SIZE; i++)
    {
        if ((testResult[i] ^ conditioningTestResult[i]) != 0U)
        {
            errorCode = TRNG_COND_TEST_FAIL;
        }
    }

    CAMTRNGCON.bits.NDRNDEN = OFF_BIT;
    CAMTRNGCON.bits.TESTEN = OFF_BIT;
    
    return errorCode;
}

// *****************************************************************************
// *****************************************************************************
// Section: TRNG Common Interface Implementation
// *****************************************************************************
// *****************************************************************************

void DRV_CRYPTO_TRNG_IsrHelper(void)
{
#ifdef GENERIC_TARGET_CAM_06048
    //Turn off TRNG
    CAMTRNGCON.bits.NDRNDEN = OFF_BIT;
    
    //1. Write to FIFOLVL
    CAMFIFOLVL = 0;

    //2. Clear status bits
#endif
    CAMTRNGSTAT.bits.PROPERR = OFF_BIT;
    CAMTRNGSTAT.bits.REPERR = OFF_BIT;
#ifdef GENERIC_TARGET_CAM_06048
    CAMTRNGSTAT.bits.HLTTSTFAIL = OFF_BIT;
    
    //3. Reset
    lDRV_CRYPTO_TRNG_Reset();
    
    //Turn back on TRNG
    CAMTRNGCON.bits.NDRNDEN = ON_BIT;
#endif
}

TRNG_ERROR DRV_CRYPTO_TRNG_Setup(void)
{
    TRNG_ERROR errorCode = TRNG_NO_ERROR;
    CAMCON.val |= CAM_ENABLE_MASK;
    lDRV_CRYPTO_TRNG_Reset();

    errorCode = lDRV_CRYPTO_TRNG_ConditioningTest();

    if (errorCode != TRNG_COND_TEST_FAIL)
    {
        TRNG_CTX trngData;

        lDRV_CRYPTO_TRNG_SetTrngData(&trngData);

        lDRV_CRYPTO_TRNG_SetRegisters(&trngData);

        errorCode = lDRV_CRYPTO_TRNG_SetKey(&trngData);
    }
    
    return errorCode;
}

TRNG_ERROR DRV_CRYPTO_TRNG_ReadData(uint8_t* data, uint32_t size)
{
    TRNG_ERROR errorCode = TRNG_NO_ERROR;
    uint32_t sizeCopy = size;
    uint8_t *localData = data;
 
    CAMTRNGCON.bits.NDRNDEN = ON_BIT;

    uint32_t timeout = GENERATION_TIMEOUT;
    while (0x1U == CAMTRNGSTAT.bits.STATE)
    {
        if (0U == timeout)
        {
            CAMTRNGCON.bits.NDRNDEN = OFF_BIT;
            errorCode = TRNG_FAILED_TO_GENERATE;
            break;
        }
        timeout--;
    }
    if (errorCode == TRNG_NO_ERROR)
    {
        timeout = GENERATION_TIMEOUT;
        while (!CAMTRNGSTAT.bits.FULLINT)
        {
            if (0U == timeout)
            {
                CAMTRNGCON.bits.NDRNDEN = OFF_BIT;
                errorCode = TRNG_FAILED_TO_GENERATE;
                break;
            }
            timeout--;
        }
        if (errorCode == TRNG_NO_ERROR)
        {
            while ((sizeCopy > 0U))
            {
                /* cppcheck-suppress misra-c2012-11.4 */
                lDRV_CRYPTO_TRNG_SplitData(TRNG_READ_ADDRESS, localData);

                if(sizeCopy < WORD_SIZE)
                {
                   sizeCopy = 0;
                } 
                else 
                {
                   sizeCopy -= WORD_SIZE;
                   localData = &localData[WORD_SIZE];
                }
            }

            CAMTRNGCON.bits.NDRNDEN = OFF_BIT;
        }
    }

    if (errorCode == TRNG_NO_ERROR)
    {
        errorCode = lDRV_CRYPTO_TRNG_CheckError();
    }

    lDRV_CRYPTO_TRNG_Reset();

    return errorCode;
}
