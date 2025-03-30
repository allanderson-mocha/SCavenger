# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/johnnyc/esp/ESP8266_RTOS_SDK/components/bootloader/subproject"
  "/home/johnnyc/capstone/build/bootloader"
  "/home/johnnyc/capstone/build/bootloader-prefix"
  "/home/johnnyc/capstone/build/bootloader-prefix/tmp"
  "/home/johnnyc/capstone/build/bootloader-prefix/src/bootloader-stamp"
  "/home/johnnyc/capstone/build/bootloader-prefix/src"
  "/home/johnnyc/capstone/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/johnnyc/capstone/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/johnnyc/capstone/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
