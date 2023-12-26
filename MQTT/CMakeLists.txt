project(
  mqtt
  VERSION 1.0.0
  )

set(
  file_hpp_public
    config.hpp
    mqtt.hpp
  )

set(
  file_hpp_private
  )

set(
  file_cpp
    mqtt.cpp
  )

set(DEF_OUTPUT_NAME ou_${PROJECT_NAME})

set(DEF_LIB_Shared ${PROJECT_NAME}_shared)

add_library(
  ${DEF_LIB_Shared} SHARED
  ${file_hpp_public}
  ${file_cpp}
  )

target_link_libraries(
  ${DEF_LIB_Shared}
    PUBLIC
      libpaho-mqtt3c.a
  )

set(DEF_LIB_Static ${PROJECT_NAME}_static)

add_library(
  ${DEF_LIB_Static} STATIC
  ${file_hpp_public}
  ${file_cpp}
  )

target_link_libraries(
  ${DEF_LIB_Static}
    PUBLIC
      libpaho-mqtt3c.a
  )

set_target_properties(
  ${DEF_LIB_Shared}
  ${DEF_LIB_Static}
    PROPERTIES
      PUBLIC_HEADER "${file_hpp_public}"
      OUTPUT_NAME ${DEF_OUTPUT_NAME}
      #DEBUG_POSTFIX "_d"
  )

set(DEF_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/ou/${PROJECT_NAME})

install(
  TARGETS
    ${DEF_LIB_Static}
    ${DEF_LIB_Shared}
  ARCHIVE
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
  RUNTIME
    DESTINATION ${INSTALL_BINDIR}
    COMPONENT bin
  LIBRARY
    DESTINATION ${INSTALL_LIBDIR}
    COMPONENT lib
  PUBLIC_HEADER
    DESTINATION ${DEF_INCLUDE_DIR}
    COMPONENT dev
  #INCLUDES
  #  DESTINATION ${DEF_INCLUDE_DIR}
  #  COMPONENT dev

)
