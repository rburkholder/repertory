project(
  telegram
  VERSION 1.0.0
  )

# https://cmake.org/cmake/help/v3.6/module/FindBoost.html
#set(Boost_ARCHITECTURE "-x64")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_DEBUG 1)
#set(Boost_REALPATH ON)
#set(BOOST_ROOT "/usr/local")
#set(Boost_DETAILED_FAILURE_MSG ON)
set(BOOST_INCLUDEDIR "/usr/local/include/boost")
set(BOOST_LIBRARYDIR "/usr/local/lib")

#find_package(Boost 1.81.0 REQUIRED COMPONENTS program_options thread filesystem log)
find_package(Boost REQUIRED COMPONENTS log)

set(DEF_OUTPUT_NAME ou_${PROJECT_NAME})

set(
  file_hpp_public
    Bot.hpp
  )

set(
  file_hpp_private
    root_certificates.hpp
    one_shot.hpp
  )

set(
  file_cpp
    one_shot.cpp
    Bot.cpp
)

set(DEF_LIB_Shared ${PROJECT_NAME}_shared)

add_library(
  ${DEF_LIB_Shared} SHARED
  ${file_hpp_public}
  ${file_hpp_private}
  ${file_cpp}
  )

set_target_properties(
  ${DEF_LIB_Shared}
    PROPERTIES
      PUBLIC_HEADER "${file_hpp_public}"
      OUTPUT_NAME ${DEF_OUTPUT_NAME}
      #DEBUG_POSTFIX "_d"
  )

target_link_libraries(
  ${DEF_LIB_Shared}
    #pthread
    ${Boost_LIBRARIES}
    crypto
    ssl
  )

target_compile_definitions(${DEF_LIB_Shared} PUBLIC BOOST_LOG_DYN_LINK )

set(DEF_LIB_Static ${PROJECT_NAME}_static)

add_library(
  ${DEF_LIB_Static} STATIC
  ${file_hpp_public}
  ${file_hpp_private}
  ${file_cpp}
  )

set_target_properties(
  ${DEF_LIB_Static}
    PROPERTIES
      PUBLIC_HEADER ${file_hpp_public}
      OUTPUT_NAME ${DEF_OUTPUT_NAME}
      #DEBUG_POSTFIX "_d"
  )

target_link_libraries(
  ${DEF_LIB_Static}
    #pthread
    ${Boost_LIBRARIES}
    crypto
    ssl
  )

target_compile_definitions(${DEF_LIB_Static} PUBLIC BOOST_LOG_DYN_LINK )

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
