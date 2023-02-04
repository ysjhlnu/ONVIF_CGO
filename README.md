cgo+gSoap+onvif学习总结：4、实现设备鉴权并获取设备信息


cd ./soap/
wsdl2h -c -t ./typemap.dat -o onvif.h http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl https://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl https://www.onvif.org/ver10/events/wsdl/event.wsdl

vim onvif.h
添加 #import "wsse.h"

//需要依赖如下文件，从gSoap源码拷贝到我们的工程中
dom.c、dom.h、wsseapi.c、wsseapi.h、smdevp.c、smdevp.h、mecevp.c、mecevp.h、threads.c、threads.h、wsaapi.c、wsaapi.h
//注意下面的文件需要custom文件夹，否则路径不对
struct_timeval.h、struct_timeval.c
//duration.c以及duration.h对于生成框架代码和编译c代码路径可能会有差异，根据实际情况可能需要两个位置都有

//只生成客户端源文件，我们只实现客户端，不添加-C会多生成服务端代码
soapcpp2 -x -L -C onvif.h



------------------------------------------

报错修改SOAP_ENV__Fault结构体
https://blog.csdn.net/qq_38795209/article/details/102968656

注意事项

SOAP_ENV__Fault重复定义，屏蔽掉gsoap-2.8/gsoap/import/wsa5.h 中SOAP_ENV__Fault的定义即可。

undefined reference to `soap_in_xsd__duration'，复制gsoap-2.8/gsoap/custom下的duration.h和duration.c。
