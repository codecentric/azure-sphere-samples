# Azure Sphere Examples

Azure Sphere is a secure IoT solution from Microsoft consisting of a whole ecosystem
of components. These include custom hardware with extended security features, a secure 
operating system and a secure cloud service for device management and firmware updates.

In this repository you can find examples that demonstrate how to write software for the
Azure Sphere hardware using the [Seeed Studio MT3620 dev kit](https://wiki.seeedstudio.com/Azure_Sphere_MT3620_Development_Kit/) and the [Seeed Studio
Grove Starter Kit for Azure Sphere MT3620](https://wiki.seeedstudio.com/Grove_Starter_Kit_for_Azure_Sphere_MT3620_Development_Kit/).

To get started you can find a general description of Azure Sphere 
[in the Azure Sphere documentation](https://docs.microsoft.com/de-de/azure-sphere/product-overview/what-is-azure-sphere). For 
instructions on how to setup a development environment and how to claim an Azure Sphere
device, refer to the [quick start tutorial on Azure Docs](https://docs.microsoft.com/de-de/azure-sphere/install/overview).

**Important**: Be aware that [claiming a device](https://docs.microsoft.com/de-de/azure-sphere/install/claim-device?tabs=cliv1)
is a one-time operation. Once a device has been
claimed by an Azure Sphere tenant, it cannot be removed or reclaimed by a different tenant.
This is a security feature that prohibits externals who get physical access to an Azure Sphere
device to take over its ownership.

## Example "Temperature Monitoring"

This example shows a simple temperature monitoring application which is also able to control
an external device (like a fan) by switching on and off a relay depending on a configurable 
temperature threshold. A display shows the current temperature readings and the temperature
threshold which can be configured via a connected potentiometer during runtime.
Once the temperature surpasses the threshold, the relay is switched on. It is switched off
once the temperature falls below the threshold.

The source code and an extended description of the example can be found in the folder 
[Samples/TemperatureMonitor](Samples/TemperatureMonitor/).

## Multi-Threading

Uses POSIX threads instead of event polling.

The source code and an extended description of the example can be found in the folder 
[Samples/MultiThreading](Samples/MultiThreading/).

## Azure IoT Hub

The example shows how to connect to the Azure IoT Hub and control the LEDs of the MT3620 Dev Kit via the IoT Hub Device Twin.
It also shows the usage of a direct message to restart the device and how to react to general messages.

The source code and an extended description of the example can be found in the folder 
[Samples/IotHub](Samples/IotHub/).
