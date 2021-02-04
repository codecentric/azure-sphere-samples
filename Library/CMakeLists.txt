CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(Library C)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    "exit.h"
    "gpio.c"
    "gpio.h"
    "led.c"
    "led.h"
    "sleep.c"
    "sleep.h"
    "parson.c"
    "parson.h"
    "azure_iot_utilities.c"
    "azure_iot_utilities.h"
)
source_group("" FILES ${no_group_source_files})

set(ALL_FILES
    ${no_group_source_files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})


set(ROOT_NAMESPACE Library)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "AzureSphere"
)

target_link_libraries (${PROJECT_NAME} applibs pthread gcc_s c azureiot)
azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "HardwareDefinitions/mt3620_rdb" TARGET_DEFINITION "template_appliance.json")

################################################################################
# Compile definitions
################################################################################

################################################################################
# Compile and link options
################################################################################
#if(MSVC)
#    target_compile_options(${PROJECT_NAME} PRIVATE
#        $<$<CONFIG:Debug>:
#            /showIncludes;
#            ${DEFAULT_CXX_DEBUG_RUNTIME_LIBRARY}
#        >
#        $<$<CONFIG:Release>:
#            ${DEFAULT_CXX_RUNTIME_LIBRARY}
#        >
#        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
#        ${DEFAULT_CXX_EXCEPTION_HANDLING}
#    )
#endif()

################################################################################
# Dependencies
################################################################################
#target_link_directories(${PROJECT_NAME} PUBLIC
#    "${CMAKE_CURRENT_SOURCE_DIR}/./azureiot/lib"
#)

