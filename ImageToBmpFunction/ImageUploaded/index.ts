import { AzureFunction, Context, HttpRequest } from "@azure/functions";
import * as ImageJs from 'image-js';
import * as Multipart from "parse-multipart-data";
import * as AzureIotHub from "azure-iothub";

const httpTrigger: AzureFunction = async function (context: Context, req: HttpRequest): Promise<void> {
    context.log('HTTP trigger function processed a request.');

    const deviceId = req.query.deviceId;
    const iotHubConnectionString = process.env["IOT_HUB_CONNECTION_STRING"];

    if (!iotHubConnectionString) {
        return endWithResponse(context, 500, "Internal Server Error");
    }

    if (!req.body) {
        return endWithResponse(context, 400, `Request Body is not defined`);
    }

    if (!deviceId) {
        return endWithResponse(context, 400, `Device ID has to be given`);
    }

    const bodyBuffer = Buffer.from(req.body);
    const boundary = Multipart.getBoundary(req.headers['content-type']);
    const parts = Multipart.parse(bodyBuffer, boundary);

    const image = await ImageJs.Image.load(parts[0].data);
    const greyScaled = image.resize({ width: 48, height: 48 }).grey().invert();

    const iotHubRegistry = AzureIotHub.Registry.fromConnectionString(iotHubConnectionString);

    try {
        const twinResponse = await iotHubRegistry.getTwin(deviceId);

        await twinResponse.responseBody.update({
            properties: {
                desired: {
                    image: [...greyScaled.data]
                }
            }
        });

        return endWithResponse(context, 200, "Successfully set image of device");
    }
    catch (err) {
        console.log(err.message);
        return endWithResponse(context, 500, "Could not update device twin");
    }
};

function endWithResponse(context: Context, status: number, message: string = "Bad Request"): void {
    context.log(message);
    context.res = {
        status: status,
        body: message,
    };
}

export default httpTrigger;