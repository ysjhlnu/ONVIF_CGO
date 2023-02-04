cmake_minimum_required(VERSION 3.0)
project(onvif-cgo)

set(CMAKE_C_FLAGS "-DWITH_DOM -DWITH_OPENSSL -DWITH_NONAMESPACES")
aux_source_directory(./soap/ SRC_LIST)
include_directories(./)
add_executable(gsoap-onvif ${SRC_LIST} onvif_cgo_client.c onvif_cgo_client.h tcp_server.c cJSON.c ./soap/custom)
target_link_libraries(gsoap-onvif -lpthread -ldl -lssl -lcrypto)



