#include <zephyr/device.h>
#include <time.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/drivers/i2c.h>
#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "stdarg.h"
#include "stdio.h"
#include "cwm_lib.h"
#include "cwm_customio.h"
#include "cwm_config.h"
#include "cwm_port.h"
#include "cwm_sleep_merge.h"
#include "os_sync.h"
#include "trace.h"
#include "platform_utils.h"
#include "os_mem.h"

#define GSENSOR_I2C_DEV_NODE                DT_ALIAS(i2c0)
static const struct device *gsensor_i2c_dev = DEVICE_DT_GET(GSENSOR_I2C_DEV_NODE);
static const struct device *rtc = DEVICE_DT_GET(DT_ALIAS(rtc));

#if CWM_AR_RECALL
#include "ar_recall.h"
#endif
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
/*平台相关头文件*/
#include "nrf_delay.h"
#include "twi_api.h"
#include "app_timer.h"
#include "app_datetime.h"
#include "app_ui.h"
#include "app_hr.h"
#include "fat_sd.h"
#include "app_obs.h"
#include "bci.h"
#include "bci_evt_notify.h"
#include "app_bat.h"
#include "app_baro.h"
extern void systickUpdate(void);
extern void sd_log_file_creat(uint8_t file_index, char *file_name);
extern void cwm_algo_data_module_init(void);
extern void cwm_alg_log_time_str_generate(const void *para);
#endif

/****************************************************操作系统相关需要实现的接口************************************************/
/*信号量用于对共享资源的保护，主要用于下面 4 个地方：
acc 数据；  【0】
gyro 数据； 【1】
mag 数据；  【2】
baro 数据； 【3】
消息队列；  【4】
*/
#define SEM_TABLE_MAX 5
static void *Semaphore_table[SEM_TABLE_MAX] = {0};
void cwm_task_init_CRITICAL(void)
{
    for (uint8_t i = 0; i < SEM_TABLE_MAX; i ++)
    {
        os_sem_create(&Semaphore_table[i], "cwm_sem", 0, 1);
        if (Semaphore_table[i] != NULL)
        {
            if (os_sem_give(Semaphore_table[i]) != true)
            {
                cwm_app_debug("[algo]cwm_task_init_CRITICAL error %d\n", i);
            }
        }
        else
        {
            cwm_app_debug("[algo]cwm_task_init_CRITICAL error %d\n", i);
        }
    }
}
void cwm_taskENTER_CRITICAL(uint8_t id)
{
    if ((id < SEM_TABLE_MAX) && (Semaphore_table[id] != NULL))
    {
        os_sem_take(Semaphore_table[id], 2);
    }
}
void cwm_taskEXIT_CRITICAL(uint8_t id)
{
    if ((id < SEM_TABLE_MAX) && (Semaphore_table[id] != NULL))
    {
        os_sem_give(Semaphore_table[id]);
    }
}
/****************************************************dml、algo 需要实现的接口************************************************/
int OS_algo_printstr(const char *format)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    // char *position = strstr(format, "IDX(0)");
    // if(NULL != position){
    //     return 0;
    // }

    // position = strstr(format, "IDX(1)");
    // if(NULL != position){
    //     return 0;
    // }

    extern int CWM_OS_dbgPrintf(const char *format, ...);
    CWM_OS_dbgPrintf("%s", format);
#endif
    DBG_DIRECT("[CWM]%s", format);
    return 0;
}
uint64_t OS_algo_get_time_ns(void)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    extern uint64_t CWM_OS_GetTimeNs(void);
    return CWM_OS_GetTimeNs();
#else
    uint64_t time_cnt = 0;
    struct rtc_time rtc_time_get;
    rtc_get_time(rtc, &rtc_time_get);
    time_cnt = (uint64_t)timeutil_timegm((struct tm *)(&rtc_time_get));
    time_cnt = time_cnt * 1000000000;
    time_cnt += rtc_time_get.tm_nsec;
    return time_cnt;
#endif
}
void OS_algo_usleep(uint32_t udelay)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    nrf_delay_us(udelay);
#endif
    platform_delay_us(udelay);
}
int OS_algo_i2c_read(uint16_t slaveAddr, uint16_t reg, int regLength, uint8_t *readData,
                     int readDataSize, int busIndex)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    return twi_read_reg1(slaveAddr, (uint8_t)reg, readData, readDataSize);
#else
    return i2c_write_read(gsensor_i2c_dev, slaveAddr, (uint8_t *)&reg, regLength, readData,
                          readDataSize);
#endif
}
int OS_algo_i2c_write(uint16_t slaveAddr, uint16_t reg, int regLength, uint8_t *writeData,
                      int writeDataSize, int busIndex)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    return twi_write_data1(slaveAddr, (uint8_t)reg, writeData, writeDataSize);
#else
    uint8_t *write_buf  =  os_mem_alloc(OS_MEM_TYPE_DATA, regLength + writeDataSize);
    if (write_buf == NULL)
    {
        DBG_DIRECT("OS_algo_i2c_write FAIL");
        return -1;
    }
    memcpy(write_buf, &reg, regLength);
    memcpy(write_buf + regLength, writeData, writeDataSize);
    int ret =  i2c_write(gsensor_i2c_dev, write_buf, regLength + writeDataSize, slaveAddr);
    os_mem_free(write_buf);
    return ret;
#endif
}


os_api customio_os_api =
{
    .dbgOutput = OS_algo_printstr,
    .GetTimeNs = OS_algo_get_time_ns,
    .uSleep = OS_algo_usleep,
    .i2cRead = OS_algo_i2c_read,
    .i2cWrite = OS_algo_i2c_write,
    .malloc = NULL,
    .free = NULL,
    .spiRead = NULL,
    .spiWrite = NULL,
};
/****************************************************打印接口************************************************/
int cwm_app_debug(const char *format, ...)
{
#define STR_MAX 256
    char str[STR_MAX] = {0};
    va_list    args;
    va_start(args, format);
    vsnprintf(str, STR_MAX - 1, format, args);
    va_end(args);

    OS_algo_printstr(str);
    return 0;
}
/****************************************************配置 sensor 相关参数************************************************/
#define CWM_DEFAUL_ODR     26
const uint16_t defautl_odr = CWM_DEFAUL_ODR;
const uint8_t baro_tmp_suppot = 0;/*baro sensor support temperature*/
const uint8_t sleep_merge_en = 1;/*cwm sleep merge enable*/
const uint8_t sleep_start[4] = {21, 0, 9, 0}; //[0]=sleep start hour,[1]=sleep start min,[2]=sleep end hour,[3]=sleep end min。
const uint16_t sleep_wake_timeout = 1 * 60; //max = 3*60。
uint8_t sleep_nap_state =
    0;//睡眠/小睡状态: 0:不在睡眠和小睡状态，1:睡眠，2:小睡
int dml_vendor_config[16]     = {1, 33010200, 110931822,};
const int dml_sens_cali_config[16]  = {1, 1, 1};
const int dml_hw_config[16]         = {1, 2, 0, 0, 0, 0, 6000902, 1 + 8 + 64 + 1024,
                                      };/*(ACC)+MMC5603(MAG) SCL_DML_DRV_HW_CONFIG*/
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
const int dml_ag_config[16]         = {1, 1, 1, 2, 11, 0, 25, 5,
                                       CWM_DEFAUL_ODR, 8, 1000, 0, 0,
                                      }; /*SCL_DML_DRV_AG_CONFIG    CWM8531*/
#else
const int dml_ag_config[16]         = {1, 1, 1, 2, 11, 0, 25, 0,
                                       CWM_DEFAUL_ODR, 8, 1000, 0, 0,
                                      }; /*SCL_DML_DRV_AG_CONFIG    CWM8531*/
#endif
const int dml_mag_config[16]        = {1, 1, 1, 1, 11, 0, 0, 0,
                                       50
                                      };/*SCL_DML_DRV_M_CONFIG*/
const int dml_mag_soft_iron[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; /*SCL_SENS_CALI_CTRL_MAG*/
const int dml_log_config[16] = {1, 0, 0, 9, 5, 0, 0, -1}; /*SCL_LOG*/
const int dml_log_debug_config[16] = {1, 0, 4 + 16,};
const int dml_drv_init[16] = {1, 1, 0, 2};
const int dml_sleep_config[16]      = {1, 0, 0, 5, 4, 0, 2, 4,}; /*SCL_SLEEP_CONFIG*/
const int dml_nap_config[16]        = {1, 0, 0, 5, 4, 0, 2, 4,}; /*SCL_SLEEP_CONFIG*/
const int dml_sleep_inactivity_config[16] = {1, 0,}; /*SCL_INACTIVITY_CONFIG*/
const int dml_nap_inactivity_config[16] = {1, 0,}; /*SCL_INACTIVITY_CONFIG*/
const int dml_sleep_set_inactivity_config[16] = {1, 2,}; /*SCL_SET_INACTIVITY_MODE*/
const int dml_nap_set_inactivity_config[16] = {1, 8,}; /*SCL_SET_INACTIVITY_MODE*/
const int dml_activity_config[16]   = {1, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0,
                                      };
const int dml_pedo_config[16]       = {1, 0, 0, 0, 1,}; //extra输出需要
const int dml_hand_updown_config[16] = {1, 5, 5, 5,};
const int dml_ar_config[16] = {1, 1, 0, 0, 7};
const int dml_ar_close_config[16] = {1, 2, 0, 0, 0};
const int dml_treadmill_config[16] = {1, 1002, 0, 0, 0}; //跑步机
const int dml_outdoor_running_config[16] = {1, 1003, 0, 0, 0}; //户外跑步
const int dml_elliptical_trainer_config[16] = {1, 6001, 2025219109, 0, 0}; //椭圆机
const int dml_rowing_machine_config[16] = {1, 6001, 1481115296, 0, 0}; //划船机
const int dml_rope_skip_config[16] = {1, 6001, 1241477959, 0, 0}; //跳绳
const int dml_outdoor_riding_config[16] = {1, 3001, 0, 0, 0}; //骑行
const int dml_freetraining_config[16] = {1, 1008, 0, 0, 0}; //自由训练
const int dml_auto_pause_config[16] = {1, 0, 1, 0, 0}; //开启自动暂停/恢复
const int dml_auto_pause_close_config[16] = {1, 0, 0, 0, 0}; //关闭自动暂停/恢复
const int dml_pedo_treadmill_config[16] = {1, 380, 4, 4, 1, 0, 0, 0, 0, 3, 0, 0, 60, 120, 0, 0}; //跑步机距离校正
const int dml_abs_static_config[16] = {1, 540, 40, 0, 0, 0, 0, 0}; /*绝对静止 SCL_ABS_STATIC_CONFIG*/
const int dml_swim_config[16] = {1, 2003, 0, 0, 0}; /*泳池游泳 SCL_SET_ACTIVITY_MODE*/
const int dml_swim_pool_length_config[16] = {1, 0, 1, 1, 100}; /*游泳泳池长度 SCL_SET_ACTIVITY_MODE*/
const int dml_open_water_swim_config[16] = {1, 2002, 0, 0, 0}; //开放水域游泳 SCL_SET_ACTIVITY_MODE*/
const int dml_open_water_swim_length_config[16] = {1, 0, 1, 0, 0, 0}; /*开放水域分段长度 SCL_SET_ACTIVITY_MODE*/
const int dml_outdoor_running_tracking_config[16] = {1, 1009, 0, 0, 0}; /*户外跑步轨迹 SCL_SET_ACTIVITY_MODE*/

/****************************************************密钥检查接口************************************************/
int cm__dcm__mdsdSens1_init(void);
int cm__dcm__mdsd1_deinit(void);
const unsigned char SEC_FILE_ADDR[102] =
{
    0x23, 0x20, 0x86, 0xae, 0x66, 0x00, 0x00, 0x00, 0x01, 0x00, 0x21, 0x02, 0x12, 0xe5, 0xc5, 0x71,
    0x1c, 0x21, 0x3e, 0xc3, 0xd0, 0xc3, 0xd0, 0x6d, 0x8b, 0xa5, 0x3f, 0x44, 0x0e, 0xec, 0xb5, 0xfd,
    0x0c, 0x62, 0x0a, 0xc1, 0xae, 0x7e, 0x80, 0x79, 0x24, 0x06, 0x19, 0xc1, 0xc5, 0x69, 0x17, 0x77,
    0x38, 0x9b, 0x43, 0xb0, 0xaf, 0x79, 0xfe, 0x95, 0x81, 0x45, 0x4d, 0xf9, 0x9f, 0xf0, 0xe3, 0xef,
    0xaf, 0x54, 0xb0, 0xd5, 0x7b, 0xe1, 0x70, 0x78, 0xed, 0x21, 0xf4, 0xa0, 0x52, 0xc8, 0x59, 0x51,
    0xa3, 0x1e, 0xbb, 0x93, 0x3f, 0xc5, 0x01, 0x43, 0x79, 0x77, 0x65, 0x65, 0x4d, 0x6f, 0x74, 0x69,
    0x6f, 0x6e, 0x5f, 0x6d, 0x31, 0x00,
};
void customio_listen_pre(void)
{
    cm__dcm__mdsdSens1_init();
}
void customio_listen_after(void)
{
    cm__dcm__mdsd1_deinit();
}

/****************************************************硬件需要实现的接口************************************************/
/*flash 读写需要实现的接口：有 mag 的算法需要实现下面的接口，其它算法不需要实现该接口
从 flash 中读取硬磁校正数据：
uint8_t* data: 数据结构
    struct mag_cali_hard_t{
        uint16_t crc_16;//校验和
        uint16_t valid:1;//数据可用
        int32_t x;
        int32_t y;
        int32_t z;
    };
uint32_t len：struct mag_cali_hard_t 长度
*/
void customio_read_flash_mag_hard_cali(uint8_t *data, uint32_t len)
{
    struct mag_cali_hard_t
    {
        uint16_t crc_16;//校验和
        uint16_t valid: 1; //数据可用
        int32_t x;
        int32_t y;
        int32_t z;
    };
}
/*flash 读写需要实现的接口：有 mag 的算法需要实现下面的接口，其它算法不需要实现该接口
硬磁校正数据写入 flash，客户可以在这里写 flash，也可以先备份之后在关机的时候写 flash：
uint8_t* data: 数据结构
    struct mag_cali_hard_t{
        uint16_t crc_16;//校验和
        uint16_t valid:1;//数据可用
        int32_t x;
        int32_t y;
        int32_t z;
    };
uint32_t len：struct mag_cali_hard_t 长度
*/
void customio_save_flash_mag_hard_cali(uint8_t *data, uint32_t len)
{
    struct mag_cali_hard_t
    {
        uint16_t crc_16;//校验和
        uint16_t valid: 1; //数据可用
        int32_t x;
        int32_t y;
        int32_t z;
    };
    struct mag_cali_hard_t *mag = (struct mag_cali_hard_t *)data;
    cwm_app_debug("[algo]save mag hard: %d,%d,  %d,%d,%d\n",
                  mag->crc_16, mag->valid,
                  mag->x, mag->y, mag->z);
}

/*baro 开关需要实现的接口，没有使用 baro 的手表项目，可不用实现
onoff = 1: 开启；
onoff = 0: 关闭；*/
void customio_baro_onoff(uint8_t onoff)
{
    /*此处填入客户 baro 开关接口*/
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    app_baro_power_en(onoff);
#endif
    cwm_app_debug("[algo]baro_onoff: %d\n", onoff);
}

/*gps 开关需要实现的接口，没有使用 gps 的手表项目，可不用实现
onoff = 1: 开启；
onoff = 0: 关闭；*/
void customio_gps_onoff(uint8_t onoff)
{
    /*此处填入客户 gps 开关接口*/
    cwm_app_debug("[algo]gps_onoff: %d\n", onoff);
}
/****************************************************客户接口：实现数据传输************************************************/
/*客户相关的初始化，与算法无关，在 cwm_algo_task 中调用*/
void customio_init(void)
{
    /*测试项目中用于 iic 初始化。在 cwm_algo_task.c 中调用。
    若客户不需要在此处初始化硬件，则无需在此处添加任何内容
    */
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    extern void w03_project_hw_init(void);
    w03_project_hw_init();

    extern int8_t app_baro_init(uint16_t odr_hz);
    app_baro_init(CWM_DEFAUL_ODR);
#endif
    dml_vendor_config[5] = (int)SEC_FILE_ADDR;
}
/*算法获取日期时间，在算法设置中调用，输入结构体指针 struct algo_datetime*   */
void customio_get_datetime(uint8_t *data)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    struct algo_datetime *date = (struct algo_datetime *)data;
    datetime_t tmp;
    app_datetime_get(&tmp);
    date->year = tmp.year + 2000; //注意：tmp.year 是减去 2000 的数值。
    date->mon = tmp.month;
    date->day = tmp.day;
    date->hour = tmp.hour;
    date->min = tmp.minutes;
    date->sec = tmp.seconds;
#endif
    struct algo_datetime *date = (struct algo_datetime *)data;
    struct rtc_time rtc_time_get;
    rtc_get_time(rtc, &rtc_time_get);
    date->year = rtc_time_get.tm_year + 1900;
    date->mon = rtc_time_get.tm_mon;
    date->day = rtc_time_get.tm_mday;
    date->hour = rtc_time_get.tm_hour;
    date->min = rtc_time_get.tm_min;
    date->sec = rtc_time_get.tm_sec;
}
/*sensor 数据输出：cwm dml 调用硬件接口读取 sensor 数据后，通过此接口输出 sensor 数据*/
void customio_hw_sensor_data_out(const dml_sensorData_t *pSdo)
{
#if (CWM_HANDUPDOWN_Split == 1)
    void customio_hw_sensor_data_for_handup(const dml_sensorData_t *pSdo);
    customio_hw_sensor_data_for_handup(pSdo);
#endif

    if (pSdo->validData & DML_SENSOR_DATA_VALID_DT)
    {
        cwm_app_debug("dt: %d us\n", pSdo->dt_us);
    }
    if (pSdo->validData & DML_SENSOR_DATA_VALID_ACC)
    {
        int32_t x_acc = pSdo->acc_data[0] * 1000;
        int32_t y_acc = pSdo->acc_data[1] * 1000;
        int32_t z_acc = pSdo->acc_data[2] * 1000;
        cwm_app_debug("acc: x=%d, y=%d, z=%d\n", x_acc, y_acc, z_acc);
        // cwm_app_debug("RAWacc: x=%f, y=%f, z=%f\n", pSdo->acc_org[0], pSdo->acc_org[1], pSdo->acc_org[2]);
        cwm_acc_buf_in((uint8_t *)pSdo->acc_data, 1);

#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
        cwm_acc_save(pSdo->acc_data[0], pSdo->acc_data[1], pSdo->acc_data[2]);

        /*累计计数，通知 hr 线程*/
        extern void cwm_algo_finish_notify(void);
        cwm_algo_finish_notify();
#endif
    }
    if (pSdo->validData & DML_SENSOR_DATA_VALID_GYRO)
    {
        cwm_app_debug("gyro: x=%.3f, y=%.3f, z=%.3f\n", pSdo->gyro_data[0], pSdo->gyro_data[1],
                      pSdo->gyro_data[2]);
        cwm_gyro_buf_in((uint8_t *)pSdo->gyro_data, 1);
    }

    if (pSdo->validData & DML_SENSOR_DATA_VALID_MAG)
    {
        cwm_app_debug("mag: x=%.3f, y=%.3f, z=%.3f\n", pSdo->mag_data[0], pSdo->mag_data[1],
                      pSdo->mag_data[2]);
        cwm_mag_buf_in((uint8_t *)pSdo->mag_data, 1);
    }
}
/*算法输出抬腕消息*/
void customio_notify_handup(void)
{
    cwm_app_debug("[algo]customio_notify_handup \n");
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    app_ui_lcd_on();
#endif
}

/*算法输出触发久坐消息*/
void customio_notify_sedentary(void)
{
    cwm_app_debug("[algo]customio_notify_sedentary \n");
}
/*算法输出放腕消息*/
void customio_notify_handdown(void)
{
    cwm_app_debug("[algo]customio_notify_handdown \n");
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    app_ui_lcd_off();
#endif
}
/*算法输出计步数据*/
void customio_notify_normal_steps(uint32_t steps)/*计步单位：步数*/
{
    cwm_app_debug("[algo]customio_notify_normal_steps %d\n", steps);
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    void set_steps(uint32_t step);
    set_steps(steps);
#endif
}
/*算法输出距离数据*/
void customio_notify_normal_distance(uint32_t meters)/*距离单位：meters*/
{
    cwm_app_debug("[algo]customio_notify_normal_distance %d\n", meters);
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    void set_distance(uint32_t meters);
    set_distance(meters);
#endif
}
/*算法输出日常卡路里数据：kcal：非基础卡路里，BMR_kcal：基础卡路里*/
void customio_notify_cal(float kcal, float BMR_kcal)/*卡路里单位：Kcal*/
{
    cwm_app_debug("[algo]customio_notify_cal(cal) %d,%d\n",
                  (int32_t)(kcal * 1000),
                  (int32_t)(BMR_kcal * 1000));
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    void set_cal(float kcal);
    set_cal(kcal);
#endif
}

/*算法输出AR运动种类(0: 未知(unknown), 1: 走路(walk), 2: 跑步(run), 3: 骑车(ride), 4: 划船机(rowing), 5: 椭圆机(elliptical))*/
void customio_notify_ar_sport(uint8_t ar_alert_type)
{
    cwm_app_debug("[algo]ar_alert_type %d\n", ar_alert_type);
#if CWM_AR_RECALL
    void CWM_AlertInfo_FromAR(float * f);
    float f[16];
    CWM_AlertInfo_FromAR(f);
    cwm_app_debug("[algo]type = %d, duratiobn = %d\n", (int32_t)f[1], (int32_t)f[2]);
#endif
}


/*Sens_Req output:sensor request(需求感测器)組合輸出，2:gyro,4:heart_rate,8:gnss,16:mag,32:baro,64: temp*/
void customio_notify_Sens_Reg_output(int32_t sens_reg)
{
    cwm_app_debug("[algo]Sens_Req_output %d\n", sens_reg);
}



/*算法输出跑步机数据：
f[1]= 步数(steps)，
f[2]= 距离(meters)，
f[3]= 卡路里 (Kcal),
f[5]= 步频 (steps/minute)，
f[6]= 步幅(meters)，
f[7]= 配速 (minutes/Km)，
f[14]= 是否暂停活动(0: 活动中, 1: 暂停活动, 2: 长暂停活动) */
void customio_notify_treadmill_sport(float *f)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    void set_alg_treadmill(float steps, float distance, float kcal, float stride_frequency,
                           float step_length, float pace, float pause_status);
    set_alg_treadmill(f[1], f[2], f[3], f[5], f[6], f[7], f[14]);
#endif
    if (100 == f[0])
    {
    }
    else
    {
        cwm_app_debug("[algo]treadmill_sport: treadmil_steps= %f treadmil_distance= %f treadmil_kcal= %f treadmil_stride_frequency= %f treadmil_step_length= %f treadmil_pace= %f\n",
                      f[1], f[2], f[3], f[5], f[6], f[7]);
        cwm_app_debug("[algo]pause_status=%f\n", f[14]);
    }
}

/*算法输出户外健走/跑步数据：
f[1]= 步数(steps)，
f[2]= 距离(meters)，
f[3]= 卡路里 (Kcal),
f[5]= 步频 (steps/minute)，
f[6]= 步幅(meters)，
f[7]= 配速 (minutes/Km)，
f[14]= 是否暂停活动(0: 活动中, 1: 暂停活动, 2: 长暂停活动)*/
void customio_notify_outdoor_running_sport(float *f)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    void set_outdoor_running(float steps, float distance, float kcal, float stride_frequency,
                             float step_length, float pace, float pause_status);
    set_outdoor_running(f[1], f[3], f[2], f[5], f[6], f[7], f[14]);
#endif
    if (100 == f[0])
    {
    }
    else
    {
        cwm_app_debug("[algo]outdoor_running_sport: treadmil_steps= %f treadmil_distance= %f treadmil_kcal= %f treadmil_stride_frequency= %f treadmil_step_length= %f treadmil_pace= %f\n",
                      f[1], f[2], f[3], f[5], f[6], f[7]);
        cwm_app_debug("[algo]pause_status=%f\n", f[14]);
    }
}

/*算法输出骑行数据:
（3001 == f[0]） 时：
    f[5]= 距离(meters)、
    f[6]= 速度(km/hour)、
    f[7]= 当前高度(meters)、
    f[9]= 卡路里(Kcal)、
    f[15]= 是否暂停活动(0: 活动中, 1: 暂停活动, 2: 长暂停活动)
（120 == f[0]） 时：
    f[3]= 爬升高度(meters) 、
    f[4]= 下降高度(meters) 、
    f[6]= 圈数(圈數)*/
void customio_notify_out_riding_sport(float *f)
{
    if (3001 == f[0])
    {
        cwm_app_debug("[algo]out_riding: distance=%f,speed=%f,current_elevation=%f,kcal=%f,pause_status=%f\n",
                      f[5], f[6], f[7], f[9], f[15]);
    }
    else if (120 == f[0])
    {
        cwm_app_debug("[algo]elevation_up=%f,elevation_down=%f,lap=%f\n",
                      f[3], f[4], f[6]);
    }
}

/*算法输出自由训练数据:
f[1]= 步数(steps),
f[3]= 卡路里(Kcal),
f[14]= 是否暂停活动(0: 活动中, 1: 暂停活动, 2: 长暂停活动)*/
void customio_notify_out_free_training_sport(float *f)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    void set_free_training_sport(float steps, float kcal, float pause_status);
    set_free_training_sport(f[1], f[3], f[14]);
#endif
    cwm_app_debug("[algo]free_training:steps=%f\n", f[1]);
    cwm_app_debug("[algo]free_training:kcal=%f\n", f[3]);
    cwm_app_debug("[algo]pause_status=%f\n", f[14]);
}


/*算法输出划船机:
（6001 == f[0]） 时：
    f[1]= 划桨次数、
    f[2]= 卡路里(Kcal)、
    f[4]= 划桨频率 (events/minute)、
    f[10]= 是否暂停活动(0: 活动中, 1: 暂停活动, 2: 长暂停活动)
（150 == f[0]） 时：
    f[6]= 拉桨时间(seconds)、
    f[7]= 收桨时间(seconds)、*/
void customio_notify_set_rowing_machine_sport(float *f)
{
    if (6001 == f[0])
    {
        cwm_app_debug("[algo] rowing_machine:stroke_count= %f kcal= %f stroke_frequency= %f\n",
                      f[1], f[2], f[4]);
        cwm_app_debug("[algo]pause_status=%f\n", f[10]);
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
        void set_rowing_machine_sport1(float stroke_count, float kcal, float stroke_frequency);
        set_rowing_machine_sport1(f[1], f[2], f[4]);
#endif
    }
    else if (150 == f[0])
    {
        cwm_app_debug("[algo]rowing_machine: pullTime= %f freeTime= %f\n",
                      f[6], f[7]);
    }
}




/*算法输出跳绳数据:
（6001 == f[0]） 时：
    f[1]= 跳绳次数、
    f[2]= 卡路里、
    f[10]= 是否暂停活动(0: 活动中, 1: 暂停活动, 2: 长暂停活动)
（150 == f[0]） 时：
    f[1]= 中断次数、
    f[2]= 当前连跳次数、
    f[3]= 最大连跳次数，
*/
void customio_notify_rope_skipping_sport(float *f)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    void set_rope_skipping_sport(float skipping_times, float kcal);
    set_rope_skipping_sport(f[1], f[2]);
#endif
    if (6001 == f[0])
    {
        cwm_app_debug("[algo]rope_skipping: skipping_times= %f kcal= %f\n",
                      f[1], f[2]);
        cwm_app_debug("[algo]pause_status=%f\n", f[10]);
    }
    else if (150 == f[0])
    {
        cwm_app_debug("[algo]rope_skipping: stopCounter= %f combo= %f max_combo= %f\n",
                      f[1], f[2], f[3]);
    }
}

/*椭圆机输出:
f[1]= 步数，
f[2]= 卡路里(Kcal)，
f[4]= 步频(events/minute)，
f[10]= 是否暂停活动(0: 活动中, 1: 暂停活动, 2: 长暂停活动)*/
void customio_notify_elliptical_trainer_sport(float *f)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    void set_elliptical_trainer_sport(float steps, float kcal, float stride_frequency,
                                      float pause_status);
    set_elliptical_trainer_sport(f[1], f[2], f[4], f[10]);
#endif
    if (150 == f[0])
    {
    }
    else
    {
        cwm_app_debug("[algo]ell_steps= %f ell_kcal= %f ell_stride_frequency= %f \n",
                      f[1], f[2], f[4]);
        cwm_app_debug("[algo]pause_status=%f\n", f[10]);
    }
}

/*算法输出触发绝对静止消息*/
void customio_notify_abs_static(void)
{
    cwm_app_debug("[algo]customio_notify_abs_static \n");
}

/* 算法输出GPS轨迹优化数据:
（1009 == f[0]）时：
算法输出户外健走/跑步数据：
f[1]= 步数(steps)，
f[2]= 距离(meters)，
f[3]= 卡路里 (Kcal),
f[5]= 步频 (steps/minute)，
f[6]= 步幅(meters)，
f[7]= 配速 (minutes/Km)，
f[14]= 是否暂停活动(0: 活动中, 1: 暂停活动, 2: 长暂停活动)
(100 == f[0]) 时：
f[1] = average_step_frequency (steps/minute)(平均步頻)
f[2] = average_step_length (meters)(步長)(平均步幅)
f[3] = average_pace (minutes/Km)(平均配速)
（1 == f[0]）时：
f[1]= latitude: part1 (degrees)
f[2]= latitude: part2 (degrees),   (latitude (double precision) = [1] + [2])
f[3]= longitude: part1 (degrees)
f[4]= longitude: part2 (degrees),  (longitude (double precision) = [3] + [4])
f[5]= distance (meters) (由GPS運算出來的累積距離)
f[6]= pace (minutes/Km) (由GPS運算出來的配速)
f[7]= smoothed distance (meters) (由GPS運算出來的累積平滑距離)*/
void customio_notify_out_run_tracking(float *f)
{
    if (1009 == f[0])
    {
        cwm_app_debug("[algo]out_run_tracking: steps= %f distance= %f kcal= %f step_frequency= %f step_length= %f pace= %f\n",
                      f[1], f[2], f[3], f[5], f[6], f[7]);
        cwm_app_debug("[algo]pause_status=%f\n", f[14]);
    }
    else if (100 == f[0])
    {
        cwm_app_debug("[algo]average_step_frequency =%f average_step_length=%f average_pace=%f\n", f[1],
                      f[2], f[3]);
    }
    else
    {
        cwm_app_debug("[algo]out_run_tracking: latitude1=%f latitude2=%f longitude1=%f longitude2=%f distance=%f pace=%f smoothed_distance=%f \n",
                      f[1], f[2], f[3], f[4], f[5], f[6], f[7]);
    }
}

/*算法输出游泳数据：
 (2004 == f[0]) 时：
    f[1]=  總划水次數,
    f[5]= 即時划水頻率(strokes/min),
    f[6] = 總卡路里(Kcal)
    f[10] = pause_status (是否暫停活動)
    (0: 活動中, 1: 暫停活動, 2: 長暫停活動)
 (110 == f[0]) 时：
    f[5]= 此趟平均配速(minutes/km),
    f[6]= 此趟游泳效率swolf,
 (111 == f[0]) 时：
    f[3]= 總趟數
    f[4] = 總游泳時間(seconds)
    f[11]= 所有趟數的主要泳姿  ( 0: unknown(未知), 1: free style(自由式) , 2: Breast stroke(蛙式) , 3: Back stroke(仰式), 4: Butterfly stroke(蝶式), 5: Medley(混合泳))*/
void customio_notify_swim_data(float *f)
{
    if (dml_swim_config[1] == f[0])
    {
        cwm_app_debug("[algo]swim_data: total_strokes=%f stroke_frequency=%f kcal=%f\n",
                      f[1], f[5], f[6]);
        cwm_app_debug("[algo]pause_status=%f\n", f[10]);
    }
    else if (110 == f[0])
    {
        cwm_app_debug("[algo]swim_data:  average_pace=%f swim_time_swolf=%f\n  ",
                      f[5], f[6]);
    }
    else if (111 == f[0])
    {
        cwm_app_debug("[algo]swim_data: total_laps=%f total_time=%f all_laps_main_swim_type=%f \n  ",
                      f[3], f[4], f[11]);
    }
}

/*算法输出个人心率区间数据
mhr_zone_1是算法累積心率表現未達【暖身】門檻的分鐘數。
mhr_zone_2對應到【暖身】的分鐘數
mhr_zone_3對應到【燃脂】的分鐘數
mhr_zone_4對應到【有氧耐力】的分鐘數
mhr_zone_5對應到【無氧耐力】的分鐘數
mhr_zone_6對應到【極限】的分鐘數
(此分鐘數算法依無條件捨去不滿1分鐘的秒數，留整數位輸出)
（专项运动结束时输出）
*/
void customio_notify_zone_time(float mhr_zone_1, float mhr_zone_2, float mhr_zone_3,
                               float mhr_zone_4, float mhr_zone_5, float mhr_zone_6)
{
    cwm_app_debug("zone_1 : %d\n", mhr_zone_1);
    cwm_app_debug("zone_2 : %d\n", mhr_zone_2);
    cwm_app_debug("zone_3 : %d\n", mhr_zone_3);
    cwm_app_debug("zone_4 : %d\n", mhr_zone_4);
    cwm_app_debug("zone_5 : %d\n", mhr_zone_5);
    cwm_app_debug("zone_6 : %d\n", mhr_zone_6);
}

/*算法输出户外游泳数据：
 (dml_open_water_swim_config == f[0]) 时：
    f[1]=  總划水次數,
    f[5]= 即時划水頻率(strokes/min),
    f[6] = 總卡路里(Kcal)
    f[10] = pause_status (是否暫停活動)
    (0: 活動中, 1: 暫停活動, 2: 長暫停活動)
 (114 == f[0]) 时：
    f[1] = reserved(保留位)
 (115 == f[0]) 时：
    f[4] = 總游泳時間(seconds)
    f[11]= 所有趟數的主要泳姿  ( 0: unknown(未知), 1: free style(自由式) , 2: Breast stroke(蛙式) , 3: Back stroke(仰式), 4: Butterfly stroke(蝶式), 5: Medley(混合泳))*/
void customio_notify_open_swim_data(float *f)
{
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    extern void sport_set_swim_data(const float s[16]);
    sport_set_swim_data(f);
#endif
    if (dml_open_water_swim_config[1] == f[0])
    {
        cwm_app_debug("[algo]swim_data: total_strokes=%f stroke_frequency=%f kcal=%f\n",
                      f[1], f[5], f[6]);
        cwm_app_debug("[algo]pause_status=%f\n", f[10]);
    }
    else if (114 == f[0])
    {

    }
    else if (115 == f[0])
    {
        cwm_app_debug("[algo]swim_data:  total_time=%f all_laps_main_swim_type=%f \n  ",
                      f[4], f[11]);
    }
}




static void check_if_switch_to_sleep(uint8_t cur_id)
{
    struct algo_datetime date;
    customio_get_datetime((uint8_t *)&date);

    uint8_t tmp_sleep_start[4];
    uint16_t wake_time;
    sleep_merge_get_config(&tmp_sleep_start[0], &tmp_sleep_start[1], &tmp_sleep_start[2],
                           &tmp_sleep_start[3], &wake_time);

    uint32_t cur_mins = date.hour * 60 + date.min;
    uint32_t start_mins;
    uint32_t end_mins;
    uint8_t switch_to_id;

    if (1 == cur_id) //睡眠
    {
        start_mins = tmp_sleep_start[0] * 60 + tmp_sleep_start[1];
        end_mins = tmp_sleep_start[2] * 60 + tmp_sleep_start[3];
        switch_to_id = 2;
    }
    else if (2 == cur_id) //小睡
    {
        start_mins = tmp_sleep_start[2] * 60 + tmp_sleep_start[3];
        end_mins = tmp_sleep_start[0] * 60 + tmp_sleep_start[1];
        switch_to_id = 1;

        if (start_mins == end_mins)
        {
            /*24 小时睡眠，必须切换成睡眠*/
            cwm_algo_sleep_ctl(switch_to_id);
            return;
        }
    }

    if (start_mins == end_mins)
    {
        /* 24 小时睡眠，不切换*/
    }
    else if (start_mins < end_mins)
    {
        /*时间段在同一天*/
        if ((cur_mins >= start_mins) && (cur_mins < end_mins))
        {
            /*在时间段内，不切换*/
        }
        else
        {
            /*不在时间段内，切换*/
            cwm_algo_sleep_ctl(switch_to_id);
        }
    }
    else
    {
        /*时间段跨天*/
        if ((cur_mins >= start_mins) || (cur_mins < end_mins))
        {
            /*在时间段内，不切换*/
        }
        else
        {
            /*不在时间段内，切换*/
            cwm_algo_sleep_ctl(switch_to_id);
        }
    }
}


void customio_notify_sleep_status(uint8_t data_type, uint8_t id)
{
    /*通知当前睡眠/小睡状态
    data_type: 21:睡眠数据，22:小睡数据
    id：4:結束睡眠, 6: 入睡通知, 13: 取消入睡通知, 14:結束小睡）
    */
    if (21 == data_type)
    {
        switch (id)
        {
        case 6:
            cwm_app_debug("[algo]enter sleep\n");
            sleep_nap_state = 1;
            break;
        case 13:
            cwm_app_debug("[algo]Cancel sleep\n");
            sleep_nap_state = 0;
            /*取消睡眠、结束睡眠时，判断是否切换*/
            check_if_switch_to_sleep(1);
            break;
        case 4:
            cwm_app_debug("[algo]exit sleep\n");
            sleep_nap_state = 0;
            /*取消睡眠、结束睡眠时，判断是否切换*/
            check_if_switch_to_sleep(1);
            break;
        }
    }
    else if (22 == data_type)
    {
        switch (id)
        {
        case 6:
            cwm_app_debug("[algo]enter nap\n");
            sleep_nap_state = 2;
            break;
        case 13:
            cwm_app_debug("[algo]Cancel nap\n");
            sleep_nap_state = 0;
            /*取消睡眠、结束睡眠时，判断是否切换*/
            check_if_switch_to_sleep(2);
            break;
        case 14:
            cwm_app_debug("[algo]exit nap\n");
            sleep_nap_state = 0;
            /*取消睡眠、结束睡眠时，判断是否切换*/
            check_if_switch_to_sleep(2);
            break;
        }
    }
}
void customio_notify_sleep_data(uint16_t idx, uint8_t data_type, uint8_t id,
                                uint8_t *data) /*睡眠数据*/
{
    /*算法输出睡眠数据
    idx:输出数据序列号，0：表示第一组数据，之后依次递增
    data_type: 31/32：表示睡眠总数据，在睡眠数据最后发送。其它值：睡眠数据。
    data_type = 31/32: 表示睡眠总数据。
        id：忽略。
        data：
            struct sleep_total_t{
                uint16_t mins_total;
                uint16_t mins_light_sleep;
                uint16_t mins_deep_sleep;
                uint16_t mins_wake_sleep;
                uint16_t mins_blink_sleep;
            };
    data_type = 其它值: 表示睡眠数据。
        id：睡眠数据的 id：
            0:進入睡眠, 1:淺層睡眠, 2:深度睡眠, 3:清醒, 4:結束睡眠,  12: 快速動眼, 14:結束小睡）
        data：
            struct sleep_datetime{
                uint8_t min;//0~59
                uint8_t hour;//0~23
                uint8_t day;//1~31
                uint8_t mon;//1~12
                uint16_t year; //year,eg:2024
            };
    */
    if ((31 == data_type) || (32 == data_type))
    {
        struct sleep_total_t *total = (struct sleep_total_t *)data;
        cwm_app_debug("[algo]customio_notify_sleep_data total(%d, %d,%d,%d,%d,%d) \n", idx,
                      total->mins_total, total->mins_light_sleep, total->mins_deep_sleep, total->mins_wake_sleep,
                      total->mins_blink_sleep);
    }
    else
    {
        struct sleep_datetime *tmp = (struct sleep_datetime *)data;
        cwm_app_debug("[algo]customio_notify_sleep_data(%d, %d,%d,%d,%d,%d,%d) \n", idx, id,
                      tmp->year, tmp->mon, tmp->day, tmp->hour, tmp->min);
    }
}




