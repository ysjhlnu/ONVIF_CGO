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

    PTZ:for {
        direction := uint(0)
        fmt.Println("输入PTZ：1:上；2：下；3：左；4：右；5：左上；6：左下；7：右上；8：右下；9：停，0：退出")
        fmt.Scanln(&direction)
        switch (direction) {
        case 0:
            break PTZ
        case 1,2,3,4,5,6,7,8,9:
            C.ptz(soap, username, password, C.int(direction), C.float(0.5), &profileToken[0], &mediaAddr[0])
            continue
        default:
            fmt.Println("Unknown direction.")
        }
    }

    C.del_soap(soap)

    C.free(unsafe.Pointer(username))
    C.free(unsafe.Pointer(password))
    C.free(unsafe.Pointer(serviceAddr))
}