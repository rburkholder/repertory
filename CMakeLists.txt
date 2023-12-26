cmake_policy(SET CMP0048 NEW)

project(
  Repertory
    LANGUAGES CXX
  )

cmake_minimum_required(VERSION 3.13)

# <<< General set up >>>
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID "x64")

option(OU_USE_MQTT       "enable MQTT build"              ON)
option(OU_USE_Telegram   "enable Telegram build"          ON)
option(OU_USE_STATIC_LIB "enable build of static library" ON)
option(OU_USE_SHARED_LIB "enable build of shared library" ON)

message(STATUS "Build type set to ${CMAKE_BUILD_TYPE}")
message(STATUS "${PROJECT_NAME} will be installed to ${CMAKE_INSTALL_PREFIX}")

include(GNUInstallDirs)

# Offer the user the choice of overriding the installation directories
# need to set these to pulic or something to propogate to includes
set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")

#set(DEF_INSTALL_CMAKEDIR lib/cmake/${PROJECT_NAME}) # does not appear to work
#set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")

# Report to user
foreach(p LIB BIN INCLUDE CMAKE)
  file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}DIR} _path )
  message(STATUS "Installing ${p} components to ${_path}")
  unset(_path)
endforeach()

if(OU_USE_MQTT)
  add_subdirectory(MQTT)
endif()

if(OU_USE_Telegram)
  add_subdirectory(Telegram)
endif()

# sudo cmake --build . --target install
# pushd build; sudo cmake --build . --target install; popd