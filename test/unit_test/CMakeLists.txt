get_filename_component(ENGINE_CMAKE_DIR ${CMAKE_BINARY_DIR} DIRECTORY)
get_filename_component(ENGINE_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} DIRECTORY)


find_package(Boost QUIET COMPONENTS date_time thread filesystem iostreams REQUIRED)
find_package (Threads REQUIRED)

find_library(Spine_LIBRARY smartmet-spine)
find_path(Spine_INCLUDE_DIR ConfigBase.h ${CMAKE_INSTALL_PREFIX} /usr/include/smartmet/spine)
if (NOT Spine_INCLUDE_DIR)
  message(FATAL_ERROR "SmartMet Spine include director not found")
endif()

find_library(Avi_LIBRARY avi.so ${Avi_SOURCE_DIR})
if (NOT Avi_LIBRARY)
  message(FATAL_ERROR "SmartMet Avi Plugin library not found")
endif()
execute_process(COMMAND ln -f -s ${Avi_LIBRARY} libavi.so WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(Avi_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/libavi.so)

find_library(CONFIGPP_LIBRARY config++)
if (NOT CONFIGPP_LIBRARY)
  message(FATAL_ERROR "libconfig++ library not found")
endif()

find_library(Avi_Engine_LIBRARY avi.so /usr/share/smartmet/engines)
if (NOT Avi_Engine_LIBRARY)
  message(FATAL_ERROR "SmartMet Avi Engine library not found")
endif()
execute_process(COMMAND ln -f -s ${Avi_Engine_LIBRARY} libengineavi.so WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(Avi_Engine_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/libengineavi.so)

find_library(Authentication_Engine_LIBRARY authentication.so /usr/share/smartmet/engines)
if (NOT Authentication_Engine_LIBRARY)
  message(FATAL_ERROR "SmartMet Authentication Engine library not found")
endif()
execute_process(COMMAND ln -f -s ${Authentication_Engine_LIBRARY} libengineauthentication.so WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(Authentication_Engine_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/libengineauthentication.so)

get_filename_component(SmartMet_INCLUDE_DIR ${Spine_INCLUDE_DIR} DIRECTORY)
file(GLOB UNIT_TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
include_directories(${CMAKE_BINARY_DIR}/include ${SmartMet_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
add_compile_options(-std=c++11 -Wall -W -Wno-unused-parameter)
set(CMAKE_CXX_FLAGS "-DUNIX")


# Copy config files
file(GLOB SAMPLE_CONFIC_FILES ${Avi_SOURCE_DIR}/test/cnf/*.conf)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cnf)
foreach(configFileName ${SAMPLE_CONFIC_FILES})
  file(COPY ${configFileName} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/cnf)
endforeach(configFileName)

foreach(testSrc ${UNIT_TEST_SRCS})
  get_filename_component(testName ${testSrc} NAME_WE)
  add_executable(${testName} ${testSrc})
  target_link_libraries(
    ${testName}
    ${CMAKE_STANDARD_LIBRARIES}
    ${Boost_LIBRARIES}
    ${Spine_LIBRARY}
    ${Avi_Engine_LIBRARY}
    ${Authentication_Engine_LIBRARY}
    ${Avi_LIBRARY}
    ${CONFIGPP_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT})

  # Move testing binaries into a testBin directory
  set_target_properties(${testName} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR})
  add_test(NAME ${testName}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${testName})
endforeach(testSrc)
