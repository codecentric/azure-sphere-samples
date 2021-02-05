# Azure IoT Hub Example

Uses Azure IoTHub device provisioning, device twin and direct messages

## Hardware

- Seeed Azure Sphere MT3620 Development Kit

## Setup an IoT Hub and Azure Device Provisioning Service

First of all you will have to setup an Azure IoT Hub.

Find information on how to setup an Azure IoT Hub and configure it to [use device provisioning service to authenticate here.](https://docs.microsoft.com/azure-sphere/app-development/setup-iot-hub#authenticate-using-the-device-provisioning-service)

## Configure the application to work with your Azure IoT Hub

The [app_manifest.json](./app_manifest.json) has to be changed to use your Azure IoTHub and DPS.

The following values are needed to be able to connect to the IoT Hub:

- The Tenant ID for your Azure Sphere device.
- The Scope ID for your device provisioning service instance.
- The Azure IoT Hub URL for your IoT Hub and the global access link to device provisioning service (global.azure-devices.provisioning.net).

The following show how to get the needed information and configure the application to use it:

1. In the [Azure Portal](https://portal.azure.com) navigate to your device provisioning service.

1. Update the **CmdArgs** field of the [app_manifest.json](./app_manifest.json) file.

   - In the Azure portal, on the overview of your device provisioning service, copy the ID Scope value and add it as first element  of the **CmdArgs** field of the app_manifest.json file as shown below:
   
        `"CmdArgs": [ "<scope_id>" ]`
    
1. Update the **AllowedConnections** field of the [app_manifest.json](./app_manifest.json) file.

   - On the Azure portal, under **Settings** of your device provisioning service, select **Linked IoT Hubs**. Copy the Name values(s) for the Azure IoT Hub(s) and append them to the **AllowedConnections** field of the app_manifest.json file.

   - Make sure that global.azure-devices-provisioning.net remains in the list because this name is required for access to the device provisioning service.

   - The **AllowedConnections** field should now look like:

     `"AllowedConnections": [ "global.azure-devices-provisioning.net", "<linked_iot_hub_url>" ]`

1. Update the **DeviceAuthentication** field of the [app_manifest.json](./app_manifest.json) file.

   - Open a terminal and use the following command to get the Tenant ID. Use the GUID, not the friendly name, and paste it into the **DeviceAuthentication** field of the app_manifest.json file:

      `azsphere tenant show-selected`

   - Your **DeviceAuthentication** field should now look like:

     `"DeviceAuthentication": "<tenat_id>"`