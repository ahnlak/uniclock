# CMakeLists for UniClock
cmake_minimum_required(VERSION 3.12)

# Set the project name
set(NAME uniclock)

# Configure some hardware specific bits
set(PICO_BOARD pico_w)
include(pimoroni_pico_import.cmake)
include(pico_sdk_import.cmake)

# Configure language requirements
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Define all the source files that go into this
add_executable(${NAME}
    uniclock.cpp
)

# Include required library definitions
# This assumes `pimoroni-pico` is stored alongside your project
include(libraries/pico_graphics/pico_graphics)
include(libraries/galactic_unicorn/galactic_unicorn)
include(usbfs/CMakeLists.txt)

# Make sure we can pick up local headers from sub-projects
target_include_directories(${NAME} PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
)

# Define the libraries we need to link in.
target_link_libraries(${NAME}
    pico_stdlib pico_cyw43_arch_lwip_threadsafe_background
    hardware_rtc
    pico_graphics galactic_unicorn usbfs
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
