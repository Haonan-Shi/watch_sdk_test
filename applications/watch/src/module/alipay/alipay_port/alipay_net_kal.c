/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "alipay_common.h"

//#include <sys/select.h>
//#include <netdb.h>
//#include <arpa/inet.h>
//#include <sys/socket.h>
#include <time.h>
#include <stdarg.h>
//#include <fcntl.h>
//#include <unistd.h>
#include <stdlib.h>
//#include <pthread.h>
#include <stdio.h>
#include "alipay_net_kal.h"
#include "alipay_pan.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "os_sync.h"
#include "alipay_config.h"
#if CONFIG_ALIPAY_TRANSIT

typedef void *alipay_iot_mutex;

alipay_iot_mutex alipay_iot_mutex_create(const char *mutex_name)
{
    DBG_DIRECT("[Alipay] alipay_iot_mutex_create");
#if 0
#warning alipay_iot_mutex_create demo
    alipay_iot_mutex mutex = (alipay_iot_mutex)malloc(sizeof(pthread_mutex_t));
    if (!mutex)
    {
        //printf("alipay_iot_mutex_create:out of memory\n");
        return NULL;
    }

    int v = pthread_mutex_init((pthread_mutex_t *)mutex, NULL);
    if (0 == v)
    {
        return mutex;
    }
    //printf("alipay_iot_mutex_create: Init alipay_iot_mutex failed\n");
    free(mutex);
#endif
    return NULL;
}

int alipay_iot_mutex_lock(alipay_iot_mutex mutex)
{
    DBG_DIRECT("[Alipay] alipay_iot_mutex_lock");
#if 0
#warning alipay_iot_mutex_lock demo
    if (!mutex)
    {
        //printf("alipay_iot_mutex_lock: Mutex is NULL\n");
        return -1;
    }

    return pthread_mutex_lock((pthread_mutex_t *)mutex);
#else
    return 0;

#endif //0
}

int alipay_iot_mutex_unlock(alipay_iot_mutex mutex)
{
    DBG_DIRECT("[Alipay] alipay_iot_mutex_unlock");
#if 0
#warning alipay_iot_mutex_unlock demo
    if (!mutex)
    {
        //printf("alipay_iot_mutex_unlock: Mutex is NULL\n");
        return -1;
    }

    return pthread_mutex_unlock((pthread_mutex_t *)mutex);
#else
    return 0;

#endif
}

int alipay_iot_mutex_delete(alipay_iot_mutex mutex)
{
    DBG_DIRECT("[Alipay] alipay_iot_mutex_delete");
#if 0
#warning alipay_iot_mutex_delete demo
    if (!mutex)
    {
        //printf("alipay_iot_mutex_delete: Mutex is NULL\n");
        return -1;
    }

    int v = pthread_mutex_destroy((pthread_mutex_t *)mutex);
    if (v == 0)
    {
        free(mutex);
    }

    return v;
#else
    return 0;
#endif
}

/*************************socket adapt*************************/
static bool alipay_socket_init_flag = false;
static int32_t alipay_sockets[ALIPAY_IOT_MAX_IP_SOCKET_NUM] = {-1};
// static aos_mutex_t alipay_socket_lock;
static alipay_iot_mutex alipay_socket_lock = NULL;

static alipay_iot_mutex get_mtx()
{
    DBG_DIRECT("[Alipay] get_mtx");
#if 1//0
    static alipay_iot_mutex s_mtx_socket;
    static bool s_isinitialized = false;
    if (!s_isinitialized)
    {
        //s_mtx_socket = alipay_iot_mutex_create("socket_mutex");
        int ret = os_mutex_create(&s_mtx_socket);//os_mutex_create
        if (ret != true)
        {
            DBG_DIRECT("[Alipay] Error!!!, mutex create failed!");
            return NULL;
        }
        s_isinitialized = true;
    }
    return s_mtx_socket;
#else
    return 0;
#endif //0
}

static int alipay_iot_fd_init()
{
    DBG_DIRECT("[Alipay] alipay_iot_fd_init");
    int ret = 0;

    if (alipay_socket_init_flag == true)
    {
        DBG_DIRECT("alipay fd have already inited, should be called twice\n");
        return -1;
    }

    memset(alipay_sockets, -1, sizeof(alipay_sockets));
    alipay_socket_init_flag = true;
    return ret;
}

static int alloc_alipay_socket(int origin_fd)
{
    DBG_DIRECT("[Alipay] alloc_alipay_socket");
    if (alipay_socket_init_flag != true)
    {
        alipay_iot_fd_init();
        DBG_DIRECT("alipay fd haven't init yet\n");
        return -1;
    }
    for (int i = 0; i < ALIPAY_IOT_MAX_IP_SOCKET_NUM; i++)
    {
        if (alipay_sockets[i] == -1)
        {
            alipay_sockets[i] = origin_fd;
            alipay_iot_mutex_unlock(get_mtx());
            return i;
        }
    }
#if 0
    int i;

    if (alipay_socket_init_flag != true)
    {
        alipay_iot_fd_init();
        // //printf("alipay fd haven't init yet\n");
        // return -1;
    }

    alipay_iot_mutex_lock(get_mtx());

    for (i = 0; i < ALIPAY_IOT_MAX_IP_SOCKET_NUM; i++)
    {
        if (alipay_sockets[i] == -1)
        {
            alipay_sockets[i] = origin_fd;
            alipay_iot_mutex_unlock(get_mtx());
            return i;
        }
    }

    alipay_iot_mutex_unlock(get_mtx());
#endif
    return -1;
}

static int free_alipay_socket(int s)
{
    DBG_DIRECT("[Alipay] free_alipay_socket");
#if 1//0
    if (alipay_socket_init_flag != true)
    {
        // //printf("alipay fd haven't init yet\n");
        // return -1;
        alipay_iot_fd_init();
    }

    if (s >= ALIPAY_IOT_MAX_IP_SOCKET_NUM || s < 0)
    {
        return -1;
    }

    alipay_iot_mutex_lock(get_mtx());
    alipay_sockets[s] = -1;
    alipay_iot_mutex_unlock(get_mtx());
#endif
    return 0;
}

static int get_origin_fd_by_alipay_socket(int socket)
{
    DBG_DIRECT("[Alipay] get_origin_fd_by_alipay_socket");
#if 1//0
    int fd = -1;
    if (alipay_socket_init_flag != true)
    {
        // //printf("alipay fd haven't init yet\n");
        // return -1;
        alipay_iot_fd_init();
    }

    if (socket >= ALIPAY_IOT_MAX_IP_SOCKET_NUM || socket < 0)
    {
        //printf("invalid input socket %d\n", socket);
        return -1;
    }

    alipay_iot_mutex_lock(get_mtx());
    fd = alipay_sockets[socket];
    alipay_iot_mutex_unlock(get_mtx());
    return fd;
#else
    return 0;
#endif
}

static void alipay_iot_fd_setResultbit(int fd, alipay_iot_fd_set *fdset)
{
    DBG_DIRECT("[Alipay] alipay_iot_fd_setResultbit");
#if 1 //0
    if (fdset != NULL && fd >= 0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM)
    {
        fdset->fd_bits[fd] |= 0x02;
    }
#endif
}

static int alipay_iot_fd_check(int fd, alipay_iot_fd_set *fdset)
{
    DBG_DIRECT("[Alipay] alipay_iot_fd_check");
#if 1//0
    if (fdset != NULL && fd >= 0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM)
    {
        return fdset->fd_bits[fd] & 0x01;
    }
#endif
    return 0;
}

int alipay_iot_fd_isset(int fd, alipay_iot_fd_set *fdset)
{
    //DBG_DIRECT("[Alipay] alipay_iot_fd_isset");
    //printf("isset fd: %d\n", fd);
#if 0

    if (fdset != NULL && fd >= 0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM)
    {
        return fdset->fd_bits[fd] & 0x02;
    }
    return 0;
#else
    //    return FD_ISSET(fd, fdset);
    AliPay_LOG("[Alipay] isset fd %d", fd);
    return FD_ISSET(fd, fdset);
#endif
}

void alipay_iot_fd_setbit(int fd, alipay_iot_fd_set *fdset)
{
    //DBG_DIRECT("[Alipay] alipay_iot_fd_setbit");
    //printf("setbit fd: %d\n", fd);
#if 0
    if (fdset != NULL && fd >= 0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM)
    {
        fdset->fd_bits[fd] |= 0x01;
    }
#else
    AliPay_LOG("[Alipay] setbit fd %d", fd);
    FD_SET(fd, fdset);
#endif
}

void alipay_iot_fd_zero(alipay_iot_fd_set *fdset)
{
    //DBG_DIRECT("[Alipay] alipay_iot_fd_zero");
#if 0
    if (fdset != NULL)
    {
        memset(fdset, 0, sizeof(alipay_iot_fd_set));
    }
#else
    AliPay_LOG("[Alipay] alipay_iot_fd_zero");
    FD_ZERO(fdset);
#endif
}

int alipay_iot_select(int maxfdp1,
                      alipay_iot_fd_set *readset,
                      alipay_iot_fd_set *writeset,
                      alipay_iot_fd_set *exceptset,
                      alipay_iot_timeval *timeout)
{
    DBG_DIRECT("[Alipay] alipay_iot_select");
#if 0
#warning alipay_iot_select demo
    fd_set  set_r, set_w, set_e;
    int i = 0;
    int origin_fd = -1;
    int origin_max = -1;
    int count = 0;

    FD_ZERO(&set_r);
    FD_ZERO(&set_w);
    FD_ZERO(&set_e);

    if (maxfdp1 < 0)
    {
        //printf("maxfdp1 error %d\n", maxfdp1);
        return -1;
    }

    //printf("maxfdp1 is %d, timeout sec %d, usec %d\r\n ", maxfdp1, timeout->tv_sec, timeout->tv_usec);
    for (i = 0; i < maxfdp1 + 1; i++)
    {
        origin_fd = get_origin_fd_by_alipay_socket(i);
        if (origin_fd == -1)
        {
            //printf("fail to get alipay sock %d origin sock\r\n", i);
            continue;
        }

        if (origin_fd > origin_max - 1)
        {
            origin_max = origin_fd + 1;
        }

        if (alipay_iot_fd_check(i, readset))
        {
            //printf("set fd %d read\n", origin_fd);
            FD_SET(origin_fd, &set_r);
        }

        if (alipay_iot_fd_check(i, writeset))
        {
            //printf("set fd %d write\n", origin_fd);
            FD_SET(origin_fd, &set_w);
        }

        if (alipay_iot_fd_check(i, exceptset))
        {
            //printf("set fd %d except\n", origin_fd);
            FD_SET(origin_fd, &set_e);
        }

        //printf("maxfp1 is %d , i is %d, fd is %d maxfd is %d\n",maxfdp1, i, origin_fd, origin_max);
    }

    struct timeval iTimeout = {0};
    if (timeout != NULL)
    {
        iTimeout.tv_sec = timeout->tv_sec;
        iTimeout.tv_usec = timeout->tv_usec;
    }
    count = select(origin_max, &set_r, &set_w, &set_e, &iTimeout);
    //printf("select return count is 0x%x\n", count);

    for (i = 0; i < maxfdp1 + 1; i++)
    {
        origin_fd = get_origin_fd_by_alipay_socket(i);
        if (origin_fd == -1)
        {
            continue;
        }

        if (FD_ISSET(origin_fd, &set_r))
        {
            alipay_iot_fd_setResultbit(i, readset);
        }
        if (FD_ISSET(origin_fd, &set_w))
        {
            alipay_iot_fd_setResultbit(i, writeset);
        }
        if (FD_ISSET(origin_fd, &set_e))
        {
            alipay_iot_fd_setResultbit(i, exceptset);
        }
    }

    return count;
#else
    return select(maxfdp1, (fd_set *)readset, (fd_set *)writeset, (fd_set *)exceptset,
                  (struct timeval *)timeout);
#endif
}

int alipay_iot_dns(const char *name, unsigned char ip[4])
{
    DBG_DIRECT("[Alipay] alipay_iot_dns");
    struct hostent *he = gethostbyname(name);
    if (he == NULL)
    {
        DBG_DIRECT("dns gethostbyname fail name %s\n", name);
        return -1;
    }
    if (he->h_addr_list == NULL)
    {
        DBG_DIRECT("dns he->h_addr_list is null host name: %s\n", name);
        return -1;
    }
    if (he->h_addr_list[0] == NULL)
    {
        DBG_DIRECT("dns he->h_addr_list[0] is null,host name: %s\n", name);
        return -1;
    }
    memcpy(ip, he->h_addr_list[0], 4);
    DBG_DIRECT("[dns ip bytes]: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    return 0;
}


int alipay_iot_socket_create(int domain, alipay_socket_type_enum type, int protocol)
{
#if 0
#warning alipay_iot_socket_create demo

    DBG_DIRECT("[Alipay] alipay_iot_socket_create type: %d\n", type);

    bool alipay_bt_pan_connect(void);
    if (false == alipay_bt_pan_connect())
    {
        AliPay_LOG("[Alipay] alipay_bt_pan_connect, failed!");
        return -1;
    }

    int socket_type = -1;
    int fd = -1;
    switch (type)
    {
    case ALIPAY_IOT_SOC_SOCK_STREAM:
        {
            socket_type = SOCK_STREAM;
            break;
        }
    case ALIPAY_IOT_SOC_SOCK_DGRAM:
        {
            socket_type = SOCK_DGRAM;
            break;
        }
    default:
        {
            return -1;
        }
    }

    int socket_fd = -1;
    if ((socket_fd = socket(PF_INET, socket_type, 0)) < 0)
    {
        return -1;
    }

    //printf("socket_fd: %d\n", socket_fd);
    fd = alloc_alipay_socket(socket_fd);
    if (fd == -1)
    {
        DBG_DIRECT("fail to create alipay socket 0x%x\n", fd);
        close(socket_fd);
        return -1;
    }

    DBG_DIRECT("[Alipay] alipay_iot_socket_create fd: %d\n", fd);
    return fd;
#else
    DBG_DIRECT("[Alipay] alipay_iot_socket_create");
    // alipay pan create
    bool alipay_bt_pan_connect(void);
    if (false == alipay_bt_pan_connect())
    {
        AliPay_LOG("[Alipay] alipay_bt_pan_connect, failed!");
        return -1;
    }

    //create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0) ;
    AliPay_LOG("[Alipay] create socket, fd %d", fd);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500 * 1000;
    lwip_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    return fd;//fd
#endif
}

int alipay_iot_socket_close(int s)
{
    DBG_DIRECT("[Alipay] alipay_iot_socket_close");
#if 0
#warning alipay_iot_socket_close demo
    int fd = -1;
    int ret  = -1;

    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1)
    {
        return -1;
    }

    ret = closesocket(s);//close(fd);

    ret |= free_alipay_socket(s);
    if (ret != 0)
    {
        //printf("close socket %d origin is %d ret %d", s, fd, ret);
        return -1;
    }

    //printf("close socket %d origin is %d ret %d", s, fd, ret);
    return ret;
#else
    AliPay_LOG("[Alipay] socket disconnect, s %d!", s);

    //lwip pan
    return closesocket(s);//alipay_tcp_data_send(cmd_data, offset);
#endif
}


int alipay_iot_socket_bind(int                        s,
                           const struct alipay_iot_sockaddr *name,
                           unsigned int               namelen)
{
#if 0
#warning alipay_iot_socket_bind demo
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(name->data.sin_data.port);
    memcpy(&serv_addr.sin_addr, name->data.sin_data.ip, 4);

    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1)
    {
        //printf("fail to connect alipay sock %d\n", s);
        return -1;
    }
    int val = bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (val < 0)
    {
        //printf("alipay_iot_socket_bind failed, %d", val);
        return -1;
    }

    return 0;
#else
    return 0;
#endif
}

int alipay_iot_socket_connect(int                            s,
                              const struct alipay_iot_sockaddr *name,
                              unsigned int                    namelen)
{
#if 0
#warning alipay_iot_socket_connect demo
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    // serv_addr.sin_port = htons(name->data.sin_data.port);
    serv_addr.sin_port = htons(443);
    //printf("[test]ip sin_port: %d\n", serv_addr.sin_port);
    memcpy(&serv_addr.sin_addr, name->data.sin_data.ip, 4);

    char str[100] = {0};
    inet_ntop(AF_INET, &(serv_addr.sin_addr), str, INET_ADDRSTRLEN);
    //printf("ip address: %s\n", str);
    //printf("port: %d - %d\n", serv_addr.sin_port, name->data.sin_data.port);

    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1)
    {
        DBG_DIRECT("fail to connect alipay sock %d\n", s);
        return -1;
    }
    int err = connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (err < 0)
    {
        DBG_DIRECT("[Alipay] socket connect fail fd %d err %d", fd, err);
        return -1;
    }

    DBG_DIRECT("[Alipay] socket connect success");

    return 0;
#else
    DBG_DIRECT("[Alipay] socket connect port %d, addr %d:%d:%d:%d", name->data.sin_data.port,
               name->data.sin_data.ip[0], name->data.sin_data.ip[1], name->data.sin_data.ip[2],
               name->data.sin_data.ip[3]);
    struct sockaddr_in lwip_name ;
    memset(&lwip_name, 0, sizeof(lwip_name));
    lwip_name.sin_len = sizeof(lwip_name);
    lwip_name.sin_family = AF_INET;//name->sa_family;
    lwip_name.sin_port = htons(443);;//htons(name->data.sin_data.port);
    //memcpy(&lwip_name.sin_addr, name->data.sin_data.ip, 4);
    uint8_t *psrc = (uint8_t *)&lwip_name.sin_addr;
    uint8_t *pdst = (uint8_t *)name->data.sin_data.ip;
    psrc[0] = pdst[0];
    psrc[1] = pdst[1];
    psrc[2] = pdst[2];
    psrc[3] = pdst[3];

    int ret = connect(s, (struct sockaddr *)&lwip_name, sizeof(lwip_name));

    DBG_DIRECT("[Alipay] socket connect ret %d", ret);

    return ret;
#endif
}

int alipay_iot_socket_sendto(int                           s,
                             const void                  *dataptr,
                             int                          size,
                             int                            flags,
                             const struct alipay_iot_sockaddr *to,
                             unsigned int                    tolen)
{
#if 0
#warning alipay_iot_socket_sendto demo
    //printf("send socket fd: %d\n", s);
    struct sockaddr_in server_addr;
    server_addr.sin_family = to->sa_family;
    server_addr.sin_port = htons(443);
    // server_addr.sin_port = htons(to->data.sin_data.port);
    //printf("[test]sin_port %d\n",server_addr.sin_port);
    memcpy(&server_addr.sin_addr, to->data.sin_data.ip, 4);

    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1)
    {
        //printf("fail to connect alipay sock %d\n", s);
        return -1;
    }
    int bytes_send = sendto(fd, dataptr, size, 0, &server_addr, sizeof(server_addr));
    //printf("bytes send: %d\n", bytes_send);
    return bytes_send;
#else
    AliPay_LOG("[Alipay] send to!");
    alipay_iot_socket_write(s, dataptr, size);

    return 0;
#endif
}

int alipay_iot_socket_write(int s, const void *dataptr, int len)
{

#if 0
#warning alipay_iot_socket_write demo
    //printf("write fd: %d - len: %d\n", s, len);
    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1)
    {
        //printf("fail to connect alipay sock %d\n", s);
        return -1;
    }
    return write(fd, dataptr, len);
#else
    // int val = (int)write(s, dataptr, len);
    // return (val);
    AliPay_LOG("[Alipay] alipay_iot_socket_write, s %d", s);
    return write(s, dataptr, len);
#endif
}

int alipay_iot_socket_recvfrom(int                     s,
                               void                        *mem,
                               int                       len,
                               int                        flags,
                               struct alipay_iot_sockaddr *from,
                               unsigned int            *fromlen)
{
#if 0
#warning alipay_iot_socket_recvfrom demo
    //printf("socket recvfrom: %d\n", s);
    int bytes_received = -1;

    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1)
    {
        //printf("fail to connect alipay sock %d\n", s);
        return -1;
    }

    if (from)
    {
        struct sockaddr_in server_addr;
        server_addr.sin_family = from->sa_family;
        /* server_addr.sin_port = htons(443); */
        server_addr.sin_port = htons(from->data.sin_data.port);
        memcpy(&server_addr.sin_addr, from->data.sin_data.ip, 4);

        int addrlen = sizeof(server_addr);
        bytes_received = recvfrom(fd, mem, len, flags, &server_addr, &addrlen);
        //printf("bytes_received: %d\n", bytes_received);
    }
    else
    {
        //fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
        bytes_received = recv(fd, mem, len, flags);
        //printf("bytes_received: %d\n", bytes_received);
    }

    return bytes_received;
#else
    AliPay_LOG("[Alipay] alipay_iot_socket_recvfrom");
    return recvfrom(s, mem, len, flags, (struct sockaddr *)from, fromlen);
#endif
}

int alipay_iot_socket_read(int s, void *mem, int len)
{
#if 0
#warning alipay_iot_socket_read demo
    //printf("read fd: %d - len: %d\n", s, len);
    int n = alipay_iot_socket_recvfrom(s, mem, len, 0, NULL, NULL);
    /* int n =  read(s, mem, len); */
    //printf("socket read %d\n", n);
    if (n <= 0 || n > len)
    {
        return -2;
    }

    return n;
#else
    AliPay_LOG("[Alipay] alipay_iot_socket_read");
    return read(s, mem, len); // yuyin
#endif
}


int alipay_iot_socket_setsockopt(int          s,
                                 int          level,
                                 int          optname,
                                 const void  *opval,
                                 unsigned int optlen)
{
//#warning alipay_iot_socket_setsockopt demo
    AliPay_LOG("[Alipay] alipay_iot_socket_setsockopt");
    return 0;
}

int alipay_iot_socket_getsockopt(int         s,
                                 int            level,
                                 int          optname,
                                 void          *opval,
                                 unsigned int *optlen)
{
//#warning alipay_iot_socket_getsockopt demo
    AliPay_LOG("[Alipay] alipay_iot_socket_getsockopt");
    return 0;
}

#endif

