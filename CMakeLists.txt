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

# Enable low-latency kernel (for kernel 3.14 and above)
option(LOWLATENCY "Enable soft-real time scheduling (for Linux kernel 3.14 and above)")

if(LOWLATENCY)
add_definitions(-DLOWLATENCY)
include_directories("/usr/include")
endif()

# Set constants for abstract simulation mode
if(ABSTRACT_SIMULATION)
add_definitions(-DABSTRACT_SIMULATION)
endif()

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
    ${PROTOBUF_IMPORT_DIR}/control_delegation.proto
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
    ${PRPT_CC_DIR}/control_delegation.pb.cc    
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
  ${RT_CONTROLLER_NETWORK_DIR}/tagged_message.cc
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
  ${RT_CONTROLLER_RIB_DIR}/rib_common.cc
  ${RT_CONTROLLER_RIB_DIR}/ue_mac_rib_info.cc
  ${RT_CONTROLLER_RIB_DIR}/cell_mac_rib_info.cc
)

add_library(RT_CONTROLLER_RIB_LIB
  ${RIB_SRC}
)

# App component

set (RT_CONTROLLER_APP_DIR ${RT_CONTROLLER_DIR}/app)

include_directories(${RT_CONTROLLER_APP_DIR})

set(APP_SRC
  ${RT_CONTROLLER_APP_DIR}/stats_manager.cc
  ${RT_CONTROLLER_APP_DIR}/remote_scheduler.cc
  ${RT_CONTROLLER_APP_DIR}/remote_scheduler_helper.cc
  ${RT_CONTROLLER_APP_DIR}/remote_scheduler_primitives.cc
  ${RT_CONTROLLER_APP_DIR}/enb_scheduling_info.cc
  ${RT_CONTROLLER_APP_DIR}/ue_scheduling_info.cc
  ${RT_CONTROLLER_APP_DIR}/remote_scheduler_delegation.cc
  ${RT_CONTROLLER_APP_DIR}/delegation_manager.cc
)

add_library(RT_CONTROLLER_APP_LIB
  ${APP_SRC}
)


# Core controller

set (RT_CONTROLLER_CORE_DIR ${RT_CONTROLLER_DIR}/core)

include_directories(${RT_CONTROLLER_CORE_DIR})

set (RT_CONTROLLER_CORE_SRC
    ${RT_CONTROLLER_CORE_DIR}/rt_wrapper.cc
    ${RT_CONTROLLER_CORE_DIR}/task_manager.cc
    ${RT_CONTROLLER_CORE_DIR}/rt_task.cc
    ${RT_CONTROLLER_CORE_DIR}/requests_manager.cc
)	

add_library(RT_CONTROLLER_CORE_LIB
  ${RT_CONTROLLER_CORE_SRC}
)	

add_executable(rt_controller ${RT_CONTROLLER_CORE_DIR}/rt_controller.cc)

target_link_libraries(rt_controller
  RT_CONTROLLER_APP_LIB
  RT_CONTROLLER_CORE_LIB
  RT_CONTROLLER_RIB_LIB
  RT_CONTROLLER_NETWORK_LIB
  ${PRPT_MSG_LIB}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${PROTOBUF_LIBRARY}
)
