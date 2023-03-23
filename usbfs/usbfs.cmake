# CMakeLists.txt for usbfs

add_library(usbfs INTERFACE)

target_sources(usbfs INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/ff.c
  ${CMAKE_CURRENT_LIST_DIR}/ufs.cpp
)

target_include_directories(usbfs INTERFACE ${CMAKE_CURRENT_LIST_DIR})
