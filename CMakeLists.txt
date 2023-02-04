cmake_minimum_required(VERSION 3.0)
project(onvif-cgo)

set(CMAKE_C_FLAGS "-DWITH_DOM -DWITH_OPENSSL -DWITH_NONAMESPACES")
aux_source_directory(./soap/ SRC_LIST)
include_directories(./ /usr/include/ ./soap/custom)
link_directories(~/ /usr/local/ /usr/lib/)
#add_executable(gsoap-onvif ${SRC_LIST} client.c client.h ./soap/custom)
#target_link_libraries(gsoap-onvif -lpthread -ldl -lssl -lcrypto)
#ADD_LIBRARY(c_onvif SHARED ${SRC_LIST} client.c)
ADD_LIBRARY(c_onvif_static STATIC ${SRC_LIST} client.c client.h ./soap/custom)


