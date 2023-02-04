//
// Created by admin on 2022/3/3.
//

#ifndef ONVIF_CGO_CLIENT_H
#define ONVIF_CGO_CLIENT_H


typedef struct soap *P_Soap;

extern struct soap *new_soap(struct soap *soap);

extern void del_soap(struct soap *soap);

extern int discovery(struct soap *soap);

extern int get_device_info(struct soap *soap, const char *username, const char *password, char *xAddr);

extern int get_capabilities(struct soap *soap, const char *username, const char *password, char *xAddr, char *mediaAddr);

extern int
get_profiles(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr);

extern int
get_rtsp_uri(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr);

extern int
get_snapshot(struct soap *soap, const char *username, const char *password, char *profileToken, char *xAddr);

#endif //ONVIF_CGO_CLIENT_H
