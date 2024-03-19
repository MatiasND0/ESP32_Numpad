# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/hhouse/esp/esp-idf/components/bootloader/subproject"
  "/home/hhouse/eclipse-workspace/ESP_Numpad/build/bootloader"
  "/home/hhouse/eclipse-workspace/ESP_Numpad/build/bootloader-prefix"
  "/home/hhouse/eclipse-workspace/ESP_Numpad/build/bootloader-prefix/tmp"
  "/home/hhouse/eclipse-workspace/ESP_Numpad/build/bootloader-prefix/src/bootloader-stamp"
  "/home/hhouse/eclipse-workspace/ESP_Numpad/build/bootloader-prefix/src"
  "/home/hhouse/eclipse-workspace/ESP_Numpad/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/hhouse/eclipse-workspace/ESP_Numpad/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
