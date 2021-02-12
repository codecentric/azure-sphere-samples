#include "device_twin.h"

#include <stdio.h>

#include "../../Library/parson.h"
#include "../../Library/azure_iot_utilities.h"

void DeviceTwinReportCapabilities(bool hasGroveShield, bool hasDisplay)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_dotset_boolean(root_object, "deviceCapabilities.hasGroveShield", hasGroveShield);
    json_object_dotset_boolean(root_object, "deviceCapabilities.hasDisplay", hasDisplay);

    AzureIoT_TwinReportStateJson(root_value);

    json_value_free(root_value);
}

void DeviceTwinReportConnectedMasterDevices(int connectedMasterDevices)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    json_object_set_number(root_object, "connectedMasterDevices", connectedMasterDevices);

    AzureIoT_TwinReportStateJson(root_value);

    json_value_free(root_value);
}

void DeviceTwinReportConnectedDevices(int connectedDevices, int nodeDevices, int mainDevices)
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    if (connectedDevices > 0)
    {
        json_object_set_number(root_object, "connectedDevices", connectedDevices);
    }

    if (nodeDevices > 0)
    {
        json_object_set_number(root_object, "connectedNodeDevices", nodeDevices);
    }

    if (mainDevices > 0)
    {
        json_object_set_number(root_object, "connectedMainDevices", mainDevices);
    }

    AzureIoT_TwinReportStateJson(root_value);

    json_value_free(root_value);
}

void DeviceTwinReportLedStates(leds_t *leds)
{
    const led_channels_t ledChannels[] = {leds->one, leds->two, leds->three, leds->four, leds->wifi, leds->status};
    const char *ledJsonKeys[] = {"one", "two", "three", "four", "wifi", "status"};

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);

    char jsonKeyBuffer[32];
    for (uint8_t i = 0; i < sizeof(ledChannels) / sizeof(led_channels_t); i++)
    {
        snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.red", ledJsonKeys[i]);
        json_object_dotset_boolean(root_object, jsonKeyBuffer, GetLedRedState(ledChannels[i]));

        snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.green", ledJsonKeys[i]);
        json_object_dotset_boolean(root_object, jsonKeyBuffer, GetLedGreenState(ledChannels[i]));

        snprintf(jsonKeyBuffer, sizeof(jsonKeyBuffer), "leds.%s.blue", ledJsonKeys[i]);
        json_object_dotset_boolean(root_object, jsonKeyBuffer, GetLedBlueState(ledChannels[i]));
    }

    AzureIoT_TwinReportStateJson(root_value);

    json_value_free(root_value);
}