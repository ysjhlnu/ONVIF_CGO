package main

/*
#cgo CFLAGS: -I ./ -I /usr/local/
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

    profileToken := [200]C.char{}
    C.get_profiles(soap, username, password, &profileToken[0], &mediaAddr[0])

    C.get_rtsp_uri(soap, username, password, &profileToken[0], &mediaAddr[0])

    C.get_snapshot(soap, username, password, &profileToken[0], &mediaAddr[0])

    videoSourceToken := [200]C.char{}
    C.get_video_source(soap, username, password, &videoSourceToken[0], &mediaAddr[0])

    PTZ:for {
        direction := uint(0)
        fmt.Println("请输入数字进行ptz,1-14分别代表上、下、左、右、左上、左下、右上、右下、停止、缩、放、调焦加、调焦减、调焦停止;退出请输入0：");
        fmt.Scanln(&direction)
        switch (direction) {
        case 0:
            break PTZ
        case 1,2,3,4,5,6,7,8,9,10,11:
            C.ptz(soap, username, password, C.int(direction), C.float(0.5), &profileToken[0], &mediaAddr[0])
            continue
        case 12,13,14:
            C.focus(soap, username, password, C.int(direction), C.float(0.5), &videoSourceToken[0], &mediaAddr[0]);
            continue
        default:
            fmt.Println("Unknown direction.")
        }
    }

    Preset:for {
        presetAction := uint(0)
        fmt.Println("请输入数字进行preset,1-4分别代表查询、设置、跳转、删除预置点；退出输入0:")
        fmt.Scanln(&presetAction)
        switch(presetAction) {
        case 0:
            break Preset
        case 1:
            C.preset(soap, username, password, C.int(presetAction), nil, nil, &profileToken[0], &mediaAddr[0])
        case 2:
            fmt.Println("请输入要设置的预置点token信息：")
            presentToken := ""
            fmt.Scanln(&presentToken)
            fmt.Println("请输入要设置的预置点name信息长度不超过200：")
            presentName := ""
            fmt.Scanln(&presentName)
            C.preset(soap, username, password, C.int(presetAction), C.CString(presentToken), C.CString(presentName), &profileToken[0], &mediaAddr[0])
        case 3:
            fmt.Println("请输入要跳转的预置点token信息：")
            presentToken := ""
            fmt.Scanln(&presentToken)
            C.preset(soap, username, password, C.int(presetAction), C.CString(presentToken), nil, &profileToken[0], &mediaAddr[0])
        case 4:
            fmt.Println("请输入要删除的预置点token信息：")
            presentToken := ""
            fmt.Scanln(&presentToken)
            C.preset(soap, username, password, C.int(presetAction), C.CString(presentToken), nil, &profileToken[0], &mediaAddr[0])
        default:
            fmt.Println("unknown present action.")
            break
        }
    }

    C.del_soap(soap)

    C.free(unsafe.Pointer(username))
    C.free(unsafe.Pointer(password))
    C.free(unsafe.Pointer(serviceAddr))

    return
}
