cmake_minimum_required (VERSION 2.8)

project (rt_controller C CXX)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
      set (CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_CXX_FLAGS}")
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)

use_cxx11()

# Base directories
set(RT_CONTROLLER_DIR src)
set(RT_CONTROLLER_BIN_DIR build)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

#Find threads library
find_package (Threads)

# Create and generate protobuf messages library
include(FindProtobuf)
find_package(Protobuf REQUIRED)

set(PROTOBUF_IMPORT_DIR "src/MESSAGES/V2")

set (msgs
    ${PROTOBUF_IMPORT_DIR}/progran.proto
    ${PROTOBUF_IMPORT_DIR}/header.proto
    ${PROTOBUF_IMPORT_DIR}/controller_commands.proto
    ${PROTOBUF_IMPORT_DIR}/mac_primitives.proto
    ${PROTOBUF_IMPORT_DIR}/stats_messages.proto
    ${PROTOBUF_IMPORT_DIR}/stats_common.proto
    ${PROTOBUF_IMPORT_DIR}/time_common.proto
    ${PROTOBUF_IMPORT_DIR}/config_messages.proto
    ${PROTOBUF_IMPORT_DIR}/config_common.proto
)

set(GENERATED_PROTO_DIR proto_messages)

set(protoc_call "tools/generate_protobuf")
set(protobuf_generated_dir ${RT_CONTROLLER_BIN_DIR})
set(PRPT_CC_DIR ${protobuf_generated_dir}/${GENERATED_PROTO_DIR})

execute_process(COMMAND ${protoc_call} ${PRPT_CC_DIR} ${PROTOBUF_IMPORT_DIR} ${msgs})
file(GLOB PRPT_source ${PRPT_CC_DIR}/*.cc)

set(PRPT_RT_CONTROLLER_generated
    ${PRPT_CC_DIR}/header.pb.cc
    ${PRPT_CC_DIR}/progran.pb.cc
    ${PRPT_CC_DIR}/stats_common.pb.cc
    ${PRPT_CC_DIR}/stats_messages.pb.cc
    ${PRPT_CC_DIR}/time_common.pb.cc
    ${PRPT_CC_DIR}/controller_commands.pb.cc
    ${PRPT_CC_DIR}/mac_primitives.pb.cc
    ${PRPT_CC_DIR}/config_messages.pb.cc
    ${PRPT_CC_DIR}/config_common.pb.cc    
    )
  
file(GLOB prpt_h ${PRPT_CC_DIR}/*.h)
set(prpt_h ${prpt_h} )
 
add_library(PRPT_MSG
  ${PRPT_RT_CONTROLLER_generated}
  ${PRPT_source}
  )
set(PRPT_MSG_LIB PRPT_MSG)
  
message("prpt cpp dir is : ${PRPT_CC_DIR}")
include_directories (${PRPT_CC_DIR})

# Boost support

set(Boost_USE_STATIC_LIBS OFF)

find_package(Boost 1.54.0 REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif(Boost_FOUND)

# Network component

set (RT_CONTROLLER_NETWORK_DIR ${RT_CONTROLLER_DIR}/network)

include_directories(${RT_CONTROLLER_NETWORK_DIR})

set(NETWORK_SRC
  ${RT_CONTROLLER_NETWORK_DIR}/async_xface.cc
  ${RT_CONTROLLER_NETWORK_DIR}/connection_manager.cc
  ${RT_CONTROLLER_NETWORK_DIR}/agent_session.cc
  ${RT_CONTROLLER_NETWORK_DIR}/protocol_message.cc
)
  
add_library(RT_CONTROLLER_NETWORK_LIB
  ${NETWORK_SRC}
)

# RIB component

set (RT_CONTROLLER_RIB_DIR ${RT_CONTROLLER_DIR}/rib)

include_directories(${RT_CONTROLLER_RIB_DIR})

set(RIB_SRC
  ${RT_CONTROLLER_RIB_DIR}/rib_updater.cc
  ${RT_CONTROLLER_RIB_DIR}/rib.cc
  ${RT_CONTROLLER_RIB_DIR}/enb_rib_info.cc
)

add_library(RT_CONTROLLER_RIB_LIB
  ${RIB_SRC}
)

# Core controller

set (RT_CONTROLLER_CORE ${RT_CONTROLLER_DIR}/core)

add_executable(rt_controller ${RT_CONTROLLER_CORE}/rt_controller.cc)

target_link_libraries(rt_controller
  RT_CONTROLLER_RIB_LIB
  RT_CONTROLLER_NETWORK_LIB
  ${PRPT_MSG_LIB}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${PROTOBUF_LIBRARY}
)
