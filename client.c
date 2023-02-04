//
// Created by admin on 2022/3/3.
//

#include <string.h>
#include "soap/soapStub.h"
#include "soap/wsdd.nsmap"
#include "soap/soapH.h"
#include "soap/wsseapi.h"
#include "client.h"

struct soap *new_soap(struct soap *soap) {
    //soap初始化，申请空间
    soap = soap_new();
    if (soap == NULL) {
        printf("func:%s,line:%d.malloc soap error.\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    soap_set_namespaces(soap, namespaces);
    soap->recv_timeout = 3;
    printf("func:%s,line:%d.new soap success!\n", __FUNCTION__, __LINE__);
    return soap;
}

void del_soap(struct soap *soap) {
    //清除soap
    soap_end(soap);
    soap_free(soap);
}

int discovery(struct soap *soap) {
    //发送消息描述
    struct wsdd__ProbeType req;
    struct __wsdd__ProbeMatches resp;
    //描述查找那类的Web消息
    struct wsdd__ScopesType sScope;
    //soap消息头消息
    struct SOAP_ENV__Header header;
    //获得的设备信息个数
    int count = 0;
    //返回值
    int res;
    //存放uuid 格式(8-4-4-4-12)
    char uuid_string[64];

    printf("func:%s,line:%d.discovery dev!\n", __FUNCTION__, __LINE__);
    if (soap == NULL) {
        printf("func:%s,line:%d.soap is nil.\n", __FUNCTION__, __LINE__);
        return -1;
    }

    sprintf(uuid_string, "464A4854-4656-5242-4530-110000000000");
    printf("func:%s,line:%d.uuid = %s\n", __FUNCTION__, __LINE__, uuid_string);

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

    //调用gSoap接口 向 239.255.255.250:3702 发送udp消息
    res = soap_send___wsdd__Probe(soap, "soap.udp://239.255.255.250:3702/", NULL, &req);

    if (res == -1) {
        printf("func:%s,line:%d.soap error: %d, %s, %s \n", __FUNCTION__, __LINE__, soap->error, *soap_faultcode(soap),
               *soap_faultstring(soap));
        res = soap->error;
    } else {
        do {
            printf("func:%s,line:%d.begin receive probe match, find dev count:%d.... \n", __FUNCTION__, __LINE__,
                   count);

            //接收 ProbeMatches，成功返回0，错误返回-1
            res = soap_recv___wsdd__ProbeMatches(soap, &resp);
            printf("func:%s,line:%d.result=%d \n", __FUNCTION__, __LINE__, res);
            if (res == -1) {
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

    return res;
}

int set_auth_info(struct soap *soap, const char *username, const char *password) {
    if (NULL == username) {
        printf("func:%s,line:%d.username is null.\n", __FUNCTION__, __LINE__);
        return -1;
    }
    if (NULL == password) {
        printf("func:%s,line:%d.password is nil.\n", __FUNCTION__, __LINE__);
        return -2;
    }

    int result = soap_wsse_add_UsernameTokenDigest(soap, NULL, username, password);

    return result;
}

int get_device_info(struct soap *soap, const char *username, const char *password, char *xAddr) {
    if (NULL == xAddr) {
        printf("func:%s,line:%d.dev addr is nil.\n", __FUNCTION__, __LINE__);
        return -1;
    }
    if (soap == NULL) {
        printf("func:%s,line:%d.malloc soap error.\n", __FUNCTION__, __LINE__);
        return -2;
    }

    struct _tds__GetDeviceInformation deviceInformation;
    struct _tds__GetDeviceInformationResponse deviceInformationResponse;

    set_auth_info(soap, username, password);

    int res = soap_call___tds__GetDeviceInformation(soap, xAddr, NULL, &deviceInformation, &deviceInformationResponse);

    if (NULL != soap) {
        printf("Manufacturer:%s\n", deviceInformationResponse.Manufacturer);
        printf("Model:%s\n", deviceInformationResponse.Model);
        printf("FirmwareVersion:%s\n", deviceInformationResponse.FirmwareVersion);
        printf("SerialNumber:%s\n", deviceInformationResponse.SerialNumber);
        printf("HardwareId:%s\n", deviceInformationResponse.HardwareId);
        soap_default__tds__GetDeviceInformation(soap, &deviceInformation);
        soap_default__tds__GetDeviceInformationResponse(soap, &deviceInformationResponse);
    }
    return res;
}

int get_capabilities(struct soap *soap, const char *username, const char *password, char *xAddr, char *mediaAddr) {
    struct _tds__GetCapabilities capabilities;
    struct _tds__GetCapabilitiesResponse capabilitiesResponse;

    set_auth_info(soap, username, password);
    int res = soap_call___tds__GetCapabilities(soap, xAddr, NULL, &capabilities, &capabilitiesResponse);
    if (soap->error) {
        printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error, *soap_faultcode(soap),
               *soap_faultstring(soap));
        return soap->error;
    }
    if (capabilitiesResponse.Capabilities == NULL) {
        printf("func:%s,line:%d.GetCapabilities  failed!  result=%d \n", __FUNCTION__, __LINE__, res);
    } else {
        printf("func:%s,line:%d.Media->XAddr=%s \n", __FUNCTION__, __LINE__,
               capabilitiesResponse.Capabilities->Media->XAddr);
        strcpy(mediaAddr, capabilitiesResponse.Capabilities->Media->XAddr);
    }
    return res;
}

int get_profiles(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr) {
    struct _trt__GetProfiles profiles;
    struct _trt__GetProfilesResponse profilesResponse;
    set_auth_info(soap, username, password);
    int res = soap_call___trt__GetProfiles(soap, xAddr, NULL, &profiles, &profilesResponse);
    if (res == -1)
        //NOTE: it may be regular if result isn't SOAP_OK.Because some attributes aren't supported by server.
        //any question email leoluopy@gmail.com
    {
        printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error, *soap_faultcode(soap),
               *soap_faultstring(soap));
        return soap->error;
    }

    if (profilesResponse.__sizeProfiles <= 0) {
        printf("func:%s,line:%d.Profiles Get Error\n", __FUNCTION__, __LINE__);
        return res;
    }

    for (int i = 0; i < profilesResponse.__sizeProfiles; i++) {
        if (profilesResponse.Profiles[i].token != NULL) {
            printf("func:%s,line:%d.Profiles token:%s\n", __FUNCTION__, __LINE__, profilesResponse.Profiles->Name);

            //默认我们取第一个即可，可以优化使用字符串数组存储多个
            if (i == 0) {
                strcpy(profileToken, profilesResponse.Profiles[i].token);
            }
        }
    }
    return res;
}

int get_rtsp_uri(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr) {
    struct _trt__GetStreamUri streamUri;
    struct _trt__GetStreamUriResponse streamUriResponse;
    streamUri.StreamSetup = (struct tt__StreamSetup *) soap_malloc(soap, sizeof(struct tt__StreamSetup));
    streamUri.StreamSetup->Stream = 0;
    streamUri.StreamSetup->Transport = (struct tt__Transport *) soap_malloc(soap, sizeof(struct tt__Transport));
    streamUri.StreamSetup->Transport->Protocol = 0;
    streamUri.StreamSetup->Transport->Tunnel = 0;
    streamUri.StreamSetup->__size = 1;
    streamUri.StreamSetup->__any = NULL;
    streamUri.StreamSetup->__anyAttribute = NULL;
    streamUri.ProfileToken = profileToken;
    set_auth_info(soap, username, password);
    int res = soap_call___trt__GetStreamUri(soap, xAddr, NULL, &streamUri, &streamUriResponse);
    if (soap->error) {
        printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error, *soap_faultcode(soap),
               *soap_faultstring(soap));
        return soap->error;
    }
    printf("func:%s,line:%d.RTSP uri is :%s \n", __FUNCTION__, __LINE__, streamUriResponse.MediaUri->Uri);
    return res;
}

int get_snapshot(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr) {
    struct _trt__GetSnapshotUri snapshotUri;
    struct _trt__GetSnapshotUriResponse snapshotUriResponse;

    set_auth_info(soap, username, password);
    snapshotUri.ProfileToken = profileToken;
    int res = soap_call___trt__GetSnapshotUri(soap, xAddr, NULL, &snapshotUri, &snapshotUriResponse);
    if (soap->error) {
        printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error, *soap_faultcode(soap),
               *soap_faultstring(soap));
        return soap->error;
    }
    printf("func:%s,line:%d.Snapshot uri is :%s \n", __FUNCTION__, __LINE__,
           snapshotUriResponse.MediaUri->Uri);
    return res;
}

int get_video_source(struct soap *soap, const char *username, const char *password, char *videoSource, char *xAddr) {
    struct _trt__GetVideoSources getVideoSources;
    struct _trt__GetVideoSourcesResponse getVideoSourcesResponse;

    set_auth_info(soap, username, password);

    int res = soap_call___trt__GetVideoSources(soap, xAddr, NULL, &getVideoSources, &getVideoSourcesResponse);
    if (soap->error) {
        printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error, *soap_faultcode(soap),
               *soap_faultstring(soap));
        return soap->error;
    }
    if (getVideoSourcesResponse.__sizeVideoSources <= 0) {
        printf("func:%s,line:%d.get video sources failed.\n", __FUNCTION__, __LINE__);
        return res;
    } else {
        for (int i = 0; i < getVideoSourcesResponse.__sizeVideoSources; i++) {
            printf("func:%s,line:%d.get video source token:%s\n", __FUNCTION__, __LINE__,
                   getVideoSourcesResponse.VideoSources[i].token);

            //我们暂时只获取第一个videoSourceToken
            if (i == 0) {
                strcpy(videoSource, getVideoSourcesResponse.VideoSources[i].token);
            }
        }
    }
    return res;
}

int ptz(struct soap *soap, const char *username, const char *password, int direction, float speed, char *profileToken,
        char *xAddr) {
    struct _tptz__ContinuousMove continuousMove;
    struct _tptz__ContinuousMoveResponse continuousMoveResponse;
    struct _tptz__Stop stop;
    struct _tptz__StopResponse stopResponse;
    int res;

    set_auth_info(soap, username, password);
    continuousMove.ProfileToken = profileToken;
    continuousMove.Velocity = (struct tt__PTZSpeed *) soap_malloc(soap, sizeof(struct tt__PTZSpeed));
    continuousMove.Velocity->PanTilt = (struct tt__Vector2D *) soap_malloc(soap, sizeof(struct tt__Vector2D));

    switch (direction) {
        case 1:
            continuousMove.Velocity->PanTilt->x = 0;
            continuousMove.Velocity->PanTilt->y = speed;
            break;
        case 2:
            continuousMove.Velocity->PanTilt->x = 0;
            continuousMove.Velocity->PanTilt->y = -speed;
            break;
        case 3:
            continuousMove.Velocity->PanTilt->x = -speed;
            continuousMove.Velocity->PanTilt->y = 0;
            break;
        case 4:
            continuousMove.Velocity->PanTilt->x = speed;
            continuousMove.Velocity->PanTilt->y = 0;
            break;
        case 5:
            continuousMove.Velocity->PanTilt->x = -speed;
            continuousMove.Velocity->PanTilt->y = speed;
            break;
        case 6:
            continuousMove.Velocity->PanTilt->x = -speed;
            continuousMove.Velocity->PanTilt->y = -speed;
            break;
        case 7:
            continuousMove.Velocity->PanTilt->x = speed;
            continuousMove.Velocity->PanTilt->y = speed;
            break;
        case 8:
            continuousMove.Velocity->PanTilt->x = speed;
            continuousMove.Velocity->PanTilt->y = -speed;
            break;
        case 9:
            stop.ProfileToken = profileToken;
            stop.Zoom = NULL;
            stop.PanTilt = NULL;
            res = soap_call___tptz__Stop(soap, xAddr, NULL, &stop, &stopResponse);
            if (soap->error) {
                printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error,
                       *soap_faultcode(soap),
                       *soap_faultstring(soap));
                return soap->error;
            }
            return res;
        case 10:
            continuousMove.Velocity->PanTilt->x = 0;
            continuousMove.Velocity->PanTilt->y = 0;
            continuousMove.Velocity->Zoom = (struct tt__Vector1D *) soap_malloc(soap, sizeof(struct tt__Vector1D));
            continuousMove.Velocity->Zoom->x = speed;
            break;
        case 11:
            continuousMove.Velocity->PanTilt->x = 0;
            continuousMove.Velocity->PanTilt->y = 0;
            continuousMove.Velocity->Zoom = (struct tt__Vector1D *) soap_malloc(soap, sizeof(struct tt__Vector1D));
            continuousMove.Velocity->Zoom->x = -speed;
            break;
        default:
            printf("func:%s,line:%d.Ptz direction unknown.\n", __FUNCTION__, __LINE__);
            return -1;
    }
    res = soap_call___tptz__ContinuousMove(soap, xAddr, NULL, &continuousMove, &continuousMoveResponse);
    if (soap->error) {
        printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error, *soap_faultcode(soap),
               *soap_faultstring(soap));
        return soap->error;
    }

    return res;
}

int
focus(struct soap *soap, const char *username, const char *password, int direction, float speed, char *videoSourceToken,
      char *xAddr) {
    struct _timg__Move timgMove;
    struct _timg__MoveResponse timgMoveResponse;
    struct _timg__Stop timgStop;
    struct _timg__StopResponse timgStopResponse;
    int res;

    set_auth_info(soap, username, password);
    timgMove.Focus = (struct tt__FocusMove *) soap_malloc(soap, sizeof(struct tt__FocusMove));
    timgMove.Focus->Continuous = (struct tt__ContinuousFocus *) soap_malloc(soap, sizeof(struct tt__ContinuousFocus));
    timgMove.VideoSourceToken = videoSourceToken;
    switch (direction) {
        case 12:
            timgMove.Focus->Continuous->Speed = speed;
            break;
        case 13:
            timgMove.Focus->Continuous->Speed = -speed;
            break;
        case 14:
            timgStop.VideoSourceToken = videoSourceToken;
            res = soap_call___timg__Stop(soap, xAddr, NULL, &timgStop, &timgStopResponse);
            if (soap->error) {
                printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error,
                       *soap_faultcode(soap), *soap_faultstring(soap));
                return soap->error;
            }
            return res;
        default:
            printf("unknown direction");
            return -1;
    }
    res = soap_call___timg__Move(soap, xAddr, NULL, &timgMove, &timgMoveResponse);
    if (soap->error) {
        printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error,
               *soap_faultcode(soap), *soap_faultstring(soap));
        return soap->error;
    }
    return res;
}

int preset(struct soap *soap, const char *username, const char *password, int presetAction, char *presetToken,
           char *presetName, char *profileToken, char *xAddr) {
    int res;
    set_auth_info(soap, username, password);
    switch (presetAction) {
        case 1: {
            struct _tptz__GetPresets getPresets;
            struct _tptz__GetPresetsResponse getPresetsResponse;
            getPresets.ProfileToken = profileToken;
            res = soap_call___tptz__GetPresets(soap, xAddr, NULL, &getPresets, &getPresetsResponse);
            if (soap->error) {
                printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error,
                       *soap_faultcode(soap), *soap_faultstring(soap));
                return soap->error;
            }
            if (getPresetsResponse.__sizePreset <= 0) {
                printf("func:%s,line:%d.get presets failed.\n", __FUNCTION__, __LINE__);
                return res;
            }
            for (int i = 0; i < getPresetsResponse.__sizePreset; i++) {
                printf("func:%s,line:%d.preset token:%s,preset name:%s\n", __FUNCTION__, __LINE__,
                       getPresetsResponse.Preset[i].token, getPresetsResponse.Preset[i].Name);
            }
            return res;
        }
        case 2: {
            struct _tptz__SetPreset setPreset;
            struct _tptz__SetPresetResponse setPresetResponse;
            setPreset.ProfileToken = profileToken;
            setPreset.PresetName = presetName;
            setPreset.PresetToken = presetToken;
            res = soap_call___tptz__SetPreset(soap, xAddr, NULL, &setPreset, &setPresetResponse);
            break;
        }
        case 3: {
            struct _tptz__GotoPreset gotoPreset;
            struct _tptz__GotoPresetResponse gotoPresetsResponse;
            gotoPreset.ProfileToken = profileToken;
            gotoPreset.PresetToken = presetToken;
            gotoPreset.Speed = NULL;
            res = soap_call___tptz__GotoPreset(soap, xAddr, NULL, &gotoPreset, &gotoPresetsResponse);
            break;
        }
        case 4: {
            struct _tptz__RemovePreset removePreset;
            struct _tptz__RemovePresetResponse removePresetResponse;
            removePreset.PresetToken = presetToken;
            removePreset.ProfileToken = profileToken;
            res = soap_call___tptz__RemovePreset(soap, xAddr, NULL, &removePreset, &removePresetResponse);
            break;
        }
        default:
            printf("func:%s,line:%d.Unknown preset action.\n", __FUNCTION__, __LINE__);
    }

    if (soap->error) {
        printf("func:%s,line:%d.soap error: %d, %s, %s\n", __FUNCTION__, __LINE__, soap->error,
               *soap_faultcode(soap), *soap_faultstring(soap));
        return soap->error;
    }

    return res;
}

//int main() {
//    struct soap *soap = NULL;
//    soap = new_soap(soap);
//    const char username[] = "ts";
//    const char password[] = "ts";
//    char serviceAddr[] = "http://192.168.2.19:80/onvif/device_service";
//
//    discovery(soap);
//
//    get_device_info(soap, username, password, serviceAddr);
//
//    char mediaAddr[200] = {'\0'};
//    get_capabilities(soap, username, password, serviceAddr, mediaAddr);
//
//    char profileToken[200] = {'\0'};
//    get_profiles(soap, username, password, profileToken, mediaAddr);
//
//    get_rtsp_uri(soap, username, password, profileToken, mediaAddr);
//
//    get_snapshot(soap, username, password, profileToken, mediaAddr);
//
//    char videoSourceToken[200] = {'\0'};
//    get_video_source(soap, username, password, videoSourceToken, mediaAddr);
//
//    int direction = -1;
//    while (direction != 0) {
//        printf("请输入数字进行ptz,1-14分别代表上、下、左、右、左上、左下、右上、右下、停止、缩、放、调焦加、调焦减、调焦停止;退出请输入0：");
//        scanf("%d", &direction);
//        if ((direction >= 1) && (direction <= 11)) {
//            ptz(soap, username, password, direction, 0.5f, profileToken, mediaAddr);
//        } else if (direction >= 12 && direction <= 14) {
//            focus(soap, username, password, direction, 0.5f, videoSourceToken, mediaAddr);
//        }
//    }
//
//    int presetAction = -1;
//    while (presetAction != 0) {
//        printf("请输入数字进行preset,1-4分别代表查询、设置、跳转、删除预置点；退出输入0:\n");
//        scanf("%d", &presetAction);
//        if (1 == presetAction) {
//            preset(soap, username, password, presetAction, NULL, NULL, profileToken, mediaAddr);
//        } else if (2 == presetAction) {
//            printf("请输入要设置的预置点token信息：\n");
//            char presentToken[10];
//            scanf("%s", presentToken);
//            printf("请输入要设置的预置点name信息()长度不超过200：\n");
//            char presentName[201];
//            scanf("%s", presentName);
//            preset(soap, username, password, presetAction, presentToken, presentName, profileToken, mediaAddr);
//        } else if (3 == presetAction) {
//            printf("请输入要跳转的预置点token信息：\n");
//            char presentToken[10];
//            scanf("%s", presentToken);
//            preset(soap, username, password, presetAction, presentToken, NULL, profileToken, mediaAddr);
//        } else if (4 == presetAction) {
//            printf("请输入要删除的预置点token信息：\n");
//            char presentToken[10];
//            scanf("%s", presentToken);
//            preset(soap, username, password, presetAction, presentToken, NULL, profileToken, mediaAddr);
//        }
//    }
//
//    del_soap(soap);
//}
