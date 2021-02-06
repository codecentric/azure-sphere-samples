import { AzureFunction, Context } from "@azure/functions"
import { Client, Registry } from "azure-iothub";
import { DeviceIdentity } from "azure-iothub/dist/device";
import { Device } from "azure-iothub/dist/pl/models";
import { Devices } from "azure-iothub/dist/pl/operations";

const eventGridTrigger: AzureFunction = async function (context: Context, eventGridEvent: any): Promise<void> {
    const registry = Registry.fromConnectionString(process.env["IOT_HUB_CONNECTION_STRING"]);

    const devices = await registry.list().then(response => response.responseBody);

    const connectedDevices = devices.reduce<number>((connected: number, current: DeviceIdentity) => connected + (current.connectionState == ('Connected' as any) ? 1 : 0), 0);
    context.log(`Connected Devices: ${connectedDevices}`)

    const twins = await Promise.all( devices.map(device => registry.getTwin(device.deviceId)))

    twins.forEach(twin => {
        context.log(`Updating Desired State of Device ${twin.responseBody.deviceId}`)
        twin.responseBody.update(desiredState(connectedDevices))
    });
};

function desiredState(connectedDevices: number): any {
    return {
        properties: {
            desired: {
                connectedDevices: connectedDevices
            }
        }
    }
}
    
export default eventGridTrigger;
