# This is a standard copy of <PICO_SDK_PATH>/external/pico_sdk_import.cmake

if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    message(Using PICO_SDK_PATH from environment (''))
endif ()

if (NOT PICO_SDK_PATH)
    set(PICO_SDK_PATH /pico-sdk)
    if (NOT EXISTS )
        message(FATAL_ERROR PICO_SDK_PATH is not defined and pico-sdk directory was not found. Please set PICO_SDK_PATH in your environment.)
    endif ()
endif ()

set(PICO_SDK_PATH " CACHE PATH Path to the Raspberry Pi Pico SDK)

include(/pico_sdk_init.cmake)
