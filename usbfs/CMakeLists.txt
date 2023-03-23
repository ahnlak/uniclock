# CMakeLists.txt for usbfs

add_library(usbfs
  ${CMAKE_CURRENT_LIST_DIR}/ff.c
  ${CMAKE_CURRENT_LIST_DIR}/ffunicode.c  
  ${CMAKE_CURRENT_LIST_DIR}/storage.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ufs.cpp
  ${CMAKE_CURRENT_LIST_DIR}/usb.cpp
  ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.cpp
)

target_include_directories(usbfs PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(usbfs
  pico_stdlib pico_unique_id 
  hardware_flash tinyusb_device
)