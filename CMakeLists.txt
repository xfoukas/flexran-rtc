cmake_minimum_required (VERSION 3.1)

project (rt_controller)

# Base directories
set(RT_CONTROLLER_DIR src)
set(RT_CONTROLLER_BIN_DIR build)


# Create and generate protobuf messages library
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

set(GENERATED_PROTO_DIR proto_messages/)
set(protoc_call "tools/generate_protobuf")
set(protobuf_generated_dir ${RT_CONTROLLER_BIN_DIR})
set(PRPT_CC_DIR ${protobuf_generated_dir}/${GENERATED_PROTO_DIR})

execute_process(COMMAND ${protoc_call} ${PRPT_CC_DIR} ${PROTOBUF_IMPORT_DIR} ${msgs})
file(GLOB PRPT_source ${PRPT_CC_DIR}/*.cc)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

set(PRPT_PACER_generated
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
    ${PRPT_PACER_generated}
    ${PRPT_source}
    )
  set(PRPT_MSG_LIB PRPT_MSG)
  #message("prpt cpp dir is : ${PRPT_CC_DIR}")
  include_directories (${PRPT_CC_DIR})

# Core controller

set (RT_CONTROLLER_CORE ${RT_CONTROLLER_DIR}/core)

add_executable(rt_controller ${RT_CONTROLLER_CORE}/rt_controller.cc )

set_property(TARGET rt_controller PROPERTY CXX_STANDARD 11)
set_property(TARGET rt_controller PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(rt_controller
    ${PROTOBUF_LIBRARY}
    ${PRPT_MSG_LIB}
)
