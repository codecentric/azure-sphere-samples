#  Copyright (c) codecentric AG. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required (VERSION 3.10)

project (IoTHubD2D C)

azsphere_configure_tools(TOOLS_REVISION "20.10")
azsphere_configure_api(TARGET_API_SET "8")

add_subdirectory("../../MT3620_Grove_Shield_Library" groveOut)
add_subdirectory("../../Library" out)

# Create executable
add_executable (${PROJECT_NAME} main.c utils.c)
target_link_libraries (${PROJECT_NAME} applibs pthread gcc_s c azureiot MT3620_Grove_Shield_Library Library)
azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "HardwareDefinitions/mt3620_rdb" TARGET_DEFINITION "template_appliance.json")

azsphere_target_add_image_package(${PROJECT_NAME})
