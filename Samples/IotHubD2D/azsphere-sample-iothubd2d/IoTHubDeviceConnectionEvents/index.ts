import { AzureFunction, Context } from "@azure/functions"
import { Client, Registry } from "azure-iothub";
import { DeviceIdentity } from "azure-iothub/dist/device";
import { Device } from "azure-iothub/dist/pl/models";
import { Devices } from "azure-iothub/dist/pl/operations";

const eventGridTrigger: AzureFunction = async function (context: Context, eventGridEvent: any): Promise<void> {
    const registry = Registry.fromConnectionString(process.env["IOT_HUB_CONNECTION_STRING"]);

    const devices = await registry.list().then(response => response.responseBody);

    const connectedDevicesList = devices.filter(device => device.connectionState == ('Connected' as any));
    const connectedTwinsList = await Promise.all(connectedDevicesList.map(device => registry.getTwin(device.deviceId)));

    const connectedDevices = connectedDevicesList.length;
    const connectedNodeDevices = connectedTwinsList.reduce<number>((connected, current) => connected + (current.responseBody.properties.reported?.deviceCapabilities?.hasDisplay ? 0 : 1), 0);
    const connectedMainDevices = connectedTwinsList.reduce<number>((connected, current) => connected + (current.responseBody.properties.reported?.deviceCapabilities?.hasDisplay ? 1 : 0), 0);

    const twins = await Promise.all(devices.map(device => registry.getTwin(device.deviceId)))
    
    twins.forEach(twin => {
        context.log(`Updating Desired State of Device ${twin.responseBody.deviceId}`)
        twin.responseBody.update(desiredState(connectedDevices, connectedNodeDevices, connectedMainDevices))
    });
};

function desiredState(connectedDevices: number, connectedNodeDevices: number, connectedMainDevices: number): any {
    return {
        properties: {
            desired: {
                connectedDevices,
                connectedNodeDevices,
                connectedMainDevices
            }
        }
    }
}

export default eventGridTrigger;
