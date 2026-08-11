#ifndef __CWM_PORT_H__
#define __CWM_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

enum
{
    E_CALI_FAIL,//失败
    E_CALI_RUNNING,//执行中
    E_CALI_SUCCESS,//成功
};
enum
{
    E_CALI_SPV_WHOLE = 1,
    E_CALI_SPV_PCBA = 2,
    E_CALI_SIX_FACE = 5,
};

enum
{
    E_ALGO_SLEEP_DIS,
    E_ALGO_SLEEP,
    E_ALGO_NAP,
};
enum
{
    E_DEV_STATUS_CHARG_ON = 0,       //已連接充電器,
    E_DEV_STATUS_CHARG_OFF,
    E_DEV_STATUS_MOTOR_ON,           //振動馬達已啟用,
    E_DEV_STATUS_MOTOR_OFF,
    E_DEV_STATUS_TRIG_ALGO_ON,       //事件觸發算法流程
    E_DEV_STATUS_TRIG_ALGO_OFF,
    E_DEV_STATUS_SPEAKER_ON,         //揚聲器作動
    E_DEV_STATUS_SPEAKER_OFF,
    E_DEV_STATUS_LCD_SLIDE_ON,       //螢幕操作
    E_DEV_STATUS_LCD_SLIDE_OFF,
    E_DEV_STATUS_KEY_COMBO_ON,       //手錶按鈕操作
    E_DEV_STATUS_KEY_COMBO_OFF,
    E_DEV_STATUS_BT_CTL_ON,          //手機透過藍芽對手錶操作
    E_DEV_STATUS_BT_CTL_OFF,
};

struct acc_t
{
    float ax;//单位：m/s^2
    float ay;//单位：m/s^2
    float az;//单位：m/s^2
};
struct gyro_t
{
    float gx;//单位：rad/s
    float gy;//单位：rad/s
    float gz;//单位：rad/s
};
struct ag_t
{
    float ax;//单位：m/s^2
    float ay;//单位：m/s^2
    float az;//单位：m/s^2
    float gx;//单位：rad/s
    float gy;//单位：rad/s
    float gz;//单位：rad/s
};
struct mag_t
{
    float x;//单位： (uT) (=0.01Gauss)
    float y;//单位： (uT) (=0.01Gauss)
    float z;//单位： (uT) (=0.01Gauss)
};
struct baro_t
{
    float tmp;      //单位： temperature(°C)
    float pres;     //单位： (mBar) (= hPa)
};
struct dml_info
{
    uint16_t odr;//算法 odr
    uint16_t acc_rang;
    uint16_t gyro_rang;
    uint8_t has_key;
    const char *sensor_name;
    int32_t dml_version[6];
    int32_t algo_version[6];
    const char *fw_version;
};

struct algo_datetime
{
    uint8_t sec;//0~59
    uint8_t min;//0~59
    uint8_t hour;//0~23
    uint8_t day;//1~31
    uint8_t mon;//1~12
    uint16_t year; //year >= 2000,,eg:2024
};

struct algo_user_info
{
    uint8_t age;//1~99
    uint8_t gender;//0:女； 1:男
    uint16_t weiht;//10~300 (kg)
    uint16_t height;//30~300 (cm)
    uint16_t hand;//0:左； 1:右
};

struct algo_gps_data
{
    double  latitude ;// (degrees) (range: -90~90)
    double  longitude;// (degrees) (range: -180~180)
    double  altitude;// (meters)
    double  bearing ;//(heading in degrees)
    double  speed ;//(m/s)
    double  Horizontal_Accuracy ;//(meters)
    double  Vertical_Accuracy ;//(meters)
    double  HDOP;
    double  VDOP;
    double  number_of_satellites;
    double  NMEA_latitude;//(degrees)(range: -90~90)
    double  NMEA_longitude;//(degrees) (range: -180~180)
};

struct sleep_datetime
{
    uint8_t min;//0~59
    uint8_t hour;//0~23
    uint8_t day;//1~31
    uint8_t mon;//1~12
    uint16_t year; //year,eg:2024
};
struct sleep_total_t
{
    uint16_t mins_total;
    uint16_t mins_light_sleep;
    uint16_t mins_deep_sleep;
    uint16_t mins_wake_sleep;
    uint16_t mins_blink_sleep;
};

void cwm_algo_task_init(void);
void cwm_algo_task_suspend(void);
void cwm_algo_task_resume(void);
void cwm_algo_task(void *pvParameters);

/*ctr=1:enable; 0:disable*/
void cwm_log_debug_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_algo_hand_updown_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_algo_ar_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_algo_elliptical_trainer_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_algo_treadmil_ctl(uint32_t ctr);
//跑步机距离校正
void cwm_algo_treadmil_set_calibration_distance(uint32_t calibration_distance);
/*ctr=1:enable; 0:disable*/
void cwm_algo_out_running_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_algo_rope_skipping_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_algo_rowing_machine_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_algo_out_riding_ctl(uint32_t ctr);
/*返回1001模式*/
void cwm_algo_return_1001(void);
/*ctr=1:暂停; ctr=0:恢复*/
void cwm_algo_activity_pause_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_algo_free_training_ctl(uint32_t ctr);
/*ctr=1:客户设置绝对静止，ctr=0：关闭绝对静止*/
void cwm_algo_abs_static_ctl(uint32_t ctr);
/*ctr=1:enable; 0:disable*/
void cwm_swim_ctl(uint32_t ctr);
//输入泳池长度(M)
void cwm_set_swim_length(uint32_t pool_length);
/*开放水域游泳：ctr=1:enable; 0:disable  */
void cwm_open_water_swim_ctl(uint32_t ctr);
//输入開放水域分段距離
void cwm_set_open_swim_length(uint32_t open_swim_length);
/*ctr=1:enable; 0:disable*/
void cwm_algo_outdoor_running_tracking_ctl(uint32_t ctr);


/*
设置睡眠起始结束时间：
uint8_t st_hour：起始时间：小时
uint8_t st_min：起始时间：分钟
uint8_t end_hour：结束时间：小时
uint8_t end_min：结束时间：分钟
*/
void cwm_algo_set_sleep_time(uint8_t st_hour, uint8_t st_min, uint8_t end_hour, uint8_t end_min);
/*ctr=2:小睡; ctr=1:睡眠; 0:disable*/
void cwm_algo_sleep_ctl(uint32_t ctr);
/*强制睡眠输出数据，终止此段睡眠*/
void cwm_algo_force_request_sleep_data(void);

/*
uint8_t*data：指向日期结构体
    struct algo_datetime{
        uint8_t sec;//0~59
        uint8_t min;//0~59
        uint8_t hour;//0~23
        uint8_t day;//1~31
        uint8_t mon;//1~12
        uint16_t year; //year >= 2000,,eg:2024
    };
uint16_t len：日期结构体长度 = sizeof(struct algo_datetime);
*/
void cwm_set_datetime_to_algo(uint8_t *data, uint16_t len);
/*
uint8_t*data：指向用户信息结构体。
    struct algo_user_info{
        uint8_t age;//1~99
        uint8_t gender;//0:女； 1:男
        uint16_t weiht;//10~300 (kg)
        uint16_t height;//30~300 (cm)
        uint16_t hand;//0:左； 1:右
    };
uint16_t len：用户信息结构体长度 = sizeof(struct algo_user_info);
*/
void cwm_set_user_info_to_algo(uint8_t *data, uint16_t len);
/*发送设备状态到算法：主要是充电状态、马达震动等，输入参数：
tatus:
enum{
    E_DEV_STATUS_CHARG_ON = 0,       //已連接充電器,
    E_DEV_STATUS_CHARG_OFF,
    E_DEV_STATUS_MOTOR_ON,           //振動馬達已啟用,
    E_DEV_STATUS_MOTOR_OFF,
    E_DEV_STATUS_TRIG_ALGO_ON,       //事件觸發算法流程
    E_DEV_STATUS_TRIG_ALGO_OFF,
    E_DEV_STATUS_SPEAKER_ON,         //揚聲器作動
    E_DEV_STATUS_SPEAKER_OFF,
    E_DEV_STATUS_LCD_SLIDE_ON,       //螢幕操作
    E_DEV_STATUS_LCD_SLIDE_OFF,
    E_DEV_STATUS_KEY_COMBO_ON,       //手錶按鈕操作
    E_DEV_STATUS_KEY_COMBO_OFF,
    E_DEV_STATUS_BT_CTL_ON,          //手機透過藍芽對手錶操作
    E_DEV_STATUS_BT_CTL_OFF,
}
*/
void cwm_set_device_status(uint32_t status);
/*
发送给算法佩戴状态：
佩戴 status=1；
未佩戴 status=0；
*/
void cwm_set_wear_status(uint8_t status);
/*
发送给算法心率值：
佩戴时， value=心率值，实时发送。hr 范围：（(value >= 30) && (value <= 220)）
未佩戴，只需要发送一次 value=0；
*/
void cwm_set_hr_value(uint16_t value);

void cwm_set_gps_value(uint8_t *data, uint16_t len);

void cwm_reset_daily_info(void);




/*dml 读取 ag sensor 数据
在中断中调用，也可以轮询调用该接口*/
void cwm_sensor_ag_interrupt_handle(void);
/*dml 读取 mag sensor 数据
在中断中调用，也可以轮询调用该接口*/
void cwm_sensor_mag_interrupt_handle(void);
/*
acc 数据输入到算法 buf。
uint8_t* data:
    struct acc_t{
        float ax;//单位：m/s^2
        float ay;//单位：m/s^2
        float az;//单位：m/s^2
    };
*/
void cwm_acc_buf_in(uint8_t *data, uint16_t frames);
/*
gyro 数据输入到算法 buf。
uint8_t* data:
    struct gyro_t{
        float gx;//单位：rad/s
        float gy;//单位：rad/s
        float gz;//单位：rad/s
    };
*/
void cwm_gyro_buf_in(uint8_t *data, uint16_t frames);
/*
mag 数据输入到算法 buf。
uint8_t* data:
    struct mag_t{
        float x;//单位： (uT) (=0.01Gauss)
        float y;//单位： (uT) (=0.01Gauss)
        float z;//单位： (uT) (=0.01Gauss)
    };
*/
void cwm_mag_buf_in(uint8_t *data, uint16_t frames);
/*
gyro 数据输入到算法 buf。
uint8_t* data:
    struct baro_t{
        float pres;     //单位： (mBar) (= hPa)
        float tmp;      //单位： temperature(°C)
    };
需要注意的是：
    baro 数据输入是有要求的，输入频率不能低于 acc 的输入频率。
    如： acc 26hz 设置 fifo 中断模式，中断 water level 4（4 组数据触发中断），那么 acc 输入频率为 26/4=6.5hz
        baro 的输入频率就不能低于 6.5hz，因为所有 sensor 数据会根据 acc 对齐，低于这个频率会导致无法对齐。
    如：acc 26hz 设置 fifo 模式，轮询获取数据，那么 acc 输入频率为 1000/轮询时间
        baro 的输入频率就不能低于 1000/轮询时间
*/
void cwm_baro_buf_in(uint8_t *data, uint16_t frames);
#ifdef __cplusplus
}
#endif

#endif



