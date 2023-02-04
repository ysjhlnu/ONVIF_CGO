//
// Created by admin on 2022/6/21.
//

/*
* 文件：tcp_server.c
* 内容：利用TCP实现客户端和服务器的实时聊天。
* 注  ：服务器的端口号及IP，客户端的端口号及IP的输入通过main函数的argc和argv来实现。
* 未输入端口号的话使用默认端口号,服务器为1111,客户端为2222。
* 编译：gcc tcp_server.c -o ser -lpthread
* 运行：./ser 127.0.0.1 7788 5
*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <cJSON.h>
#include "onvif_cgo_client.h"

#define SPORT 1111
#define IP "127.0.0.1"
#define MAXLEN 1024

struct network_info
{
    struct sockaddr addr;
    int addr_len;
    int sock;
}target = {0,0,0};

enum action {
    GET_DEV_INFO = 1,
    GET_CAPABILITIES = 2,
    GET_PROFILES = 3,
    GET_RTSP_URI = 4,
    GET_SNAPSHOT_URI = 5,
    GET_VIDEO_SOURCE = 6,
    PTZ = 7,
    PRESET = 8
};

int send_message(struct network_info netinfo, char buf[200]) {
    int len = sendto(netinfo.sock, buf, strlen(buf), 0, (struct sockaddr*)(&(netinfo.addr)), netinfo.addr_len);
    if(len < 0)
    {
        printf("msg is:%s,send failer,errno is %d,errno message is:%s\n",buf,errno,strerror(errno));
        return len;
    }

    return 0;
}

int parse_recv_message(struct network_info networkInfo, char *buf) {
    cJSON *root = NULL;
    cJSON *action = NULL;
    const cJSON *userNameObj = NULL;
    const cJSON *passwordObj = NULL;
    const cJSON *serviceAddrObj = NULL;
    const cJSON *direction = NULL;
    const cJSON *speed = NULL;
    const cJSON *presetAction = NULL;
    const cJSON *presetToken = NULL;
    const cJSON *presetName = NULL;
    char res[200] = {'\0'};
    struct soap *soap = NULL;
    char username[200] = {'\0'};
    char password[200] = {'\0'};
    char serviceAddr[200] = {'\0'};
    char mediaAddr[200] = {'\0'};
    char profileToken[200] = {'\0'};
    char rtspUri[200] = {'\0'};
    char videoSourceToken[200] = {'\0'};

    root = cJSON_Parse(buf);
    if (NULL == root) {
        printf("file:%s,line:%d.Parse root failed.\n", __FILE__, __LINE__);
        return -1;
    }

    action = cJSON_GetObjectItem(root, "action");
    if (NULL == action) {
        cJSON_Delete(root);
        printf("file:%s,line:%d.Get action failed.\n", __FILE__, __LINE__);
        return -2;
    }

    userNameObj = cJSON_GetObjectItem(root, "username");
    if (NULL == userNameObj) {
        cJSON_Delete(root);
        printf("file:%s,line:%d.Get username failed.\n", __FILE__, __LINE__);
        return -3;
    }
    strcpy(username, userNameObj->valuestring);

    passwordObj = cJSON_GetObjectItem(root, "password");
    if (NULL == passwordObj) {
        cJSON_Delete(root);
        printf("file:%s,line:%d.Get password failed.\n", __FILE__, __LINE__);
        return -4;
    }
    strcpy(password, passwordObj->valuestring);

    serviceAddrObj = cJSON_GetObjectItem(root, "serviceAddr");
    if (NULL == serviceAddrObj) {
        cJSON_Delete(root);
        printf("file:%s,line:%d.Get service addr failed.\n", __FILE__, __LINE__);
        return -5;
    }
    strcpy(serviceAddr, serviceAddrObj->valuestring);

    switch (action->valueint) {
        case GET_DEV_INFO:
            break;
        case GET_CAPABILITIES:
            break;
        case GET_PROFILES:
            break;
        case GET_RTSP_URI:
            soap = new_soap(soap);
            get_device_info(soap, username, password, serviceAddr);
            get_capabilities(soap, username, password, serviceAddr, mediaAddr);
            get_profiles(soap, username, password, profileToken, mediaAddr);
            if (0 == get_rtsp_uri(soap, username, password, profileToken, mediaAddr, rtspUri)) {
                strcpy(res, "get rtsp success");
            } else {
                strcpy(res, "get rtsp failed");
            }
            send_message(networkInfo, res);
            del_soap(soap);
            break;
        case GET_SNAPSHOT_URI:
            break;
        case GET_VIDEO_SOURCE:
            break;
        case PTZ:
            soap = new_soap(soap);
            get_device_info(soap, username, password, serviceAddr);
            get_capabilities(soap, username, password, serviceAddr, mediaAddr);
            get_profiles(soap, username, password, profileToken, mediaAddr);
            get_video_source(soap, username, password, videoSourceToken, mediaAddr);

            int dir = 0;
            float ptzSpeed = 0;
            direction = cJSON_GetObjectItem(root, "direction");
            if (direction != NULL) {
                dir = direction->valueint;
            }
            speed = cJSON_GetObjectItem(root, "speed");
            if (speed != NULL) {
                ptzSpeed = speed->valuedouble;
            }
            printf("1-14分别代表上、下、左、右、左上、左下、右上、右下、停止、缩、放、调焦加、调焦减、调焦停止,dir=%d,speed=%f\n", dir, ptzSpeed);
            if ((dir >= 1) && (dir <= 11)) {
                if (0 == ptz(soap, username, password, dir, ptzSpeed, profileToken, mediaAddr)) {
                    strcpy(res, "ptz success");
                } else {
                    strcpy(res, "ptz failed");
                }
            } else if (dir >= 12 && dir <= 14) {
                if (0 == focus(soap, username, password, dir, ptzSpeed, videoSourceToken, mediaAddr)) {
                    strcpy(res, "focus success");
                } else {
                    strcpy(res, "focus failed");
                }
            }
            send_message(networkInfo, res);
            del_soap(soap);
            break;
        case PRESET:
            break;
        default:
            break;
    }
    cJSON_Delete(root);

    return 0;
}

void* myrecv(void* arg)
{
    int new_fd = 0,len = 0;
    char buf[MAXLEN] = {'\0'};
    time_t t;
    struct network_info netinfo;
    netinfo = *((struct network_info*)arg);
    new_fd = netinfo.sock;

    while(1)
    {
        memset(buf, '\0', sizeof(buf)-1);
        len = recv(new_fd, buf, sizeof(buf) - 1, 0);
        if(len > 0)
        {
            t = time(NULL);
            printf("&client:%s    @%s\n", buf, ctime(&t));
            parse_recv_message(netinfo, buf);
        }
        else if(len < 0)
        {
            printf("recv msg error,error code = %d,error msg = %s\n",errno, strerror(errno));
            break;
        }
        else
        {
            printf("client is quit\n");
            close(new_fd);
            break;
        }
    }
}

int main(int argc,char* argv[])
{
    int pid = 0,sock_fd = 0,new_fd = 0;
    socklen_t len;
    struct sockaddr_in self_addr,their_addr;
    unsigned int myport,lisnum;
    char ip[17] = {'\0'};
    time_t t;
    pthread_t pthread_recv;

    if(argv[2] && argc >=3)
        myport = atoi(argv[2]);
    else
        myport = SPORT;

    if(argv[3] && argc >= 4)
        lisnum = atoi(argv[3]);
    else
        lisnum = 3;

    //创建套接字
    sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd < 0)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    //绑定自己的端口号
    memset(&self_addr, 0, sizeof(self_addr));
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(myport);
    if(argv[1])
        self_addr.sin_addr.s_addr = inet_addr(argv[1]);
    else
        self_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock_fd, (struct sockaddr*)&self_addr, sizeof(self_addr)) == -1)
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    //监听
    if(listen(sock_fd,lisnum) == -1)
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    //接受客户端的连接请求
    memset(&their_addr, 0, sizeof(their_addr));
    len = sizeof(their_addr);
    printf("wait for connect\n");
    while(1)
    {
        if((new_fd = accept(sock_fd, (struct sockaddr*)&their_addr, &len)) == -1)
        {
            printf("len=%d,new_fd=%d\n", len, new_fd);
            perror("accept error");
            exit(EXIT_FAILURE);
        }
        printf("=====================================================================\n");
        printf("client connect ok\n");
        printf("=====================================================================\n");
        memcpy(&(target.addr), &(their_addr), len);
        target.addr_len = len;
        target.sock = new_fd;

        //创建线程用于进程间通信
        pthread_create(&pthread_recv, NULL, myrecv, (void*)&target);
    }

    pthread_join(pthread_recv, NULL);
    close(new_fd);
    close(sock_fd); //关闭套接字

    return 0;
}
