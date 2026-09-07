/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_PAN_SUPPORT

#define MBEDTLS_ALLOW_PRIVATE_ACCESS //must defined first to allow access private member of mbedtls
#include "lwip/sockets.h"
#include "lwip/tcpip.h"
#include "bt_pan.h"
#include "bt_types.h"
#include "app_pan.h"
#include "string.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy.h"
#include "mbedtls/debug.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "lwip/netdb.h"
#include "bnepif.h"
#include "app_main.h"
#include "bt_sdp.h"
#include "os_task.h"
#include "lwip/init.h"


#define F_APP_PAN_PANU_SUPPORT     1
#define F_APP_PAN_NAP_SUPPORT      0
#define F_APP_PAN_GN_SUPPORT       0


static struct
{
    bool bnepif_inited;
} pan =
{
    .bnepif_inited = false
};


#if F_APP_PAN_PANU_SUPPORT
static const uint8_t pan_panu_sdp_record[] =
{
    //total length
    SDP_DATA_ELEM_SEQ_HDR,
    0x76,//0x59,

    //Attribute SDP_ATTR_SRV_CLASS_ID_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SRV_CLASS_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_SRV_CLASS_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PANU >> 8),
    (uint8_t)(UUID_PANU),

    //attribute SDP_ATTR_PROTO_DESC_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROTO_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROTO_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x1E,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_L2CAP >> 8),
    (uint8_t)(UUID_L2CAP),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(PSM_BNEP >> 8),
    (uint8_t)(PSM_BNEP),
    SDP_DATA_ELEM_SEQ_HDR,
    0x14,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_BNEP >> 8),
    (uint8_t)(UUID_BNEP),
    SDP_UNSIGNED_TWO_BYTE,
    0x01,
    0x00,
    SDP_DATA_ELEM_SEQ_HDR,
    0x0C,
    SDP_UNSIGNED_TWO_BYTE,
    0x08,
    0x00,
    SDP_UNSIGNED_TWO_BYTE,
    0x08,
    0x06,
    SDP_UNSIGNED_TWO_BYTE,
    0x81,
    0x00,
    SDP_UNSIGNED_TWO_BYTE,
    0x86,
    0xdd,

    //attribute SDP_ATTR_BROWSE_GROUP_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_BROWSE_GROUP_LIST >> 8),
    (uint8_t)SDP_ATTR_BROWSE_GROUP_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP >> 8),
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP),

    //Attribute SDP_ATTR_LANG_BASE_ATTR_ID_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_LANG_BASE_ATTR_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_LANG_BASE_ATTR_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_LANG_ENGLISH >> 8),
    (uint8_t)(SDP_LANG_ENGLISH),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_CHARACTER_UTF8 >> 8),
    (uint8_t)(SDP_CHARACTER_UTF8),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_BASE_LANG_OFFSET >> 8),
    (uint8_t)(SDP_BASE_LANG_OFFSET),

    //attribute SDP_ATTR_PROFILE_DESC_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROFILE_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROFILE_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x08,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PANU >> 8),
    (uint8_t)(UUID_PANU),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(0x0100 >> 8),
    (uint8_t)(0x0100),

    //Attribute SDP_ATTR_SRV_NAME
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)((SDP_ATTR_SRV_NAME + SDP_BASE_LANG_OFFSET) >> 8),
    (uint8_t)(SDP_ATTR_SRV_NAME + SDP_BASE_LANG_OFFSET),
    SDP_STRING_HDR,
    0x0C,
    'R', 'e', 'a', 'l', 't', 'e', 'k', ' ', 'P', 'A', 'N', 'U',

    //Attribute SDP_ATTR_SRV_DESC
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)((SDP_ATTR_SRV_DESC + SDP_BASE_LANG_OFFSET) >> 8),
    (uint8_t)(SDP_ATTR_SRV_DESC + SDP_BASE_LANG_OFFSET),
    SDP_STRING_HDR,
    0x0C,
    'R', 'e', 'a', 'l', 't', 'e', 'k', ' ', 'P', 'A', 'N', 'U',

    //Attribute SDP_ATTR_SECURITY_DESC
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SECURITY_DESC >> 8),
    (uint8_t)SDP_ATTR_SECURITY_DESC,
    SDP_UNSIGNED_TWO_BYTE,
    0x00,
    0x00
};
#endif

#if F_APP_PAN_NAP_SUPPORT
static const uint8_t pan_nap_sdp_record[] =
{
    //total length
    SDP_DATA_ELEM_SEQ_HDR,
    0x82,//0x59,

    //Attribute SDP_ATTR_SRV_CLASS_ID_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SRV_CLASS_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_SRV_CLASS_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_NAP >> 8),
    (uint8_t)(UUID_NAP),

    //attribute SDP_ATTR_PROTO_DESC_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROTO_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROTO_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x1B,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_L2CAP >> 8),
    (uint8_t)(UUID_L2CAP),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(PSM_BNEP >> 8),
    (uint8_t)(PSM_BNEP),
    SDP_DATA_ELEM_SEQ_HDR,
    0x11,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_BNEP >> 8),
    (uint8_t)(UUID_BNEP),
    SDP_UNSIGNED_TWO_BYTE,
    0x01,
    0x00,
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UNSIGNED_TWO_BYTE,
    0x08,
    0x00,
    SDP_UNSIGNED_TWO_BYTE,
    0x08,
    0x06,
    SDP_UNSIGNED_TWO_BYTE,
    0x81,
    0x00,
    SDP_UNSIGNED_TWO_BYTE,
    0x86,
    0xdd,

    //attribute SDP_ATTR_BROWSE_GROUP_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_BROWSE_GROUP_LIST >> 8),
    (uint8_t)SDP_ATTR_BROWSE_GROUP_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP >> 8),
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP),

    //Attribute SDP_ATTR_LANG_BASE_ATTR_ID_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_LANG_BASE_ATTR_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_LANG_BASE_ATTR_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_LANG_ENGLISH >> 8),
    (uint8_t)(SDP_LANG_ENGLISH),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_CHARACTER_UTF8 >> 8),
    (uint8_t)(SDP_CHARACTER_UTF8),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_BASE_LANG_OFFSET >> 8),
    (uint8_t)(SDP_BASE_LANG_OFFSET),

    //attribute SDP_ATTR_PROFILE_DESC_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROFILE_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROFILE_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x08,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_NAP >> 8),
    (uint8_t)(UUID_NAP),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(0x0100 >> 8),
    (uint8_t)(0x0100),

    //Attribute SDP_ATTR_SRV_NAME
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)((SDP_ATTR_SRV_NAME + SDP_BASE_LANG_OFFSET) >> 8),
    (uint8_t)(SDP_ATTR_SRV_NAME + SDP_BASE_LANG_OFFSET),
    SDP_STRING_HDR,
    0x0B,
    'R', 'e', 'a', 'l', 't', 'e', 'k', ' ', 'N', 'A', 'P',

    //Attribute SDP_ATTR_SRV_DESC
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)((SDP_ATTR_SRV_DESC + SDP_BASE_LANG_OFFSET) >> 8),
    (uint8_t)(SDP_ATTR_SRV_DESC + SDP_BASE_LANG_OFFSET),
    SDP_STRING_HDR,
    0x0B,
    'R', 'e', 'a', 'l', 't', 'e', 'k', ' ', 'N', 'A', 'P',

    //Attribute SDP_ATTR_SECURITY_DESC
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SECURITY_DESC >> 8),
    (uint8_t)SDP_ATTR_SECURITY_DESC,
    SDP_UNSIGNED_TWO_BYTE,
    0x00,
    0x01,

    //Attribute SDP_ATTR_NET_ACCESS_TYPE
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_NET_ACCESS_TYPE >> 8),
    (uint8_t)SDP_ATTR_NET_ACCESS_TYPE,
    SDP_UNSIGNED_TWO_BYTE,
    0x00,
    0x05,

    //Attribute SDP_ATTR_MAX_NET_ACCESS_RATE
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_MAX_NET_ACCESS_RATE >> 8),
    (uint8_t)SDP_ATTR_MAX_NET_ACCESS_RATE,
    SDP_UNSIGNED_FOUR_BYTE,
    0x00,
    0x13,
    0x12,
    0xd0
};
#endif

#if F_APP_PAN_GN_SUPPORT
static const uint8_t pan_gn_sdp_record[] =
{
    //total length
    SDP_DATA_ELEM_SEQ_HDR,
    0x77,//0x59,

    //Attribute SDP_ATTR_SRV_CLASS_ID_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SRV_CLASS_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_SRV_CLASS_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_GN >> 8),
    (uint8_t)(UUID_GN),

    //attribute SDP_ATTR_PROTO_DESC_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROTO_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROTO_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x1B,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_L2CAP >> 8),
    (uint8_t)(UUID_L2CAP),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(PSM_BNEP >> 8),
    (uint8_t)(PSM_BNEP),
    SDP_DATA_ELEM_SEQ_HDR,
    0x11,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_BNEP >> 8),
    (uint8_t)(UUID_BNEP),
    SDP_UNSIGNED_TWO_BYTE,
    0x01,
    0x00,
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UNSIGNED_TWO_BYTE,
    0x08,
    0x00,
    SDP_UNSIGNED_TWO_BYTE,
    0x08,
    0x06,
    SDP_UNSIGNED_TWO_BYTE,
    0x81,
    0x00,
    SDP_UNSIGNED_TWO_BYTE,
    0x86,
    0xdd,

    //attribute SDP_ATTR_BROWSE_GROUP_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_BROWSE_GROUP_LIST >> 8),
    (uint8_t)SDP_ATTR_BROWSE_GROUP_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP >> 8),
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP),

    //Attribute SDP_ATTR_LANG_BASE_ATTR_ID_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_LANG_BASE_ATTR_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_LANG_BASE_ATTR_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_LANG_ENGLISH >> 8),
    (uint8_t)(SDP_LANG_ENGLISH),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_CHARACTER_UTF8 >> 8),
    (uint8_t)(SDP_CHARACTER_UTF8),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_BASE_LANG_OFFSET >> 8),
    (uint8_t)(SDP_BASE_LANG_OFFSET),

    //Attribute SDP_ATTR_SRV_AVAILABILITY
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SRV_AVAILABILITY >> 8),
    (uint8_t)SDP_ATTR_SRV_AVAILABILITY,
    SDP_UNSIGNED_ONE_BYTE,
    0xff,

    //attribute SDP_ATTR_PROFILE_DESC_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROFILE_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROFILE_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x08,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_GN >> 8),
    (uint8_t)(UUID_GN),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(0x0100 >> 8),
    (uint8_t)(0x0100),

    //Attribute SDP_ATTR_SRV_NAME
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)((SDP_ATTR_SRV_NAME + SDP_BASE_LANG_OFFSET) >> 8),
    (uint8_t)(SDP_ATTR_SRV_NAME + SDP_BASE_LANG_OFFSET),
    SDP_STRING_HDR,
    0x0A,
    'R', 'e', 'a', 'l', 't', 'e', 'k', ' ', 'G', 'N',

    //Attribute SDP_ATTR_SRV_DESC
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)((SDP_ATTR_SRV_DESC + SDP_BASE_LANG_OFFSET) >> 8),
    (uint8_t)(SDP_ATTR_SRV_DESC + SDP_BASE_LANG_OFFSET),
    SDP_STRING_HDR,
    0x0A,
    'R', 'e', 'a', 'l', 't', 'e', 'k', ' ', 'G', 'N',

    //Attribute SDP_ATTR_SECURITY_DESC
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SECURITY_DESC >> 8),
    (uint8_t)SDP_ATTR_SECURITY_DESC,
    SDP_UNSIGNED_TWO_BYTE,
    0x00,
    0x01
};
#endif





void mbedtls_tls_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    char *file_sep;
    file_sep = strrchr(file, '/');
    if (file_sep)
    {
        file = file_sep + 1;
    }
    mbedtls_printf(" %s: %d %s", TRACE_STRING(file), line, TRACE_STRING(str));
//    mbedtls_platform_printf("[mbedtls] %s:%d %s", file, line, str);
}


/*
 * Return 0 if the file descriptor is valid, an error otherwise.
 * If for_select != 0, check whether the file descriptor is within the range
 * allowed for fd_set used for the FD_xxx macros and the select() function.
 */
static int check_fd(int fd, int for_select)
{
    if (fd < 0)
    {
        return MBEDTLS_ERR_NET_INVALID_CONTEXT;
    }

#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
    !defined(EFI32)
    (void) for_select;
#else
    /* A limitation of select() is that it only works with file descriptors
     * that are strictly less than FD_SETSIZE. This is a limitation of the
     * fd_set type. Error out early, because attempting to call FD_SET on a
     * large file descriptor is a buffer overflow on typical platforms. */
    if (for_select && fd >= FD_SETSIZE)
    {
        return MBEDTLS_ERR_NET_POLL_FAILED;
    }
#endif

    return 0;
}

static int net_would_block(const mbedtls_net_context *ctx)
{
    /*
     * Never return 'WOULD BLOCK' on a non-blocking socket
     */
    int val = 0;
    (void)(val);

    if ((fcntl(ctx->MBEDTLS_PRIVATE(fd), F_GETFL, val) & O_NONBLOCK) != O_NONBLOCK)
    {
        return (0);
    }

    switch (errno)
    {
#if defined EAGAIN
    case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
    case EWOULDBLOCK:
#endif
        return (1);
    }

    return (0);
}

/*
 * Initialize a context
 */
void mbedtls_net_init(mbedtls_net_context *ctx)
{
    ctx->MBEDTLS_PRIVATE(fd) = -1;
}

/*
 * Gracefully close the connection
 */
void mbedtls_net_free(mbedtls_net_context *ctx)
{
    if (ctx->MBEDTLS_PRIVATE(fd) == -1)
    {
        return;
    }

    shutdown(ctx->MBEDTLS_PRIVATE(fd), 2);
    close(ctx->MBEDTLS_PRIVATE(fd));

    ctx->MBEDTLS_PRIVATE(fd) = -1;
}


/*
 * Read at most 'len' characters
 */
int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int fd = ((mbedtls_net_context *) ctx)->MBEDTLS_PRIVATE(fd);

    ret = check_fd(fd, 0);
    if (ret != 0)
    {
        return ret;
    }

    ret = (int) read(fd, buf, len);

    if (ret < 0)
    {
        if (net_would_block(ctx) != 0)
        {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
        !defined(EFI32)
        if (WSAGetLastError() == WSAECONNRESET)
        {
            return MBEDTLS_ERR_NET_CONN_RESET;
        }
#else
        if (errno == EPIPE || errno == ECONNRESET)
        {
            return MBEDTLS_ERR_NET_CONN_RESET;
        }

        if (errno == EINTR)
        {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
#endif

        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    return ret;
}

/*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout(void *ctx, unsigned char *buf,
                             size_t len, uint32_t timeout)
{
    DBG_DIRECT(" ------ mbedtls_net_recv_timeout ------- ");
    return 0;
}

/*
 * Write at most 'len' characters
 */
int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    int fd = ((mbedtls_net_context *) ctx)->MBEDTLS_PRIVATE(fd);

    ret = check_fd(fd, 0);
    if (ret != 0)
    {
        return ret;
    }

    ret = (int) write(fd, buf, len);

    if (ret < 0)
    {
        if (net_would_block(ctx) != 0)
        {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }

#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
        !defined(EFI32)
        if (WSAGetLastError() == WSAECONNRESET)
        {
            return MBEDTLS_ERR_NET_CONN_RESET;
        }
#else
        if (errno == EPIPE || errno == ECONNRESET)
        {
            return MBEDTLS_ERR_NET_CONN_RESET;
        }

        if (errno == EINTR)
        {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
#endif

        return MBEDTLS_ERR_NET_SEND_FAILED;
    }

    return ret;
}

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
int mbedtls_net_connect(mbedtls_net_context *ctx, const char *host, const char *port, int proto)
{
    int ret;
    struct addrinfo hints;
    struct addrinfo *list;
    struct addrinfo *current;

    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == MBEDTLS_NET_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;

    if (getaddrinfo(host, port, &hints, &list) != 0)
    {
        mbedtls_printf("mbedtls_net_connect(), MBEDTLS_ERR_NET_UNKNOWN_HOST");
        return MBEDTLS_ERR_NET_UNKNOWN_HOST;
    }

    /* Try the sockaddrs until a connection succeeds */
    ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;
    for (current = list; current != NULL; current = current->ai_next)
    {
        ctx->MBEDTLS_PRIVATE(fd) = (int) socket(current->ai_family, current->ai_socktype,
                                                current->ai_protocol);
        if (ctx->MBEDTLS_PRIVATE(fd) < 0)
        {
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }

        struct sockaddr_in *temp = (struct sockaddr_in *)current->ai_addr;
        mbedtls_printf("port %d, addr %s", (uint16_t)lwip_ntohs(temp->sin_port), inet_ntoa(temp->sin_addr));
        if (connect(ctx->MBEDTLS_PRIVATE(fd), current->ai_addr, (uint32_t)current->ai_addrlen) == 0)
        {
            ret = 0;
            break;
        }

        close(ctx->MBEDTLS_PRIVATE(fd));
        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    freeaddrinfo(list);

    return ret;

}

static void pan_cback(T_BT_PAN_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_PAN_EVENT_PARAM *param = event_buf;

    switch (event_type)
    {
    case BT_PAN_EVENT_CONN_IND:
        bt_pan_connect_cfm(app_db.factory_addr, app_cfg_nv.bud_local_addr, true);
        break;

    case BT_PAN_EVENT_SETUP_CONN_IND:
        bt_pan_setup_connection_rsp(param->pan_conn_ind.bd_addr, 0);
        break;

    case BT_PAN_EVENT_CONN_CMPL:
        {
            APP_PRINT_TRACE0("app_pan pan_cback: PAN Connected!");
            bnepif_netif_up(param->pan_conn_cmpl.bd_addr);
            bnepif_dhcp_start();
        }

        break;

    case BT_PAN_EVENT_DISCONN_CMPL:
        {
            APP_PRINT_TRACE0("app_pan pan_cback: PAN Disconnected!");
            bnepif_netif_down();

        }
        break;

    case BT_PAN_EVENT_ETHERNET_PACKET_IND:
        APP_PRINT_TRACE1("app_pan pan_cback: BT_PAN_EVENT_ETHERNET_PACKET_IND len 0x%x",
                         param->pan_ethernet_packet_ind.len);
        bnepif_low_level_input(param->pan_ethernet_packet_ind.buf, param->pan_ethernet_packet_ind.len);
        break;

    default:
        break;
    }
}

static void bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_READY:
        {
            if (pan.bnepif_inited == false)
            {
                bnepif_init(app_db.factory_addr, bt_pan_send);
                pan.bnepif_inited = true;
            }

        }
        break;
    default:
        break;
    }
}


void app_pan_init(void)
{
    int32_t ret = 0;
#if F_APP_PAN_PANU_SUPPORT
    if (bt_sdp_record_add((void *)pan_panu_sdp_record) == false)
    {
        ret = -1;
        goto fail_sdp_add;
    }
#endif

#if F_APP_PAN_NAP_SUPPORT
    if (bt_sdp_record_add((void *)pan_nap_sdp_record) == false)
    {
        ret = -1;
        goto fail_sdp_add;
    }
#endif

#if F_APP_PAN_GN_SUPPORT
    if (bt_sdp_record_add((void *)pan_gn_sdp_record) == false)
    {
        ret = -1;
        goto fail_sdp_add;
    }
#endif

    if (bt_pan_init() == false)
    {
        ret = 2;
        goto fail_init;
    }

    bt_pan_cback_register(pan_cback);
    bt_mgr_cback_register(bt_cback);
    tcpip_init(NULL, NULL);

    return;

fail_init:
fail_sdp_add:
    APP_PRINT_ERROR1("app_bt_pan_demo_init: failed %d", ret);
}


bool app_pan_connect(uint8_t *bd_addr)
{
    T_BT_SDP_UUID_DATA uuid;

    uuid.uuid_16 = UUID_NAP;

    return bt_sdp_discov_start(bd_addr, BT_SDP_UUID16, uuid);
}

bool app_pan_disconnect(uint8_t *bd_addr)
{
    return bt_pan_disconnect_req(bd_addr);
}


void app_pan_cmd(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t app_idx,
                 uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    APP_PRINT_TRACE1("app_pan_cmd: cmd_id 0x%04x", cmd_id);


    switch (cmd_id)
    {
    case CMD_PAN_CONN:
        {
            struct
            {
                uint16_t cmd_id;
                uint8_t  addr[6];
            } __attribute__((packed)) *cmd = (__typeof__(cmd))cmd_ptr;
            app_pan_connect(cmd->addr);
        }
        break;

    case CMD_PAN_DISC:
        {
            struct
            {
                uint16_t cmd_id;
                uint8_t  addr[6];
            } __attribute__((packed)) *cmd = (__typeof__(cmd))cmd_ptr;
            app_pan_disconnect(cmd->addr);
        }
        break;

    case CMD_PAN_HTTPS:
        {
            void app_bt_pan_https_client(void);
            app_bt_pan_https_client();
        }
        break;

    default:
        break;
    }
}

#define MBEDTLS_ERR_LEVEL       1
#define MBEDTLS_DEBUG_LEVEL     4

#define WEB_SERVER      "httpbin.org"
#define WEB_PORT        "443"
#define WEB_URL         "https://httpbin.org/get"
#define USER_AGENT      "lwIP/" LWIP_VERSION_STRING " MbedTLS/" MBEDTLS_VERSION_STRING


static const char GET_REQUEST[] = "GET " WEB_URL " HTTP/1.1\r\n" \
                                  "Host: " WEB_SERVER "\r\n" \
                                  "User-Agent: " USER_AGENT "\r\n" \
                                  "\r\n";



const char *pers = "https_client";

/* This is get from httpbin.org. */
const unsigned char server_cas_der[] =
{
    0x30, 0x82, 0x03, 0x41, 0x30, 0x82, 0x02, 0x29, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x13,
    0x06, 0x6c, 0x9f, 0xcf, 0x99, 0xbf, 0x8c, 0x0a, 0x39, 0xe2, 0xf0, 0x78, 0x8a, 0x43, 0xe6,
    0x96, 0x36, 0x5b, 0xca, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
    0x01, 0x0b, 0x05, 0x00, 0x30, 0x39, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06,
    0x13, 0x02, 0x55, 0x53, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x06,
    0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x03,
    0x13, 0x10, 0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20, 0x43,
    0x41, 0x20, 0x31, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x35, 0x30, 0x35, 0x32, 0x36, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x33, 0x38, 0x30, 0x31, 0x31, 0x37, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x5a, 0x30, 0x39, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
    0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0f, 0x30, 0x0d, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13,
    0x06, 0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04,
    0x03, 0x13, 0x10, 0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x20, 0x52, 0x6f, 0x6f, 0x74, 0x20,
    0x43, 0x41, 0x20, 0x31, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48,
    0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82,
    0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xb2, 0x78, 0x80, 0x71, 0xca, 0x78, 0xd5, 0xe3,
    0x71, 0xaf, 0x47, 0x80, 0x50, 0x74, 0x7d, 0x6e, 0xd8, 0xd7, 0x88, 0x76, 0xf4, 0x99, 0x68,
    0xf7, 0x58, 0x21, 0x60, 0xf9, 0x74, 0x84, 0x01, 0x2f, 0xac, 0x02, 0x2d, 0x86, 0xd3, 0xa0,
    0x43, 0x7a, 0x4e, 0xb2, 0xa4, 0xd0, 0x36, 0xba, 0x01, 0xbe, 0x8d, 0xdb, 0x48, 0xc8, 0x07,
    0x17, 0x36, 0x4c, 0xf4, 0xee, 0x88, 0x23, 0xc7, 0x3e, 0xeb, 0x37, 0xf5, 0xb5, 0x19, 0xf8,
    0x49, 0x68, 0xb0, 0xde, 0xd7, 0xb9, 0x76, 0x38, 0x1d, 0x61, 0x9e, 0xa4, 0xfe, 0x82, 0x36,
    0xa5, 0xe5, 0x4a, 0x56, 0xe4, 0x45, 0xe1, 0xf9, 0xfd, 0xb4, 0x16, 0xfa, 0x74, 0xda, 0x9c,
    0x9b, 0x35, 0x39, 0x2f, 0xfa, 0xb0, 0x20, 0x50, 0x06, 0x6c, 0x7a, 0xd0, 0x80, 0xb2, 0xa6,
    0xf9, 0xaf, 0xec, 0x47, 0x19, 0x8f, 0x50, 0x38, 0x07, 0xdc, 0xa2, 0x87, 0x39, 0x58, 0xf8,
    0xba, 0xd5, 0xa9, 0xf9, 0x48, 0x67, 0x30, 0x96, 0xee, 0x94, 0x78, 0x5e, 0x6f, 0x89, 0xa3,
    0x51, 0xc0, 0x30, 0x86, 0x66, 0xa1, 0x45, 0x66, 0xba, 0x54, 0xeb, 0xa3, 0xc3, 0x91, 0xf9,
    0x48, 0xdc, 0xff, 0xd1, 0xe8, 0x30, 0x2d, 0x7d, 0x2d, 0x74, 0x70, 0x35, 0xd7, 0x88, 0x24,
    0xf7, 0x9e, 0xc4, 0x59, 0x6e, 0xbb, 0x73, 0x87, 0x17, 0xf2, 0x32, 0x46, 0x28, 0xb8, 0x43,
    0xfa, 0xb7, 0x1d, 0xaa, 0xca, 0xb4, 0xf2, 0x9f, 0x24, 0x0e, 0x2d, 0x4b, 0xf7, 0x71, 0x5c,
    0x5e, 0x69, 0xff, 0xea, 0x95, 0x02, 0xcb, 0x38, 0x8a, 0xae, 0x50, 0x38, 0x6f, 0xdb, 0xfb,
    0x2d, 0x62, 0x1b, 0xc5, 0xc7, 0x1e, 0x54, 0xe1, 0x77, 0xe0, 0x67, 0xc8, 0x0f, 0x9c, 0x87,
    0x23, 0xd6, 0x3f, 0x40, 0x20, 0x7f, 0x20, 0x80, 0xc4, 0x80, 0x4c, 0x3e, 0x3b, 0x24, 0x26,
    0x8e, 0x04, 0xae, 0x6c, 0x9a, 0xc8, 0xaa, 0x0d, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x42,
    0x30, 0x40, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01, 0xff, 0x04, 0x05, 0x30,
    0x03, 0x01, 0x01, 0xff, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04,
    0x04, 0x03, 0x02, 0x01, 0x86, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04,
    0x14, 0x84, 0x18, 0xcc, 0x85, 0x34, 0xec, 0xbc, 0x0c, 0x94, 0x94, 0x2e, 0x08, 0x59, 0x9c,
    0xc7, 0xb2, 0x10, 0x4e, 0x0a, 0x08, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
    0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x98, 0xf2, 0x37, 0x5a,
    0x41, 0x90, 0xa1, 0x1a, 0xc5, 0x76, 0x51, 0x28, 0x20, 0x36, 0x23, 0x0e, 0xae, 0xe6, 0x28,
    0xbb, 0xaa, 0xf8, 0x94, 0xae, 0x48, 0xa4, 0x30, 0x7f, 0x1b, 0xfc, 0x24, 0x8d, 0x4b, 0xb4,
    0xc8, 0xa1, 0x97, 0xf6, 0xb6, 0xf1, 0x7a, 0x70, 0xc8, 0x53, 0x93, 0xcc, 0x08, 0x28, 0xe3,
    0x98, 0x25, 0xcf, 0x23, 0xa4, 0xf9, 0xde, 0x21, 0xd3, 0x7c, 0x85, 0x09, 0xad, 0x4e, 0x9a,
    0x75, 0x3a, 0xc2, 0x0b, 0x6a, 0x89, 0x78, 0x76, 0x44, 0x47, 0x18, 0x65, 0x6c, 0x8d, 0x41,
    0x8e, 0x3b, 0x7f, 0x9a, 0xcb, 0xf4, 0xb5, 0xa7, 0x50, 0xd7, 0x05, 0x2c, 0x37, 0xe8, 0x03,
    0x4b, 0xad, 0xe9, 0x61, 0xa0, 0x02, 0x6e, 0xf5, 0xf2, 0xf0, 0xc5, 0xb2, 0xed, 0x5b, 0xb7,
    0xdc, 0xfa, 0x94, 0x5c, 0x77, 0x9e, 0x13, 0xa5, 0x7f, 0x52, 0xad, 0x95, 0xf2, 0xf8, 0x93,
    0x3b, 0xde, 0x8b, 0x5c, 0x5b, 0xca, 0x5a, 0x52, 0x5b, 0x60, 0xaf, 0x14, 0xf7, 0x4b, 0xef,
    0xa3, 0xfb, 0x9f, 0x40, 0x95, 0x6d, 0x31, 0x54, 0xfc, 0x42, 0xd3, 0xc7, 0x46, 0x1f, 0x23,
    0xad, 0xd9, 0x0f, 0x48, 0x70, 0x9a, 0xd9, 0x75, 0x78, 0x71, 0xd1, 0x72, 0x43, 0x34, 0x75,
    0x6e, 0x57, 0x59, 0xc2, 0x02, 0x5c, 0x26, 0x60, 0x29, 0xcf, 0x23, 0x19, 0x16, 0x8e, 0x88,
    0x43, 0xa5, 0xd4, 0xe4, 0xcb, 0x08, 0xfb, 0x23, 0x11, 0x43, 0xe8, 0x43, 0x29, 0x72, 0x62,
    0xa1, 0xa9, 0x5d, 0x5e, 0x08, 0xd4, 0x90, 0xae, 0xb8, 0xd8, 0xce, 0x14, 0xc2, 0xd0, 0x55,
    0xf2, 0x86, 0xf6, 0xc4, 0x93, 0x43, 0x77, 0x66, 0x61, 0xc0, 0xb9, 0xe8, 0x41, 0xd7, 0x97,
    0x78, 0x60, 0x03, 0x6e, 0x4a, 0x72, 0xae, 0xa5, 0xd1, 0x7d, 0xba, 0x10, 0x9e, 0x86, 0x6c,
    0x1b, 0x8a, 0xb9, 0x59, 0x33, 0xf8, 0xeb, 0xc4, 0x90, 0xbe, 0xf1, 0xb9,
};
/* END FILE */

/* This is get from httpbin.org. */
#define SERVER_CA_PEM   \
    "-----BEGIN CERTIFICATE-----\r\n" \
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n" \
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n" \
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n" \
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n" \
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n" \
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n" \
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n" \
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n" \
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n" \
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n" \
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n" \
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n" \
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n" \
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n" \
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n" \
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n" \
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n" \
    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n" \
    "-----END CERTIFICATE-----\r\n"
/* END FILE */

const unsigned char server_cas_pem[] = SERVER_CA_PEM;

#if defined(MBEDTLS_KEY_EXCHANGE_SOME_PSK_ENABLED)
const unsigned char psk[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};
const char psk_id[] = "Client_identity";
#endif

static char buf[240] = {0};

static int random(void *p_rng, unsigned char *output,
                  size_t len)
{
    extern uint32_t platform_random(uint32_t max);

    uint32_t i;
    uint8_t *output_data = (uint8_t *)output;
    for (i = 0; i < len; i++)
    {
        output_data[i] = platform_random(0xFF);
    }
    return 0;
}



/**
 * @brief   Create an HTTPS client.
 * @param   None
 * @return  None
*/
void https_client(void)
{
    int ret = 0, flags = 0, len = 0;
    uint32_t written_bytes = 0;
    mbedtls_net_context server_fd;
//    struct sockaddr_in addr;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt ca_cert;

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(MBEDTLS_ERR_LEVEL);
#endif

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_net_init(&server_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&ca_cert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    mbedtls_printf("Seeding the random number generator...");

    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                (const unsigned char *) pers, strlen(pers));
    if (ret != 0)
    {
        mbedtls_printf("mbedtls_ctr_drbg_seed failed ret %d", ret);
//        goto exit;
    }

    mbedtls_printf("ok");

    mbedtls_printf("Loading the CA root certificate ...");
    if ((ret = mbedtls_x509_crt_parse_der(&ca_cert, server_cas_der, sizeof(server_cas_der))) != 0)
    {
        mbedtls_printf("mbedtls_x509_crt_parse_der Failed ! temp_ret = %d", ret);
        goto exit;
    }
    if ((ret = mbedtls_x509_crt_parse(&ca_cert, server_cas_pem, sizeof(server_cas_pem))) != 0)
    {
        mbedtls_printf("mbedtls_x509_crt_parse_der failed! returned -0x%x", (unsigned int) - ret);
        goto exit;
    }
    mbedtls_printf("ok");


    /*
     * 1. Start the connection
     */
    mbedtls_printf("Connecting to tcp/ %s / %s ...", TRACE_STRING(WEB_SERVER), TRACE_STRING(WEB_PORT));

    if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER, WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        mbedtls_printf("failed! mbedtls_net_connect returned %d", ret);
//      goto exit;
    }
    mbedtls_printf("conncet ok");

    /*
     * 2. Setup stuff
     */
    mbedtls_printf("Setting up the SSL/TLS structure...");
    if ((ret = mbedtls_ssl_config_defaults(&conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        mbedtls_printf("failed! mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }
    mbedtls_printf("ok");

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &ca_cert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_dbg(&conf, mbedtls_tls_debug, (void *)1);

#if defined(MBEDTLS_KEY_EXCHANGE_SOME_PSK_ENABLED)
    mbedtls_ssl_conf_psk(&conf, psk, sizeof(psk),
                         (const unsigned char *) psk_id, sizeof(psk_id) - 1);
#endif

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        mbedtls_printf("failed! mbedtls_ssl_setup returned %d", ret);
        goto exit;
    }

    if ((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
        mbedtls_printf("failed! mbedtls_ssl_set_hostname returned %d", ret);
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    /*
     * 3. Handshake
     */
    mbedtls_printf("Performing the SSL/TLS handshake...");
    if ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
    {
        mbedtls_printf("failed! mbedtls_ssl_handshake returned -0x%x", (unsigned int) - ret);
        goto exit;
    }
    mbedtls_printf("ok");

    /*
     * 4. Verify the server certificate
     */
    mbedtls_printf("Verifying peer X.509 certificate... ");

    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
        /* In real life, we probably want to close connection if ret != 0 */
        mbedtls_printf("Failed to verify peer certificate!");
        memset(buf, 0, sizeof(buf));
        mbedtls_x509_crt_verify_info(buf, sizeof(buf), " ! ", flags);
        mbedtls_printf("verification info: %s", buf);
    }
    else
    {
        mbedtls_printf("Certificate verification OK.");
    }
    mbedtls_printf("Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));

    /*
     * 5. Write the GET request
     */
    mbedtls_printf(" > Write to server...");

    written_bytes = 0;
    while (written_bytes < strlen(GET_REQUEST))
    {
        ret = mbedtls_ssl_write(&ssl, (const unsigned char *)GET_REQUEST + written_bytes,
                                strlen(GET_REQUEST) - written_bytes);
        if (ret >= 0)
        {
            mbedtls_printf("write %d bytes ", ret);
            written_bytes += ret;
        }
        else if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            mbedtls_printf("failed! mbedtls_ssl_write returned %d", ret);
            goto exit;
        }
    }

    /*
     * 7. Read the HTTP response
     */
    mbedtls_printf(" < Read from server:");
    len = 0;
    do
    {
        len = sizeof(buf) - 1;
        memset(buf, 0, sizeof(buf));
        ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            continue;
        }

        if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
        {
            ret = 0;
            break;
        }

        if (ret < 0)
        {
            mbedtls_printf("mbedtls_ssl_read returned -0x%x", -ret);
            break;
        }

        if (ret == 0)
        {
            mbedtls_printf("connection closed");
            break;
        }

        len = ret;
        mbedtls_printf("read %d bytes:", len);
        /* Print response directly to stdout as it is read */
        mbedtls_printf("%s", TRACE_STRING(buf));
    }
    while (1);

    mbedtls_ssl_close_notify(&ssl);

exit:
    mbedtls_net_free(&server_fd);
    mbedtls_x509_crt_free(&ca_cert);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
}

/**
 * @brief   Entry function of https demo task.
 * @param   param:
 * @return  None
*/
static void https_task_entry(void *param)
{
    mbedtls_printf("https_task start");

    https_client();

    os_task_delete(NULL);
}



/**
 * @brief  Creat http demo task.
 * @param  None
 * @return None
 */
static void *http_task_handle;
void app_bt_pan_https_client(void)
{
    mbedtls_printf("https_task_init ok");
    os_task_create(&http_task_handle, "https_task", https_task_entry, 0, 1024 * 6, 1);
}

#endif
