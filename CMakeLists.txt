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

set (CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")

# Enable low-latency kernel (for kernel 3.14 and above)
option(LOWLATENCY "Enable soft-real time scheduling (for Linux kernel 3.14 and above)" ON)

if(LOWLATENCY)
add_definitions(-DLOWLATENCY)
include_directories("/usr/include")
endif()

# Set constants for abstract simulation mode
if(ABSTRACT_SIMULATION)
add_definitions(-DABSTRACT_SIMULATION)
endif()

# Enable RESTful northbound API
option(REST_NORTHBOUND "Enable RESTful northbound API for the management of the controller" ON)

if(REST_NORTHBOUND)
add_definitions(-DREST_NORTHBOUND)
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
    ${PROTOBUF_IMPORT_DIR}/flexran.proto
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
set(FLPT_CC_DIR ${protobuf_generated_dir}/${GENERATED_PROTO_DIR})

execute_process(COMMAND ${protoc_call} ${FLPT_CC_DIR} ${PROTOBUF_IMPORT_DIR} ${msgs})
file(GLOB FLPT_source ${FLPT_CC_DIR}/*.cc)

set(FLPT_RT_CONTROLLER_generated
    ${FLPT_CC_DIR}/header.pb.cc
    ${FLPT_CC_DIR}/flexran.pb.cc
    ${FLPT_CC_DIR}/stats_common.pb.cc
    ${FLPT_CC_DIR}/stats_messages.pb.cc
    ${FLPT_CC_DIR}/time_common.pb.cc
    ${FLPT_CC_DIR}/controller_commands.pb.cc
    ${FLPT_CC_DIR}/mac_primitives.pb.cc
    ${FLPT_CC_DIR}/config_messages.pb.cc
    ${FLPT_CC_DIR}/config_common.pb.cc
    ${FLPT_CC_DIR}/control_delegation.pb.cc    
    )
  
file(GLOB flpt_h ${FLPT_CC_DIR}/*.h)
set(flpt_h ${flpt_h} )
 
add_library(FLPT_MSG
  ${FLPT_RT_CONTROLLER_generated}
  ${FLPT_source}
  )
set(FLPT_MSG_LIB FLPT_MSG)
  
message("flpt cpp dir is : ${FLPT_CC_DIR}")
include_directories (${FLPT_CC_DIR})

# Boost support

set(Boost_USE_STATIC_LIBS OFF)

find_package(Boost 1.54.0 REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif(Boost_FOUND)

if(REST_NORTHBOUND)

# Pistache support

set(PISTACHE_INCLUDE_DIRS /usr/local/include/pistache)
set(PISTACHE_LIB /usr/local/lib/libnet_static.a)

include_directories(${PISTACHE_INCLUDE_DIRS})

endif()

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
#  ${RT_CONTROLLER_APP_DIR}/remote_scheduler.cc
  ${RT_CONTROLLER_APP_DIR}/remote_scheduler_helper.cc
  ${RT_CONTROLLER_APP_DIR}/remote_scheduler_primitives.cc
  ${RT_CONTROLLER_APP_DIR}/enb_scheduling_info.cc
  ${RT_CONTROLLER_APP_DIR}/ue_scheduling_info.cc
#  ${RT_CONTROLLER_APP_DIR}/remote_scheduler_delegation.cc
#  ${RT_CONTROLLER_APP_DIR}/delegation_manager.cc
#  ${RT_CONTROLLER_APP_DIR}/remote_scheduler_eicic.cc
  ${RT_CONTROLLER_APP_DIR}/flexible_scheduler.cc	
)

add_library(RT_CONTROLLER_APP_LIB
  ${APP_SRC}
)

if(REST_NORTHBOUND)

# North_API component

set (RT_CONTROLLER_NORTH_API_DIR ${RT_CONTROLLER_DIR}/north_api)

include_directories(${RT_CONTROLLER_NORTH_API_DIR})

set(NORTH_API_SRC
  ${RT_CONTROLLER_NORTH_API_DIR}/call_manager.cc
  ${RT_CONTROLLER_NORTH_API_DIR}/flexible_sched_calls.cc
  ${RT_CONTROLLER_NORTH_API_DIR}/stats_manager_calls.cc
)

add_library(RT_CONTROLLER_NORTH_API_LIB
  ${NORTH_API_SRC}
)

endif()

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
  ${FLPT_MSG_LIB}
  ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${PROTOBUF_LIBRARY}
)

if(REST_NORTHBOUND)

target_link_libraries(rt_controller
  RT_CONTROLLER_NORTH_API_LIB
  ${PISTACHE_LIB}
)

endif()
