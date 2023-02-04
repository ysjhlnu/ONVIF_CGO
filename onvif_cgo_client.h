//
// Created by admin on 2022/3/3.
//

#ifndef ONVIF_CGO_CLIENT_H
#define ONVIF_CGO_CLIENT_H

struct soap *new_soap(struct soap *soap);

void del_soap(struct soap *soap);

int get_device_info(struct soap *soap, const char *username, const char *password, char *xAddr);

int get_capabilities(struct soap *soap, const char *username, const char *password, char *xAddr, char *mediaAddr);

int get_profiles(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr);

int get_rtsp_uri(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr,
                 char *rtsp_uri);

int get_snapshot(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr,
                 char *snapshot_uri);

int get_video_source(struct soap *soap, const char *username, const char *password, char *videoSource, char *xAddr);

int ptz(struct soap *soap, const char *username, const char *password, int direction, float speed, char *profileToken,
        char *xAddr);

int
focus(struct soap *soap, const char *username, const char *password, int direction, float speed, char *videoSourceToken,
      char *xAddr);

#endif //ONVIF_CGO_CLIENT_H
