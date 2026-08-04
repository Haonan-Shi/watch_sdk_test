/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <string.h>
#include <stdio.h>
//#include "utils.h"
//#include "iotsec.h"
#include "trace.h"
#include "alipay_ble_transport.h"
#include "app_main.h"
#include "module_global_data.h"
#include "alipay_time.h"
#include "alipay_config.h"
#include "csi_common.h"

#if  CONFIG_ALIPAY

// uint32_t get_timestamp(void);
extern char g_mac_address[18];
// extern void ALIPAY_ble_write(uint8_t *data, uint16_t length);
extern uint32_t Unix_time;

extern T_APP_DB app_db;

uint32_t upay_get_compile_timestamp()
{
    /*The reference code is as follows. Due to the use of library functions,
    there may be compatibility issues on different platforms, and manufacturers can implement it themselves.*/
    int year = 0, month = 0, day = 0;
    int hour = 0, minute = 0, seconds = 0;
    char m[4] = {0};
    sscanf(__DATE__, "%3s %2d %4d", m, &day, &year);
    sscanf(__TIME__, "%2d:%2d:%2d", &hour, &minute, &seconds);

    time_union_t tm_time;
    tm_time.time.year = year - 2000 /*since 2000 */;
    tm_time.time.month = month; // convert_time_to_second need month start from 1
    tm_time.time.day = day;
    tm_time.time.hours = hour;
    tm_time.time.minute = minute;
    tm_time.time.seconds = seconds;

    uint32_t t = convert_time_to_second(tm_time);
    return t;
}

struct timeval
{
    long int tv_sec;
    long int tv_usec;
};

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    tv->tv_sec = 0;
    tv->tv_usec = 0;
    tz->tz_minuteswest = 0;
    tz->tz_dsttime = 0;
    return 0;
}

csi_error_t csi_get_deviceid(uint8_t *buf_did, uint32_t *len_did)
{
    AliPay_LOG("[Yuyin] csi_get_deviceid");
    uint8_t Mac[6]; //= {0x56,0x65, 0x23, 0x32, 0x23, 0x61};
    // memcpy(Mac, app_db.factory_addr, 6);
    uint8_t *pMac = (uint8_t *)ALIPAY_BT_MAC_RAM_ARRD;
    memcpy(Mac, pMac, 6); //get OTP->bt_bd_addr

    char str[18] = {0};
    // sprintf(str,"%02x:%02x:%02x:%02x:%02x:%02x",Mac[0],Mac[1],Mac[2],Mac[3],Mac[4],Mac[5]);
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", Mac[5], Mac[4], Mac[3], Mac[2], Mac[1], Mac[0]);

    *len_did = strlen(str);
    memcpy(buf_did, str, strlen(str));

    AliPay_LOG(str);

    return CSI_OK;
}

csi_error_t csi_get_timestamp(uint32_t *tm)
{
    AliPay_LOG("[Yuyin] csi_get_timestamp");
    //  uint8_t dialInfo[] = {0x01};
    // APP_FitProSend_cmd(0x1c, 0x10, dialInfo, sizeof(dialInfo));
    // *tm = get_timestamp();
    //      SYS_delay_ms(100);
    // tm = Unix_time;
    *tm = alipay_get_system_second_greenwich();
    //*tm  = 1000;//test
    AliPay_LOG("Unix_time = %d\n", *tm);
    return CSI_OK;
}

csi_error_t csi_get_compile_timestamp(uint32_t *timestamp)
{
    AliPay_LOG("[Yuyin] csi_get_compile_timestamp");
    if (timestamp == NULL)
    {
        return CSI_ERROR;
    }
    *timestamp = upay_get_compile_timestamp();
    //*timestamp = get_compile_timestamp();
    return CSI_OK;
}

csi_error_t csi_get_sn(uint8_t *buf_sn, uint32_t *len_sn)
{
    AliPay_LOG("[Yuyin] csi_get_sn");

    const char *mock_sn = "123456";
    *len_sn = strlen(mock_sn);
    memcpy(buf_sn, mock_sn, strlen(mock_sn));

    return CSI_OK;
}

csi_error_t csi_get_companyname(uint8_t *buffer, uint32_t *len)
{
    AliPay_LOG("[Yuyin] csi_get_companyname");

    const char *mock_company = "Realtek";
    *len = strlen(mock_company);
    memcpy(buffer, mock_company, strlen(mock_company));

    return CSI_OK;
}

csi_error_t csi_get_protoctype(uint32_t *ptype)
{
    AliPay_LOG("[Yuyin] csi_get_protoctype");

    *ptype = 0;
    return CSI_OK;
}

void csi_log(int level, const char *format, int32_t value)
{
    AliPay_LOG("[Yuyin] csi_log");
    // printf("[level%d %d] %s\n",level, value,format);
    AliPay_LOG("[level %d %d] %s\n", level, value, format);
    // AliPay_LOG("%s", log_buf);
}

void csi_log_ext(const char *format, va_list *val_list)
{
    //AliPay_LOG("[Yuyin] csi_log_ext");
    char log_buf[256] = {0};
    // va_list arg_list;
    // va_start(arg_list, format);
    vsnprintf(log_buf, 240, format, *val_list);

    // MyPrintf("%s", log_buf);
    AliPay_LOG("[Alipay]%s", log_buf); // ""
    // va_end(arg_list);
}

csi_error_t csi_get_productmodel(uint8_t *buffer, uint32_t *len)
{
    AliPay_LOG("[Yuyin] csi_get_productmodel");
    const char *mock_company = "1121E_1";
    *len = strlen(mock_company);
    memcpy(buffer, mock_company, strlen(mock_company));

    return CSI_OK;
}



#endif // CONFIG_ALIPAY
