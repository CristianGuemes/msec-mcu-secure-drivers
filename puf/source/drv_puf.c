/*******************************************************************************
* Copyright (C) 2025 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

/**
 * PUF Driver Source File
 *
 * @file drv_puf.c
 *
 * @ingroup puf_internal
 *
 * @brief This file contains the API implementations for the PUF (Physical Unclonable Function) driver.
 *
 * @version PUF Driver Version 1.0.0
 *
 */

#include <stddef.h>
#include "device.h"
#include "drv_puf.h"
#include "drv_puf_hw_iface.h"

/**
 * @ingroup puf_internal
 * @typedef drv_puf_state_t
 * @brief Enumeration for PUF Driver states.
 */
typedef enum
{
    DRV_PUF_STATE_UNINITIALIZED = 0x00U, /**< Driver is not initialized */
    DRV_PUF_STATE_INITIALIZED   = 0x01U, /**< Driver is initialized */
    DRV_PUF_STATE_ENROLLED      = 0x02U, /**< PUF is enrolled */
    DRV_PUF_STATE_STARTED       = 0x03U, /**< PUF operation has started */
    DRV_PUF_STATE_STOPPED       = 0x04U, /**< PUF operation has stopped */
    DRV_PUF_STATE_BISTING       = 0x05U, /**< PUF is performing Built-In Self-Test (BIST) */
    DRV_PUF_STATE_ZEROIZED      = 0x06U, /**< PUF has been zeroized (data erased) */
    DRV_PUF_STATE_LOCKED        = 0x07U, /**< PUF is locked and cannot be accessed */
} drv_puf_state_t;

static drv_puf_state_t pufState = DRV_PUF_STATE_UNINITIALIZED;

/**
 * @ingroup puf_internal
 * @brief Map PUF result from hardware return value.
 *
 * This function checks the return code obtained from the PUF hardware
 * driver and returns the PUF result accordingly.
 *
 * @retval drv_puf_result_t The result of the PUF driver corresponding to hardware return code.
 *
 * @note This function is intended for internal use only.
 */
static drv_puf_result_t lDRV_PUF_MapResult(drv_puf_hw_return_t hwCode)
{
    drv_puf_result_t pufResult;

    switch (hwCode)
    {
        case PUF_HW_OK:
            pufResult = DRV_PUF_RES_OK;
            break;
        case PUF_HW_ERR_PARAMETER:
            pufResult = DRV_PUF_RES_PARAM_ERR;
            break;
        case PUF_HW_ERR_CMD_NOT_ALLOWED:
            pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;
            break;
        case PUF_HW_ERR_CMD_REJECTED:
            pufResult = DRV_PUF_RES_CMD_REJECTED_ERR;
            break;
        case PUF_HW_ERR_BIST:
            pufResult = DRV_PUF_RES_BIST_ERR;
            break;
        case PUF_HW_ERR_TRANSFER_ABORTED:
            pufResult = DRV_PUF_RES_TRANSFER_ABORTED_ERR;
            break;
        case PUF_HW_ERR_PRODUCT:
            pufResult = DRV_PUF_RES_WRONG_AC_ERR;
            break;
        case PUF_HW_ERR_PRODUCT_PH2:
            pufResult = DRV_PUF_RES_WRONG_AC_PH2_ERR;
            break;
        case PUF_HW_ERR_TRANSFER:
            pufResult = DRV_PUF_RES_AC_CORRUPTED_ERR;
            break;
        case PUF_HW_ERR_TRANSFER_PH2:
            pufResult = DRV_PUF_RES_AC_CORRUPTED_PH2_ERR;
            break;
        case PUF_HW_ERR_AUTH:
            pufResult = DRV_PUF_RES_AC_AUTH_FAILED_ERR;
            break;
        case PUF_HW_ERR_AUTH_PH2:
            pufResult = DRV_PUF_RES_AC_AUTH_FAILED_PH2_ERR;
            break;
        case PUF_HW_ERR_PUF_QUALITY:
            pufResult = DRV_PUF_RES_QUALITY_ERR;
            break;
        case PUF_HW_ERR_CONTEXT:
            pufResult = DRV_PUF_RES_CONTEXT_ERR;
            break;
        case PUF_HW_ERR_DESTINATION:
            pufResult = DRV_PUF_RES_DESTINATION_ERR;
            break;
        case PUF_HW_FAILURE_SRAM:
            pufResult = DRV_PUF_RES_FAILURE_SRAM_ERR;
            break;
        case PUF_HW_ERR_GENERIC:
        default:
            pufResult = DRV_PUF_RES_GENERIC_ERR;
            break;
    }

    return pufResult;
}

/**
 * @ingroup puf_internal
 * @brief Waits for the PUF module to become ready.
 *
 * This function blocks execution until the PUF (Physically Unclonable Function) module
 * completes its initialization or current operation. It continuously checks the BUSY
 * status bit in the PUF status register and returns only when the module is ready.
 *
 * @note This function is intended for internal use only.
 */
static inline void lDRV_PUF_WaitForReady(void)
{
    volatile uint32_t status = PUF_REGS->PUF_STATUSA & PUF_STATUSA_BUSY_Msk;

    while (status > 0U)
    {
        status = PUF_REGS->PUF_STATUSA & PUF_STATUSA_BUSY_Msk;
    }
}

/**
 * @ingroup puf_internal
 * @brief Checks if the given length in bits has a valid value.
 *
 * This function returns the result of checking if the given length in bits
 * has a valid value. Accepted values are multiples of 64 (until 1024) and
 * 2048, 3072, 4096.
 *
 * @retval true Length in bits is valid.
 * @retval false Length in bits is not valid.
 *
 * @note This function is intended for internal use only.
 */
static bool lDRV_PUF_isValidLenBits(uint32_t lenBits)
{
    bool isValid = false;

    /* Check for multiples of 64 between 64 and 1024 */
    if ((lenBits >= 64U) && (lenBits <= 1024U))
    {
        uint32_t lenBitsRemainder = lenBits % 64U;
        if (lenBitsRemainder == 0U)
        {
            isValid = true;
        }
    }
    /* Check for specific accepted values */
    else if ((lenBits == 2048U) || (lenBits == 3072U) || (lenBits == 4096U))
    {
        isValid = true;
    }
    else
    {
        isValid = false;
    }

    return isValid;
}

drv_puf_result_t DRV_PUF_Initialize(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_OK;

    return pufResult;
}

drv_puf_result_t DRV_PUF_Start(const drv_puf_start_t *const pufStartConfig)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_OK;

    pufState = DRV_PUF_STATE_UNINITIALIZED;

    if (pufStartConfig == NULL)
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }

    if ((pufResult == DRV_PUF_RES_OK) && (pufStartConfig->initSramPwrSwEnable))
    {
        // SRAM is powered before module is enabled
        pufResult = DRV_PUF_SramPwrSwitchEnable(pufStartConfig->initSramPwrUpTime);
    }

#if DRV_PUF_CTRLAPRIV_IMPLEMENTED
    if ((pufResult == DRV_PUF_RES_OK) && (pufStartConfig->initPrivAccess))
    {
        // Setup privileged access mode
        pufResult = DRV_PUF_PrivilegedAccessEnable();
    }
#endif

    if ((pufResult == DRV_PUF_RES_OK) && (pufStartConfig->initCmdDisableMask != 0U))
    {
        // Disable commands
        pufResult = DRV_PUF_DisableCommand(pufStartConfig->initCmdDisableMask);
    }

    // Enable PUF
    if (pufResult == DRV_PUF_RES_OK)
    {
        pufResult = DRV_PUF_Enable();
    }

    if (pufResult == DRV_PUF_RES_OK)
    {
        lDRV_PUF_WaitForReady();
        
        drv_puf_hw_return_t hwCode;
        hwCode = drv_puf_hw_initialize();
        pufResult = lDRV_PUF_MapResult(hwCode);
    }

    if (pufResult == DRV_PUF_RES_OK)
    {
        pufState = DRV_PUF_STATE_INITIALIZED;
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_Enable(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_OK;

    PUF_REGS->PUF_CTRLA |= PUF_CTRLA_ENABLE_Msk;

    // Data Sync Barrier
    __DSB();

    return pufResult;
}

drv_puf_result_t DRV_PUF_Disable(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_REGISTER_ERR;

    // Check if QK module is busy
    if ((PUF_REGS->PUF_QK_SR & PUF_QK_SR_qk_busy_Msk) == 0U)
    {
        PUF_REGS->PUF_CTRLA &= ~PUF_CTRLA_ENABLE_Msk;

        // Data Sync Barrier
        __DSB();

        pufState = DRV_PUF_STATE_UNINITIALIZED;
        pufResult = DRV_PUF_RES_OK;
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_SoftReset(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_OK;

    PUF_REGS->PUF_CTRLA |= PUF_CTRLA_SWRST_Msk;

    pufState = DRV_PUF_STATE_UNINITIALIZED;

    return pufResult;
}

drv_puf_result_t DRV_PUF_SramPwrSwitchEnable(uint32_t onTime)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_OK;

    uint32_t waitTime = onTime;

    PUF_REGS->PUF_CTRL_PWR |= PUF_CTRL_PWR_SRAM_PWRSW_Msk;

    // Data Sync Barrier
    __DSB();

    // Wait for power up
    while (waitTime > 0U)
    {
        __NOP();
        waitTime--;
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_SramPwrSwitchDisable(uint32_t offTime)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_OK;

    uint32_t waitTime = offTime;

    PUF_REGS->PUF_CTRL_PWR &= ~PUF_CTRL_PWR_SRAM_PWRSW_Msk;

    // Data Sync Barrier
    __DSB();

    // Wait for power down
    while (waitTime > 0U)
    {
        Nop();
        waitTime--;
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_PrivilegedAccessEnable(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_REGISTER_ERR;

#if DRV_PUF_CTRLAPRIV_IMPLEMENTED
    PUF_REGS->PUF_CTRLA |= PUF_CTRLA_PRIV_Msk;

    pufResult = DRV_PUF_RES_OK;
#endif

    return pufResult;

}

drv_puf_result_t DRV_PUF_PrivilegedAccessDisable(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_REGISTER_ERR;

#if DRV_PUF_CTRLAPRIV_IMPLEMENTED
    PUF_REGS->PUF_CTRLA &= ~PUF_CTRLA_PRIV_Msk;

    pufResult = DRV_PUF_RES_OK;
#endif

    return pufResult;
}

drv_puf_result_t DRV_PUF_RestrictContext(drv_puf_user_ctx_t ctxNum, uint32_t ctxVal)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_REGISTER_ERR;

    if (ctxNum == DRV_PUF_USER_CONTEXT_0)
    {
        PUF_REGS->PUF_RESTRICT0 = (uint32_t)ctxVal;
        pufResult = DRV_PUF_RES_OK;
    }
    else if (ctxNum == DRV_PUF_USER_CONTEXT_1)
    {
        PUF_REGS->PUF_RESTRICT1 = (uint32_t)ctxVal;
        pufResult = DRV_PUF_RES_OK;
    }
    else
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_DisableCommand(uint32_t cmdMask)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_OK;

    PUF_REGS->PUF_DISABLE = cmdMask & PUF_DISABLE_Msk;

    return pufResult;
}

drv_puf_result_t DRV_PUF_CheckDisableCommand(uint32_t *cmdMask)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_REGISTER_ERR;

    if (cmdMask == NULL)
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        *cmdMask = PUF_REGS->PUF_DISABLE;

        pufResult = DRV_PUF_RES_OK;
    }

    return pufResult;
}

bool DRV_PUF_IsBusy(void)
{
    return (((PUF_REGS->PUF_STATUSA & PUF_STATUSA_BUSY_Msk) == PUF_STATUSA_BUSY_1_Val) ? true : false);
}

drv_puf_result_t DRV_PUF_ClearKey(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_OK;

    PUF_REGS->PUF_CTRL_KEY |= PUF_CTRL_KEY_CLR_PUF_KEY_Msk;

    return pufResult;
}

drv_puf_result_t DRV_PUF_EnrollCmd(uint8_t * const activationCode, uint8_t *pufScore)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((activationCode == NULL) || (pufScore == NULL))
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if (pufState == DRV_PUF_STATE_INITIALIZED)
        {
            uint8_t hwCode;
            hwCode = drv_puf_hw_enroll(activationCode, pufScore);
            
            pufResult = lDRV_PUF_MapResult(hwCode);
            if (pufResult == DRV_PUF_RES_OK)
            {
                pufState = DRV_PUF_STATE_ENROLLED;
            }
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_StartCmd(const uint8_t * const activationCode, uint8_t *pufScore)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((activationCode == NULL) || (pufScore == NULL))
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if ((pufState == DRV_PUF_STATE_INITIALIZED) ||
            (pufState == DRV_PUF_STATE_STOPPED))
        {
            uint8_t hwCode;
            hwCode = drv_puf_hw_start(activationCode, pufScore);
                    
            pufResult = lDRV_PUF_MapResult(hwCode);
            if (pufResult == DRV_PUF_RES_OK)
            {
                pufState = DRV_PUF_STATE_STARTED;
            }
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_ReconstructCmd(const uint8_t * const activationCode, uint8_t *pufScore)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((activationCode == NULL) || (pufScore == NULL))
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if ((pufState == DRV_PUF_STATE_INITIALIZED) ||
            (pufState == DRV_PUF_STATE_STOPPED))
        {
            uint8_t hwCode;
            hwCode = drv_puf_hw_reconstruct(activationCode, pufScore);
            
            pufResult = lDRV_PUF_MapResult(hwCode);
            if (pufResult == DRV_PUF_RES_OK)
            {
                pufState = DRV_PUF_STATE_STARTED;
            }
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_StopCmd(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((pufState == DRV_PUF_STATE_INITIALIZED) ||
        (pufState == DRV_PUF_STATE_ENROLLED)    ||
        (pufState == DRV_PUF_STATE_STARTED))
    {
        uint8_t hwCode;
        hwCode = drv_puf_hw_stop();
        
        pufResult = lDRV_PUF_MapResult(hwCode);
        if (pufResult == DRV_PUF_RES_OK)
        {
            pufState = DRV_PUF_STATE_STOPPED;
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_CreateUserContext0(drv_puf_key_index_t keyIndex,
        uint32_t userContext0, uint32_t *combinedUserContext0)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_PARAM_ERR;

    if (combinedUserContext0 != NULL)
    {
        *combinedUserContext0 = (userContext0 & ~0x0FU) | ((uint32_t)keyIndex & 0x0FU);
        pufResult = DRV_PUF_RES_OK;
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_GetKeyCmd(uint16_t keyLenBits,
        drv_puf_key_scope_t keyScopeStarted, drv_puf_key_scope_t keyScopeEnrolled,
        uint32_t userContext0, uint32_t userContext1,
        drv_puf_data_destination_t keyDest, uint8_t *key)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((lDRV_PUF_isValidLenBits(keyLenBits) == false) || (key == NULL))
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if ((pufState == DRV_PUF_STATE_ENROLLED) ||
            (pufState == DRV_PUF_STATE_STARTED))
        {
            drv_puf_hw_key_context_cfg_t keyCfg = {
                .context_type       = PUF_HW_KEY_CONTEXT_TYPE,
                .key_length_in_bits = keyLenBits,
                .key_type           = PUF_HW_KEY_TYPE_GENERIC,
                .key_scope_started  = (uint8_t) keyScopeStarted,
                .key_scope_enrolled = (uint8_t) keyScopeEnrolled,
                .user_context_0     = userContext0,
                .user_context_1     = userContext1
            };
            uint8_t hwCode;
            hwCode = drv_puf_hw_get_key(&keyCfg, (uint8_t)keyDest, key);
            
            pufResult = lDRV_PUF_MapResult(hwCode);
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_WrapGeneratedRandomCmd(uint16_t keyLenBits,
        drv_puf_key_scope_t keyScopeStarted, drv_puf_key_scope_t keyScopeEnrolled,
        uint32_t userContext0, uint32_t userContext1, uint8_t *keyCode)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((lDRV_PUF_isValidLenBits(keyLenBits) == false) || (keyCode == NULL))
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if ((pufState == DRV_PUF_STATE_ENROLLED) ||
            (pufState == DRV_PUF_STATE_STARTED))
        {
            drv_puf_hw_key_context_cfg_t keyCfg = {
                .context_type       = PUF_HW_KEY_CONTEXT_TYPE,
                .key_length_in_bits = keyLenBits,
                .key_type           = PUF_HW_KEY_TYPE_GENERIC,
                .key_scope_started  = (uint8_t) keyScopeStarted,
                .key_scope_enrolled = (uint8_t) keyScopeEnrolled,
                .user_context_0     = userContext0,
                .user_context_1     = userContext1
            };
            uint8_t hwCode;
            hwCode = drv_puf_hw_wrap_generated_random(&keyCfg, keyCode);
            
            pufResult = lDRV_PUF_MapResult(hwCode);
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_WrapCmd(uint16_t keyLenBits,
        drv_puf_key_scope_t keyScopeStarted, drv_puf_key_scope_t keyScopeEnrolled,
        uint32_t userContext0, uint32_t userContext1, const uint8_t *key, uint8_t *keyCode)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((lDRV_PUF_isValidLenBits(keyLenBits) == false) ||
        (key == NULL) || (keyCode == NULL))
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if ((pufState == DRV_PUF_STATE_ENROLLED) ||
            (pufState == DRV_PUF_STATE_STARTED))
        {
            drv_puf_hw_key_context_cfg_t keyCfg = {
                .context_type       = PUF_HW_KEY_CONTEXT_TYPE,
                .key_length_in_bits = keyLenBits,
                .key_type           = PUF_HW_KEY_TYPE_GENERIC,
                .key_scope_started  = (uint8_t) keyScopeStarted,
                .key_scope_enrolled = (uint8_t) keyScopeEnrolled,
                .user_context_0     = userContext0,
                .user_context_1     = userContext1
            };
            uint8_t hwCode;
            hwCode = drv_puf_hw_wrap(&keyCfg, key, keyCode);
            
            pufResult = lDRV_PUF_MapResult(hwCode);
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_UnwrapCmd(const uint8_t *keyCode,
        drv_puf_data_destination_t keyDest, uint8_t *key)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((key == NULL) || (keyCode == NULL))
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if ((pufState == DRV_PUF_STATE_ENROLLED) ||
            (pufState == DRV_PUF_STATE_STARTED))
        {
            uint8_t hwCode;
            hwCode = drv_puf_hw_unwrap(keyCode, (uint8_t) keyDest, key);
            
            pufResult = lDRV_PUF_MapResult(hwCode);
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_GenerateRandomCmd(uint16_t rndDataLenBits,
        drv_puf_data_destination_t rndDataDest, uint8_t *rndData)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((lDRV_PUF_isValidLenBits(rndDataLenBits) == false) || (rndData == NULL))
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if ((pufState == DRV_PUF_STATE_INITIALIZED) ||
            (pufState == DRV_PUF_STATE_ENROLLED)    ||
            (pufState == DRV_PUF_STATE_STARTED)     ||
            (pufState == DRV_PUF_STATE_STOPPED))
        {
            uint8_t hwCode;
            hwCode = drv_puf_hw_generate_random(PUF_HW_RND_CONTEXT_TYPE,
                                         rndDataLenBits,
                                         (uint8_t) rndDataDest,
                                         rndData);
            
            pufResult = lDRV_PUF_MapResult(hwCode);
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_TestMemoryCmd(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((pufState == DRV_PUF_STATE_INITIALIZED) ||
        (pufState == DRV_PUF_STATE_STOPPED))
    {
        uint8_t hwCode;
        hwCode = drv_puf_hw_test_memory();
        pufResult = lDRV_PUF_MapResult(hwCode);
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_TestPufCmd(uint8_t *pufScore)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if (pufScore == NULL)
    {
        pufResult = DRV_PUF_RES_PARAM_ERR;
    }
    else
    {
        if (pufState == DRV_PUF_STATE_INITIALIZED)
        {
            uint8_t hwCode;
            hwCode = drv_puf_hw_test_puf(pufScore);
            pufResult = lDRV_PUF_MapResult(hwCode);
        }
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_Zeroize(void)
{
    drv_puf_result_t pufResult;
    uint8_t hwCode;

    hwCode = drv_puf_hw_zeroize();
    
    pufResult = lDRV_PUF_MapResult(hwCode);
    if (pufResult == DRV_PUF_RES_OK)
    {
        pufState = DRV_PUF_STATE_ZEROIZED;
    }

    return pufResult;
}

drv_puf_result_t DRV_PUF_Bist(void)
{
    drv_puf_result_t pufResult = DRV_PUF_RES_CMD_NOT_ALLOWED_ERR;

    if ((pufState == DRV_PUF_STATE_INITIALIZED) ||
        (pufState == DRV_PUF_STATE_STOPPED))
    {
        uint8_t hwCode;
        hwCode = drv_puf_hw_bist();
        pufResult = lDRV_PUF_MapResult(hwCode);
        pufState = DRV_PUF_STATE_INITIALIZED;
    }

    return pufResult;
}
