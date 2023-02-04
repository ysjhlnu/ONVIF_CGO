package main

/*
#cgo CFLAGS: -I ./ -I /usr/local/ -I ~/work/gsoap-2.8/gsoap/
#cgo LDFLAGS: -L ./build -lc_onvif_static -lpthread -ldl -lssl -lcrypto
#include "client.h"
#include "malloc.h"
*/
import "C"

import (
    "fmt"
    "unsafe"
)

func main() {
    var soap C.P_Soap
    soap = C.new_soap(soap)
    username := C.CString("ts")
    password := C.CString("ts")
    serviceAddr := C.CString("http://192.168.2.19:80/onvif/device_service")

    C.discovery(soap)

    C.get_device_info(soap, username, password, serviceAddr)

    mediaAddr := [200]C.char{}
    C.get_capabilities(soap, username, password, serviceAddr, &mediaAddr[0])
    fmt.Println(C.GoString(&mediaAddr[0]))

    profileToken := [200]C.char{}
    C.get_profiles(soap, username, password, &profileToken[0], &mediaAddr[0])
    fmt.Println(C.GoString(&profileToken[0]))

    C.get_rtsp_uri(soap, username, password, &profileToken[0], &mediaAddr[0])

    C.get_snapshot(soap, username, password, &profileToken[0], &mediaAddr[0])

    C.del_soap(soap)

    C.free(unsafe.Pointer(username))
    C.free(unsafe.Pointer(password))
    C.free(unsafe.Pointer(serviceAddr))
}
