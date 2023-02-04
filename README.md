go+gSoap+onvif学习总结：7、进行镜头调焦、聚焦和预置点的增删改查

cd ./soap/
//注意这里的typemap.dat是上节我们修改过的，不然还会出现duration.c编译报错
wsdl2h -c -t ./typemap.dat -o onvif.h http://www.onvif.org/onvif/ver10/network/wsdl/remotediscovery.wsdl https://www.onvif.org/ver10/device/wsdl/devicemgmt.wsdl https://www.onvif.org/ver10/events/wsdl/event.wsdl https://www.onvif.org/ver10/media/wsdl/media.wsdl https://www.onvif.org/ver20/ptz/wsdl/ptz.wsdl https://www.onvif.org/ver20/imaging/wsdl/imaging.wsdl

vim onvif.h
添加 #import "wsse.h"

//需要依赖如下文件，从gSoap源码拷贝到我们的工程中（拷贝一次后续就可以一直用了）
dom.c、dom.h、wsseapi.c、wsseapi.h、smdevp.c、smdevp.h、mecevp.c、mecevp.h、threads.c、threads.h、wsaapi.c、wsaapi.h
//注意下面的文件需要custom文件夹，否则路径不对
struct_timeval.h、struct_timeval.c
//duration.c以及duration.h对于生成框架代码和编译c代码路径可能会有差异，根据实际情况可能需要两个位置都有

//只生成客户端源文件，我们只实现客户端，不添加-C会多生成服务端代码
soapcpp2 -x -L -C onvif.h
