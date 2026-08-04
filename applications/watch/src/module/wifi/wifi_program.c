/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "string.h"
#include "trace.h"
#include "rtl876x_pinmux.h"
#include "platform_utils.h"
#include "bt_types.h"
#include "wifi_uart.h"
#include "os_sched.h"
#include "os_mem.h"
#include "wifi_program.h"
#include "patch_header_check.h"
#include "fmc_api.h"
#include "flash_map.h"
// A13 A0 PULL HIGH

#define DUT_POWER_PIN_CTRL      P2_7// HIGH POWER ON
#define Z2_DOWNLOAD_CTRL0       P2_6 // z2 A0 pull high enter download mode
#define Z2_DOWNLOAD_CTRL1       P2_5 // z2 A13 pull high enter download mode

#define PROGRAM_BUF_SIZE (1024*16)
#define SIZE_PER_PACKET      1024
#define MIN(a, b) ((a)<(b)?(a):(b))

#define STX_XMODEM  0x02
#define EOT_XMODEM  0x04
#define ACK_XMODEM  0x06
#define NAK_XMODEM  0x15


char ping[] = "ping\n";
char EW_string[] = "EW 40002800 7EFFFFF\n";
char ucfg_str[] = "ucfg 115200 0 0\n";

char ucfg1m_str[] = "ucfg 1000000 0 0\n";
char floader_str[] = "floader\r\n";
char eraser_str[] = "ceras 0 0 \n";// waiting 7s for eraser down  need send floader_str before
char fwd_str[] = "fwd 0 1 \n";
int uart_dl_cfg(uint32_t baud);
int ping_z2(void);
bool download_start(void);

static uint8_t *cmdbuf = NULL;


static void app_z2_pin_ctrl_enter_download(void)
{
//A1 A13 PULL HIGH
    Pad_Config(Z2_DOWNLOAD_CTRL0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(Z2_DOWNLOAD_CTRL1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    Pad_Config(DUT_POWER_PIN_CTRL, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
    platform_delay_ms(500);
    Pad_Config(DUT_POWER_PIN_CTRL, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);

    platform_delay_ms(1000);
}


bool app_z2_download(void)
{
    int ret;
    app_z2_pin_ctrl_enter_download();
    ret = uart_dl_cfg(115200);
    if (ret)
    {
        APP_PRINT_INFO0("uart_dl_cfg failed\n");
        goto down_fail;
    }
    ret = ping_z2();
    if (ret)
    {
        APP_PRINT_INFO0("ping_z2 failed\n");
        goto down_fail;
    }
    download_start();
    mp_close();
    return true;
down_fail:
    mp_close();
    return false;
}

// uart init  baudrate set as 115200 default
int uart_dl_cfg(uint32_t baud)
{
    int ret = mp_open();
    if (ret)
    {
        APP_PRINT_INFO1("mp_open failed ret = %d !", ret);
        return ret;
    }

    ret = mp_set_baudrate(baud);
    if (ret)
    {
        APP_PRINT_INFO1("mp_set_baudrate failed ret = %d!", ret);
        return ret;
    }

    return ret;
}

// send ping and 'EW 40002800 7EFFFFF\n'
int ping_z2(void)
{
    int ret = 0;
    uint8_t rev[50] = {0};
    uint8_t recv_len = 0;
    mp_send((uint8_t *)ping, sizeof(ping));
    recv_len = mp_recv(rev, sizeof(rev));
    // return 0
    if (strcmp((char *)rev, "ping") != 0)
    {
        APP_PRINT_WARN2("ping fail recv_len =%d  %s !\n", recv_len, TRACE_STRING(rev));
        ret = -1;
        goto ping_fail;
    }
    else
    {
        APP_PRINT_WARN2(" after send ping recv_len =%d string is %s !\n", recv_len, TRACE_STRING(rev));
        mp_send((uint8_t *)EW_string, sizeof(EW_string));
        recv_len = mp_recv(rev, sizeof(rev));
        APP_PRINT_WARN2(" after send EW_string recv_len =%d string is %s !\n", recv_len, TRACE_STRING(rev));
        //mp_send((uint8_t *)ucfg_str, sizeof(ucfg_str));

        mp_send((uint8_t *)ucfg1m_str, sizeof(ucfg1m_str) - 1);
        os_delay(400);

        mp_set_baudrate(1000000);

        //mp_send((uint8_t *)ping, sizeof(ping));
        recv_len = mp_recv(rev, sizeof(rev));
        APP_PRINT_WARN2("after config z2 baud_rate recv_len =%d string is %s !\n", recv_len,
                        TRACE_STRING(rev));
        if (strcmp((char *)rev, "OK") != 0)
        {
            APP_PRINT_WARN2("ping fail recv_len =%d  %s !\n", recv_len, TRACE_STRING(rev));
            ret = -2;
            goto ping_fail;
        }
        else
        {
            os_delay(500);
        }

        mp_send((uint8_t *)floader_str, sizeof(floader_str) - 1);
        os_delay(50);
        mp_send((uint8_t *)floader_str, sizeof(floader_str) - 1);
        os_delay(80);
        mp_send((uint8_t *)fwd_str, sizeof(fwd_str) - 1);
        os_delay(800);
        recv_len = mp_recv(rev, sizeof(rev));
        APP_PRINT_WARN2("after fwd_str recv_len =%d  0x%x !\n", recv_len, rev[0]);
        os_delay(80);
        if (rev[0] != NAK_XMODEM)
        {
            APP_PRINT_WARN2("NAK_XMODEM fail recv_len =%d  0x%x !\n", recv_len, rev[0]);
            ret = -3;
            goto ping_fail;
        }
    }
ping_fail:
    return ret;
}

void download_end(void)
{
    uint8_t recv_len;
    uint8_t rev[10] = {0};
    uint8_t cmd_end = EOT_XMODEM;

    mp_send(&cmd_end, 1);
    recv_len = mp_recv(rev, sizeof(rev));
    APP_PRINT_WARN2("download_end after send cmd_end  recv_len =%d  %s !\n", recv_len,
                    TRACE_STRING(rev));
    mp_send((uint8_t *)ucfg_str, sizeof(ucfg_str) - 1);
    os_delay(400);
    mp_set_baudrate(115200);
    recv_len = mp_recv(rev, sizeof(rev));
    APP_PRINT_WARN2("download_end after send config ucfg_str recv_len =%d  %s !\n", recv_len,
                    TRACE_STRING(rev));
}

static uint8_t checksum(unsigned char *data, size_t length)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++)
    {
        sum += data[i];
    }
    return sum;
}

static void packet_assembly(uint8_t *pcmdbuf, uint32_t *cmdlen, uint32_t len, uint8_t *data)
{
    uint8_t check_sum;
    static uint8_t sqn = 0;
    uint16_t i = 0;
    sqn++;
    if (sqn == 0)
    {
        //sqn++;
    }
    pcmdbuf[0]  = STX_XMODEM;                       /* start byte:  1 byte    */
    LE_UINT8_TO_ARRAY(&pcmdbuf[1], sqn);            /* sqn: 1 bytes   */
    LE_UINT8_TO_ARRAY(&pcmdbuf[2], (0xff - sqn));   /* sqn:        1 bytes   */
    if (len == SIZE_PER_PACKET)
    {
        memcpy(pcmdbuf + 3, data, len);             /* data:        len bytes */
    }
    else
    {
        memcpy(pcmdbuf + 3, data, len);                /* data:        len bytes */
        for (i = 0; (i + len) < SIZE_PER_PACKET; i++)
        {
            LE_UINT8_TO_ARRAY(&pcmdbuf[len + 3 + i], 0x1A);  /* crc16:       2 byters  */
        }
    }
    check_sum = checksum(&pcmdbuf[3], SIZE_PER_PACKET);
    LE_UINT8_TO_ARRAY(&pcmdbuf[len + 3 + i], check_sum); /* crc16:       2 byters  */
    APP_PRINT_INFO3("check_sum = 0x%x, pcmdbuf[len + 3] = 0x%x len= 0x%x", check_sum, pcmdbuf[len + 3],
                    len);
    *cmdlen = 4 + len;
}

static WIFI_PROGRAM_STATUS wifi_program_func(uint32_t len, uint8_t *buf)
{
    WIFI_PROGRAM_STATUS res = STATUS_OK;
    uint32_t i = 0;
    uint32_t cmd_len;
    int bytes_recv = 0;
    uint8_t recv_data = 0;
    APP_PRINT_INFO1("wifi_program_func len = 0x%x", len);
    for (i = 0; i < len / SIZE_PER_PACKET; i++)
    {
        packet_assembly(cmdbuf, &cmd_len, SIZE_PER_PACKET, (buf + i * SIZE_PER_PACKET));
_send_data1:
        mp_send(cmdbuf, SIZE_PER_PACKET + 4);
        bytes_recv = mp_recv(&recv_data, 1); /* 1 bytes crc16 */

        if (bytes_recv == 1)
        {
            if (ACK_XMODEM == recv_data)
            {
                res = STATUS_OK;
            }
            else if (NAK_XMODEM == recv_data)
            {
                APP_PRINT_WARN0("wifi_program_func respond NAK");
                os_delay(10);
                goto _send_data1;
            }
            else
            {
                res = STATUS_PROGRAM_FAIL;
                APP_PRINT_ERROR1("wifi_program_func:%d\n", res);
                goto _end;
            }
        }
        else
        {
            res = STATUS_RECV_TIMEOUT;
            APP_PRINT_ERROR2("wifi_program_func:%d,\
                              recv timeout! read %d bytes\n", \
                             res,  bytes_recv);
        }
    }

    // send remain
    if (len % SIZE_PER_PACKET)
    {
        packet_assembly(cmdbuf, &cmd_len, (len % SIZE_PER_PACKET), (buf + i * SIZE_PER_PACKET));
        //printf("send write command!\n");
_send_data2:
        mp_send(cmdbuf, (SIZE_PER_PACKET + 4));
        bytes_recv = mp_recv(&recv_data, 1); /* 2 bytes crc16 */
        if (bytes_recv == 1)
        {
            if (ACK_XMODEM == recv_data)
            {
                res = STATUS_OK;
            }
            else if (NAK_XMODEM == recv_data)
            {
                APP_PRINT_WARN0("wifi_program_func 2 respond NAK");
                goto _send_data2;
            }
            else
            {
                res = STATUS_PROGRAM_FAIL;
                APP_PRINT_ERROR1("wifi_program_func 2:%d\n", res);
                goto _end;
            }
            download_end();
        }
        else
        {
            res = STATUS_RECV_TIMEOUT;
            APP_PRINT_ERROR2("wifi_program_func 2:%d,\
                              recv timeout! read %d bytes\n", \
                             res, bytes_recv);
        }
    }
_end:
    return res;
}

static bool wifi_program(uint32_t len, uint8_t *buf)
{
    WIFI_PROGRAM_STATUS res = wifi_program_func(len, buf);
    if (res)
    {
        APP_PRINT_ERROR0("wifi_program_func error");
        return false;
    }
    return true;
}

static bool wifi_program_verify_function(uint32_t program_size, uint8_t *buf)
{
    bool ret = true;
    if (wifi_program(program_size, buf) != true)
    {
        ret = false;
        goto _end;
    }
    else
    {
        APP_PRINT_INFO1("program size 0x%08X success!\r\n", program_size);
    }
_end:
    return ret;
}

bool download_start(void)
{
    bool ret = true;
    uint32_t btr = 0;
    uint32_t residual_bytes;
    uint32_t wr_addr;
    uint32_t flash_read_addr;
    uint32_t flash_read_addr_start;
    T_IMG_HEADER_FORMAT *addr;
    addr = (T_IMG_HEADER_FORMAT *)USER_DATA2_ADDR;
    residual_bytes = addr->ctrl_header.payload_len;

    APP_PRINT_INFO1("download_start residual_bytes =%d!\r\n", residual_bytes);

    uint8_t *p_buf = os_mem_alloc(OS_MEM_TYPE_DATA, PROGRAM_BUF_SIZE);
    cmdbuf = os_mem_alloc(OS_MEM_TYPE_DATA, SIZE_PER_PACKET + 4);

    if (p_buf == NULL || cmdbuf == NULL)
    {
        APP_PRINT_WARN0("os_mem_alloc failed!\r\n");
        ret = false;
        goto _end;
    }

    flash_read_addr_start = USER_DATA2_ADDR + 0x400;
    flash_read_addr = flash_read_addr_start;
    for (; residual_bytes > 0; residual_bytes -= btr, wr_addr += btr)

    {
        flash_read_addr += btr;
        btr = MIN(PROGRAM_BUF_SIZE, residual_bytes);

        if (fmc_flash_nor_read(flash_read_addr, p_buf,  btr) == false)
        {
            ret = false;
            goto _end;
        }
        APP_PRINT_INFO5("f_read flash_read_addr = 0x%x  0x%x 0x%x 0x%x 0x%x !\r\n", flash_read_addr,
                        p_buf[0], p_buf[1], p_buf[2], p_buf[3]);
        APP_PRINT_INFO3("f_read wr_addr =0x%x, br =0x%x p_buf =0x%x !\r\n", wr_addr, btr, p_buf);
        ret = wifi_program_verify_function(btr, p_buf);
    }

_end:
    if (p_buf)
    {
        os_mem_free(p_buf);
        p_buf = NULL;
    }
    if (cmdbuf)
    {
        os_mem_free(cmdbuf);
        cmdbuf = NULL;
    }
    return ret;
}

