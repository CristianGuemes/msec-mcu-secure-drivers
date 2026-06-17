# coding: utf-8
"""
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
"""

################################################################################
#### Component ####
################################################################################
def instantiateComponent(pufComponent):

    print("instantiate Component: Microchip Secure drivers")

    res = Database.activateComponents(["HarmonyCore"])

    # Enable "Generate Harmony Driver Common Files" option in MHC
    Database.sendMessage("HarmonyCore", "ENABLE_DRV_COMMON", {"isEnabled":True})

    # Enable "Generate Harmony System Service Common Files" option in MHC
    Database.sendMessage("HarmonyCore", "ENABLE_SYS_COMMON", {"isEnabled":True})

    # Enable "Generate Harmony System Port Files" option in MHC
    Database.sendMessage("HarmonyCore", "ENABLE_SYS_PORTS", {"isEnabled":True})

    ############################################################################
    #### Code Generation ####
    ############################################################################

    configName = Variables.get("__CONFIGURATION_NAME")

    #### Common Files ##########################################################

    pufHeaderFile = pufComponent.createFileSymbol("PUF_HEADER", None)
    pufHeaderFile.setSourcePath("puf/include/drv_puf.h")
    pufHeaderFile.setOutputName("drv_puf.h")
    pufHeaderFile.setDestPath("secure_services/driver/puf")
    pufHeaderFile.setProjectPath("config/" + configName + "/secure_services/driver/puf")
    pufHeaderFile.setType("HEADER")
    pufHeaderFile.setSecurity("SECURE")

    pufHwHeaderFile = pufComponent.createFileSymbol("PUF_HW_HEADER", None)
    pufHwHeaderFile.setSourcePath("puf/include/drv_puf_hw_iface.h")
    pufHwHeaderFile.setOutputName("drv_puf_hw_iface.h")
    pufHwHeaderFile.setDestPath("secure_services/driver/puf")
    pufHwHeaderFile.setProjectPath("config/" + configName + "/secure_services/driver/puf")
    pufHwHeaderFile.setType("HEADER")
    pufHwHeaderFile.setSecurity("SECURE")

    pufSourceFile = pufComponent.createFileSymbol("PUF_SOURCE", None)
    pufSourceFile.setSourcePath("puf/source/drv_puf.c")
    pufSourceFile.setOutputName("drv_puf.c")
    pufSourceFile.setDestPath("secure_services/driver/puf")
    pufSourceFile.setProjectPath("config/" + configName + "/secure_services/driver/puf")
    pufSourceFile.setType("SOURCE")
    pufSourceFile.setSecurity("SECURE")

    pufHwSourceFile = pufComponent.createFileSymbol("PUF_HW_SOURCE", None)
    pufHwSourceFile.setSourcePath("puf/source/drv_puf_hw_iface.c")
    pufHwSourceFile.setOutputName("drv_puf_hw_iface.c")
    pufHwSourceFile.setDestPath("secure_services/driver/puf")
    pufHwSourceFile.setProjectPath("config/" + configName + "/secure_services/driver/puf")
    pufHwSourceFile.setType("SOURCE")
    pufHwSourceFile.setSecurity("SECURE")

    #### FreeMaker Files ######################################################

    pufSystemDefFile = pufComponent.createFileSymbol("DRV_PUF_DEF", None)
    pufSystemDefFile.setType("STRING")
    pufSystemDefFile.setOutputName("core.LIST_SYSTEM_DEFINITIONS_SECURE_H_INCLUDES")
    pufSystemDefFile.setSourcePath("puf/templates/system/definitions.h.ftl")
    pufSystemDefFile.setMarkup(True)
