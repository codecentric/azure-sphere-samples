#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

cmake_minimum_required (VERSION 3.10)
project (TemperatureMonitor C)

azsphere_configure_tools(TOOLS_REVISION "20.04")
azsphere_configure_api(TARGET_API_SET "7")

add_subdirectory("../../MT3620_Grove_Shield_Library" out)
# Create executable
add_executable (${PROJECT_NAME} main.c util.c)
target_link_libraries (${PROJECT_NAME} applibs pthread gcc_s c MT3620_Grove_Shield_Library)

# Add hardware definition
azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "HardwareDefinitions/mt3620_rdb" TARGET_DEFINITION "template_appliance.json")
azsphere_target_add_image_package(${PROJECT_NAME})