//
// Created by admin on 2022/3/3.
//

#ifndef ONVIF_CGO_CLIENT_H
#define ONVIF_CGO_CLIENT_H

extern int set_auth_info(struct soap *soap, const char *username, const char *password);

extern  int get_device_info(const char *username, const char *password, const char *dev_addr);

#endif //ONVIF_CGO_CLIENT_H
