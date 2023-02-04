//
// Created by admin on 2022/3/3.
//

#include "soap/soapStub.h"
#include "soap/wsdd.nsmap"
#include "soap/soapH.h"
#include "soap/wsseapi.h"
#include "client.h"

static void del_soap(struct soap *soap) {
    //清除soap
    // clean up and remove deserialized data
    soap_end(soap);
    //detach and free runtime context
    soap_free(soap);
}

int discovery() {
//soap环境变量
    struct soap *soap;

    //发送消息描述
    struct wsdd__ProbeType req;
    struct wsdd__ProbeType wsdd__Probe;

    struct __wsdd__ProbeMatches resp;

    //描述查找那类的Web消息
    struct wsdd__ScopesType sScope;

    //soap消息头消息
    struct SOAP_ENV__Header header;

    //获得的设备信息个数
    int count = 0;

    //返回值
    int result = 0;

    //存放uuid 格式(8-4-4-4-12)
    char uuid_string[64];

    printf("%s: %d 000: \n", __FUNCTION__, __LINE__);
    sprintf(uuid_string, "464A4854-4656-5242-4530-110000000000");
    printf("uuid = %s \n", uuid_string);

    //soap初始化，申请空间
    soap = soap_new();
    if (soap == NULL) {
        printf("malloc soap error \n");
        return -1;
    }

    soap_set_namespaces(soap, namespaces);  //设置命名空间，就是xml文件的头
    soap->recv_timeout = 5;  //超出5s没数据就推出，超时时间

    //将header设置为soap消息，头属性，暂且认为是soap和header绑定
    soap_default_SOAP_ENV__Header(soap, &header);
    header.wsa5__MessageID = uuid_string;
    header.wsa5__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    header.wsa5__Action = "http://schemas.xmllocal_soap.org/ws/2005/04/discovery/Probe";
    //设置soap头消息的ID
    soap->header = &header;

    /* 设置所需寻找设备的类型和范围，二者至少设置一个
        否则可能收到非ONVIF设备，出现异常
     */

    //设置soap消息的请求服务属性
    soap_default_wsdd__ScopesType(soap, &sScope);
    sScope.__item = "onvif://www.onvif.org";
    soap_default_wsdd__ProbeType(soap, &req);
    req.Scopes = &sScope;

    /* 设置所需设备的类型，ns1为命名空间前缀，在wsdd.nsmap 文件中
       {"tdn","http://www.onvif.org/ver10/network/wsdl"}的tdn,如果不是tdn,而是其它,
       例如ns1这里也要随之改为ns1
    */
    req.Types = "ns1:NetworkVideoTransmitter";
//    req.Types = "dn:NetworkVideoTransmitter";

    //调用gSoap接口 向 239.255.255.250:3702 发送udp消息
    result = soap_send___wsdd__Probe(soap, "soap.udp://239.255.255.250:3702/", NULL, &req);

    if (result == -1) {
        printf("soap error: %d, %s, %s \n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        result = soap->error;
    } else {
        do {
            printf("begin receive probematch.... func:%s,line:%d\n", __FUNCTION__, __LINE__);
            printf("count = %d \n", count);

            //接收 ProbeMatches，成功返回0，错误返回-1
            result = soap_recv___wsdd__ProbeMatches(soap, &resp);
            printf(" --soap_recv___wsdd__ProbeMatches() result=%d \n", result);
            if (result == -1) {
                printf("Find %d devices!\n", count);
                break;
            } else {
                //读取服务器回应的Probematch消息
                printf("soap_recv___wsdd__Probe: __sizeProbeMatch = %d \n", resp.wsdd__ProbeMatches->__sizeProbeMatch);
                printf("Target EP Address : %s \n",
                       resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
                printf("Target Type : %s \n", resp.wsdd__ProbeMatches->ProbeMatch->Types);
                printf("Target Service Address : %s \n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
                printf("Target Metadata Version: %d \n", resp.wsdd__ProbeMatches->ProbeMatch->MetadataVersion);
                printf("Target Scope Address : %s \n", resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item);
                count++;
            }
        } while (1);
    }

    del_soap(soap);
    return result;
}

int set_auth_info(struct soap *soap, const char *username, const char *password) {
    int result = 0;

    if (NULL == username) {
        printf("username is null.\n");
        return -1;
    }
    if (NULL == password) {
        printf("password is nil.\n");
        return -2;
    }

    result = soap_wsse_add_UsernameTokenDigest(soap, NULL, username, password);

    return result;
}

int get_device_info(const char *username, const char *password, const char *dev_addr) {
    int result = 0;
    struct soap *soap = NULL;
    struct _tds__GetDeviceInformation devinfo_req;
    struct _tds__GetDeviceInformationResponse devinfo_resp;

    if (NULL == dev_addr) {
        printf("dev addr is nil.\n");
        return -1;
    }
    soap = soap_new();
    if (soap == NULL) {
        printf("malloc soap error \n");
        return -2;
    }
    soap_set_namespaces(soap, namespaces);  //设置命名空间，就是xml文件的头
    soap->recv_timeout = 5;  //超出5s没数据就推出，超时时间

//    set_auth_info(soap, username, password);
    result = soap_wsse_add_UsernameTokenDigest(soap, NULL, username, password);
    printf("result:%d\n", result);

    memset(&devinfo_req, 0x00, sizeof(devinfo_req));
    memset(&devinfo_resp, 0x00, sizeof(devinfo_resp));
    result = soap_call___tds__GetDeviceInformation(soap, dev_addr, NULL, &devinfo_req, &devinfo_resp);

    if (NULL != soap) {
        printf("Manufacturer:%s\n", devinfo_resp.Manufacturer);
        printf("Model:%s\n", devinfo_resp.Model);
        printf("FirmwareVersion:%s\n", devinfo_resp.FirmwareVersion);
        printf("SerialNumber:%s\n", devinfo_resp.SerialNumber);
        printf("HardwareId:%s\n", devinfo_resp.HardwareId);
        del_soap(soap);
    }
    return result;
}

//int main() {
//    discovery();
//    get_device_info("admin", "admin", "http://40.40.40.101:80/onvif/device_service");
//}
