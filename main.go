package main

/*
#cgo CFLAGS: -I ./ -I /usr/local/ -I /home/ts/Downloads/gsoap_2.8.124/gsoap-2.8/gsoap/
#cgo LDFLAGS: -L ./build -lc_onvif_static -lpthread -ldl -lssl -lcrypto
#include "client.h"
*/
import "C"

func main() {
    C.discovery();
    C.get_device_info(C.CString("ts"), C.CString("ts"), C.CString("http://192.168.2.19:80/onvif/device_service"));
}
