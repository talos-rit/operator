# Proto library for pre-generated protobuf files

add_library(proto INTERFACE)

target_include_directories(proto INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(proto INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/icd/v1/icd.pb.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/icd/v1/icd.grpc.pb.cc
)
