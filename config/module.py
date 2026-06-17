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

######################  Secure Drivers  ######################
def loadModule():

    processor = Variables.get("__PROCESSOR")
    print("Load Module: Microchip Secure Services")

    peripheralList = ATDF.getNode("/avr-tools-device-file/devices/device/peripherals").getChildren()
    for peripheral in peripheralList:
        peripheralName = peripheral.getAttribute("name")
        if peripheralName in ['PUF']:
            pioPeripheral = "{}_{}".format(peripheralName, peripheral.getAttribute("id"))
            print("Load Module: Microchip Secure Driver - {} ".format(pioPeripheral))

            ## PUF Driver
            pufComponent = Module.CreateComponent("drvPuf", "SECURE_PUF", "/Secure Services/Drivers/", "puf/config/drv_puf.py")
            pufComponent.addCapability("puf", "DRV_PUF", True)
            pufComponent.setDisplayType("Secure PUF Driver")
            pufComponent.setHelpKeyword("drv_puf")

