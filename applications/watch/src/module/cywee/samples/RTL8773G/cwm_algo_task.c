#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include <stdio.h>
#include "cwm_lib.h"
#include "cwm_lib_dml.h"
#include "cwm_config.h"
#include "cwm_customio.h"
#include "cwm_common.h"
#include "cwm_port.h"
#include "cwm_test.h"
#include "cwm_sleep_merge.h"
#include <zephyr/kernel.h>
#include "os_task.h"
#include "app_module_init.h"

#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
/* Platform related */
#include "fat_sd.h"
extern void systickUpdate(void);
#endif

#define MESSAGE_SAMPLE_CODE 0

#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
#define ALGO_TASK_PRIORITY  (configMAX_PRIORITIES - 3)
#else
#define ALGO_TASK_PRIORITY  1
#endif

#define ALGO_STACK_SIZE (8*1024/4)
/****************************************************************************************************/

enum
{
    E_ALGO_MSG_LOG_DEBUG_CTL,
    E_ALGO_MSG_HAND_UPDOWN_CTL,
    E_ALGO_MSG_SLEEP_SET_TIME,
    E_ALGO_MSG_SLEEP_CTL,
    E_ALGO_MSG_FORCE_REQ_SLEEP_DATA,

    // E_ALGO_MSG_SPV_CALI_EN,
    // E_ALGO_MSG_SPV_CALI_DIS,
    E_ALGO_MSG_SET_DATETIME,
    E_ALGO_MSG_SET_USER_INFO,
    E_ALGO_MSG_SET_DEV_STATUS,
    E_ALGO_MSG_SET_WEAR_STATUS,
    E_ALGO_MSG_SET_HR_VALUE,

    E_ALGO_MSG_RESET_DAILY_INFO,
    E_ALGO_MSG_AR_CTL,
    E_ALGO_MSG_ELLIPTICAL_TRAINER_CTL,
    E_ALGO_MSG_TREADMIL_CTL,
    E_ALGO_MSG_TREADMIL_CALIBRATION,
    E_ALGO_MSG_OUT_RUNNING_CTL,
    E_ALGO_MSG_ROPE_SKIPPING_CTL,
    E_ALGO_MSG_ROWING_MACHINE_CTL,
    E_ALGO_MSG_OUT_RIDING_CTL,
    E_ALGO_MSG_RETURN_1001,
    E_ALGO_MSG_ACTIVITY_PAUSE_CTL,
    E_ALGO_MSG_FREE_TRAINING_CTL,
    E_ALGO_MSG_ABS_STATIC_CTL,
    E_ALGO_MSG_SWIM_CTL,
    E_ALGO_MSG_POOL_LENGTH,
    E_ALGO_MSG_OPEN_WATER_SWIM_CTL,
    E_ALGO_MSG_OPEN_SWIM_LENGTH,
    E_ALGO_MSG_SET_GPS_VALUE,
    E_ALGO_MSG_OUT_RUNNING_TRACKING_CTL,

#if defined(MESSAGE_SAMPLE_CODE) && (1 == MESSAGE_SAMPLE_CODE)
    //DML INTERFACE
    E_DML_ENABLE = 0xF0,
    E_DML_DISABLE,
    E_DML_SETING_CONTROL,
#endif
};

/****************************************************************************************************/
static void *cwm_algo_task_handle;
static bool algo_running = true;
static uint32_t period_ms;/* Algorithm running period */
/****************************************************task************************************************/
uint32_t cwm_get_task_period(void)
{
    return period_ms;
}
void cwm_algo_task_init(void)
{
    /* Message queue initialization */
    algo_message_init();
    cwm_task_init_CRITICAL();
    os_task_create(&cwm_algo_task_handle, "cwm_algo_task", cwm_algo_task, 0, ALGO_STACK_SIZE,
                   ALGO_TASK_PRIORITY);
}

static void cywee_motion_module_init(void)
{
    cwm_algo_task_init();
}
APP_MODULE_INIT(cywee_motion_module_init);

void cwm_algo_task_suspend(void)
{
    os_task_suspend(cwm_algo_task_handle);
}
void cwm_algo_task_resume(void)
{
    os_task_resume(cwm_algo_task_handle);
}
void algo_message_handle(uint32_t id, uint8_t *data)
{
    cwm_app_debug("[algo]algo_message_handle %u %u\n", id, *((uint32_t *)data));
    switch (id)
    {
    case E_ALGO_MSG_LOG_DEBUG_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_log_debug_ctl(*ctr);
        }
        break;
    case E_ALGO_MSG_HAND_UPDOWN_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_hand_updown_ctl(*ctr);
        }
        break;
    case E_ALGO_MSG_SLEEP_SET_TIME:
        {
            sleep_merge_set_config(data[0], data[1], data[2], data[3], sleep_wake_timeout);
        }
        break;
    case E_ALGO_MSG_SLEEP_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_sleep_ctl(*ctr);
        }
        break;
    case E_ALGO_MSG_FORCE_REQ_SLEEP_DATA:
        algo_force_req_sleep_data();
        break;
    case E_ALGO_MSG_SET_DATETIME:
        algo_set_datetime(data);
        break;
    case E_ALGO_MSG_SET_USER_INFO:
        algo_set_user_infio(data);
        break;
    case E_ALGO_MSG_SET_DEV_STATUS:
        {
            uint32_t *status = (uint32_t *)data;
            algo_set_dev_status(*status);
        }
        break;

    case E_ALGO_MSG_SET_WEAR_STATUS:
        {
            uint32_t *status = (uint32_t *)data;
            algo_set_wear_status(*status);
        }
        break;

    case E_ALGO_MSG_SET_HR_VALUE:
        {
            uint32_t *value = (uint32_t *)data;
            algo_set_hr_value(*value);
        }
        break;

    case E_ALGO_MSG_RESET_DAILY_INFO:
        algo_reset_daily_infio();
        break;

    case E_ALGO_MSG_AR_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_ar_ctl(*ctr);
        }
        break;
    case E_ALGO_MSG_ELLIPTICAL_TRAINER_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_elliptical_trainer_ctl(*ctr);
        }
        break;

    case E_ALGO_MSG_TREADMIL_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_treadmill_ctl(*ctr);
        }
        break;

    case E_ALGO_MSG_OUT_RUNNING_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_outdoor_running_ctl(*ctr);
        }
        break;

    case E_ALGO_MSG_TREADMIL_CALIBRATION:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_treadmill_calibration(*ctr);
        }
        break;

    case E_ALGO_MSG_ROPE_SKIPPING_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_rope_skip_ctl(*ctr);
        }
        break;

    case E_ALGO_MSG_ROWING_MACHINE_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_rowing_machine_ctl(*ctr);
        }
        break;

    case E_ALGO_MSG_OUT_RIDING_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_out_riding_ctl(*ctr);
        }
        break;

    case E_ALGO_MSG_RETURN_1001:
        algo_return_1001();
        break;

    case E_ALGO_MSG_ACTIVITY_PAUSE_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_activity_pause_ctl(*ctr);
        }
        break;
    case E_ALGO_MSG_FREE_TRAINING_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_freetraining_ctl(*ctr);
        }

    case E_ALGO_MSG_ABS_STATIC_CTL :
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_abs_static_config_ctl(*ctr);
        }
        break;
    case E_ALGO_MSG_SWIM_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_swim_ctl(*ctr);
        }
        break;
    case E_ALGO_MSG_POOL_LENGTH:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_set_pool_length(*ctr);
        }
        break;

    case  E_ALGO_MSG_OPEN_WATER_SWIM_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_open_water_swim_ctl(*ctr);
        }
        break;

    case E_ALGO_MSG_OPEN_SWIM_LENGTH:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_set_open_swim_length(*ctr);
        }
        break;

    case E_ALGO_MSG_SET_GPS_VALUE:
        algo_set_gps_data(data);
        break;

    case E_ALGO_MSG_OUT_RUNNING_TRACKING_CTL:
        {
            uint32_t *ctr = (uint32_t *)data;
            algo_outdoor_running_tracking_ctl(*ctr);
        }
        break;


#if defined(MESSAGE_SAMPLE_CODE) && (1 == MESSAGE_SAMPLE_CODE)
    case E_DML_ENABLE:
        {
            uint32_t *id = (uint32_t *)data;
            CWM_Sensor_Enable(*id);
        }
        break;
    case E_DML_DISABLE:
        {
            uint32_t *id = (uint32_t *)data;
            CWM_Sensor_Disable(*id);
        }
        break;
    case E_DML_SETING_CONTROL:
        {
            uint32_t *id = (uint32_t *)&data[0];
            SettingControl_t *scl = (SettingControl_t *)&data[4];
            CWM_SettingControl(*id, scl);
        }
        break;
#endif
    default:
        break;
    }
}

void cwm_algo_task(void *pvParameters)
{
    /* Provide differentiated initialization interface for different platforms */
    customio_init();

    algo_init();

    period_ms = 1000 / algo_get_odr();

#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
    /* W03 project requirement: for transferring info to global */
    extern struct dml_info *get_dml_info(void);
    extern void set_algo_version_string(int32_t *version);
    extern void set_dml_version_string(int32_t *version);
    set_algo_version_string(get_dml_info()->algo_version);
    set_dml_version_string(get_dml_info()->dml_version);
#endif

#if (CWM_HANDUPDOWN_Split == 1)
    void cwm_watch_handupdown_init(void);
    cwm_watch_handupdown_init();
#endif

    int64_t xLastWakeTime;
    xLastWakeTime = k_uptime_get();
    for (;;)
    {
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
        /* W03 project requirement: for updating time */
        systickUpdate();
#endif

        struct algo_msg_t msg;
        while (!get_msg((uint8_t *)&msg))
        {
            algo_message_handle(msg.id, &msg.data[0]);
        }

        if (true == algo_running)
        {
            cwm_sensor_ag_interrupt_handle();// read ag sensor data

            // // read mag data
            cwm_sensor_mag_interrupt_handle();

            // // read baro data
            // struct baro_t baro_data[25] = {0};
            // uint16_t baro_data_len = 0;
            // app_baro_get_fifo_data(baro_data, &baro_data_len);
            // cwm_baro_buf_in((uint8_t*)baro_data,baro_data_len);

#if (CWM_HANDUPDOWN_Split == 1)
            static uint32_t run_sn = 0;
            run_sn++;

            // Independent wrist lift algorithm execution function
            void cwm_call_watch_handupdown(void);
            cwm_call_watch_handupdown();

            if (run_sn % 5 == 0)
            {
                algo_data_input();//input data to algo, and execute algo
            }
            algo_data_handle();

#else
            algo_data_input();//input data to algo, and execute algo
            algo_data_handle();
#endif
        }

        // algo_test();

        // /*monitor algo task stack*/
        // cwm_app_debug("[algo]:used = %d,  free = %d\n",
        //     ALGO_STACK_SIZE*4 - uxTaskGetStackHighWaterMark(NULL)*4,
        //     uxTaskGetStackHighWaterMark(NULL)*4);

        /*sensor fifo waterlevel interrupt is 4*/
        period_ms = 5 * 1000 / algo_get_odr();
        //cwm_app_debug("[algo]:task period = %d\n",period_ms);
        xLastWakeTime += period_ms;
        int64_t now = k_uptime_get();
        int64_t sleep_time = xLastWakeTime - now;
        if (sleep_time > 0)
        {
            k_msleep(sleep_time);
        }
        else
        {
            xLastWakeTime = now;
        }
    }
}

/****************************************************External Call Interfaces************************************************/
/*ctr=1:enable; 0:disable*/
void cwm_log_debug_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_LOG_DEBUG_CTL, ctr);
}
/*ctr=1:enable; 0:disable*/
void cwm_algo_hand_updown_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_HAND_UPDOWN_CTL, ctr);
}
/*ctr=1:enable; 0:disable*/
void cwm_algo_ar_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_AR_CTL, ctr);
}
/*ctr=1:enable; 0:disable*/
void cwm_algo_elliptical_trainer_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_ELLIPTICAL_TRAINER_CTL, ctr);
}
/*ctr=1:enable; 0:disable*/
void cwm_algo_treadmil_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_TREADMIL_CTL, ctr);
}

// Treadmill distance calibration
void cwm_algo_treadmil_set_calibration_distance(uint32_t calibration_distance)
{
    message_to_algo(E_ALGO_MSG_TREADMIL_CALIBRATION, calibration_distance);
}

/*ctr=1:enable; 0:disable*/
void cwm_algo_out_running_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_OUT_RUNNING_CTL, ctr);
}

/*ctr=1:enable; 0:disable*/
void cwm_algo_rope_skipping_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_ROPE_SKIPPING_CTL, ctr);
}

/*ctr=1:enable; 0:disable*/
void cwm_algo_rowing_machine_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_ROWING_MACHINE_CTL, ctr);
}

/*ctr=1:enable; 0:disable*/
void cwm_algo_out_riding_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_OUT_RIDING_CTL, ctr);
}

/*ctr=1:enable; 0:disable*/
void cwm_swim_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_SWIM_CTL, ctr);
}

// Input pool length
void cwm_set_swim_length(uint32_t pool_length)
{
    message_to_algo(E_ALGO_MSG_POOL_LENGTH, pool_length);
}

/* Open water swim: ctr=1:enable; 0:disable */
void cwm_open_water_swim_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_OPEN_WATER_SWIM_CTL, ctr);
}

// Input open water swim segment distance
void cwm_set_open_swim_length(uint32_t open_swim_length)
{
    message_to_algo(E_ALGO_MSG_OPEN_SWIM_LENGTH, open_swim_length);
}

/* Return 1001 mode */
void cwm_algo_return_1001(void)
{
    message_to_algo(E_ALGO_MSG_RETURN_1001, 0);
}
/* ctr=1:pause; ctr=0:resume */
void cwm_algo_activity_pause_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_ACTIVITY_PAUSE_CTL, ctr);
}
/*ctr=1:enable; 0:disable*/
void cwm_algo_free_training_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_FREE_TRAINING_CTL, ctr);
}

/*ctr=1:enable; 0:disable*/
void cwm_algo_outdoor_running_tracking_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_OUT_RUNNING_TRACKING_CTL, ctr);
}


/*
Set sleep start and end time:
uint8_t st_hour: start time: hour
uint8_t st_min: start time: minute
uint8_t end_hour: end time: hour
uint8_t end_min: end time: minute
*/
void cwm_algo_set_sleep_time(uint8_t st_hour, uint8_t st_min, uint8_t end_hour, uint8_t end_min)
{
    uint32_t ctr;
    uint8_t *data = (uint8_t *)&ctr;
    data[0] = st_hour;
    data[1] = st_min;
    data[2] = end_hour;
    data[3] = end_min;
    message_to_algo(E_ALGO_MSG_SLEEP_SET_TIME, ctr);
}


/* ctr=2:nap; ctr=1:sleep; 0:off */
void cwm_algo_sleep_ctl(uint32_t ctr)
{
    uint32_t sleep_mode = 0;

    if (sleep_nap_state == 0) // Not in sleep/nap state
    {
        sleep_mode = ctr;
    }
    else
    {
        return;
    }

    message_to_algo(E_ALGO_MSG_SLEEP_CTL, sleep_mode);
}

/* Force sleep output data, terminate this sleep segment */
void cwm_algo_force_request_sleep_data(void)
{
    message_to_algo(E_ALGO_MSG_FORCE_REQ_SLEEP_DATA, 0);
}

/* ctr=1:client sets absolute rest, ctr=0:turn off absolute rest */
void cwm_algo_abs_static_ctl(uint32_t ctr)
{
    message_to_algo(E_ALGO_MSG_ABS_STATIC_CTL, ctr);
}


/*
uint8_t*data =
struct algo_datetime{
    uint8_t sec;//0~59
    uint8_t min;//0~59
    uint8_t hour;//0~23
    uint8_t day;//1~31
    uint8_t mon;//1~12
    uint16_t year; //year >= 2000, eg:2024
};
*/
void cwm_set_datetime_to_algo(uint8_t *data, uint16_t len)
{
    //max data len = MSG_DATA_MAX_SIZE byes
    data_to_algo(E_ALGO_MSG_SET_DATETIME, data, len);
}
/*
struct algo_user_info{
    uint8_t age;//1~99
    uint8_t gender;//0:female; 1:male
    uint16_t weiht;//10~300 (kg)
    uint16_t height;//30~300 (cm)
    uint16_t hand;//0:left; 1:right
};
*/
void cwm_set_user_info_to_algo(uint8_t *data, uint16_t len)
{
    //max data len = MSG_DATA_MAX_SIZE byes
    data_to_algo(E_ALGO_MSG_SET_USER_INFO, data, len);
}
/* Send device status to algorithm: mainly charge status, motor vibration, etc. Input parameters:
status:
enum{
    E_DEV_STATUS_CHARG_ON = 0,       //Charger connected
    E_DEV_STATUS_CHARG_OFF,
    E_DEV_STATUS_MOTOR_ON,           //Vibration motor enabled,
    E_DEV_STATUS_MOTOR_OFF,
    E_DEV_STATUS_TRIG_ALGO_ON,       //Event triggered algorithm flow
    E_DEV_STATUS_TRIG_ALGO_OFF,
    E_DEV_STATUS_SPEAKER_ON,         //Speaker active
    E_DEV_STATUS_SPEAKER_OFF,
    E_DEV_STATUS_LCD_SLIDE_ON,       //Screen operation
    E_DEV_STATUS_LCD_SLIDE_OFF,
    E_DEV_STATUS_KEY_COMBO_ON,       //Watch button operation
    E_DEV_STATUS_KEY_COMBO_OFF,
    E_DEV_STATUS_BT_CTL_ON,          //Phone operates watch via Bluetooth
    E_DEV_STATUS_BT_CTL_OFF,
}
*/
void cwm_set_device_status(uint32_t status)
{
    message_to_algo(E_ALGO_MSG_SET_DEV_STATUS, status);
}
/*
Send wear status to algorithm:
Wear status=1;
Not wear status=0;
*/
void cwm_set_wear_status(uint8_t status)
{
    static uint8_t pre_is_wear = 0xff;
    if (pre_is_wear != status)
    {
        pre_is_wear = status;
        message_to_algo(E_ALGO_MSG_SET_WEAR_STATUS, (uint32_t)status);
    }
}
/*
Send heart rate value to algorithm:
When wearing, value=heart rate value, send when data changes. hr range:((value >= 30) && (value <= 220))
When not wearing, only need to send once value=0;
*/
void cwm_set_hr_value(uint16_t value)
{
    static uint16_t pre_hr = 0xffff;
    if (pre_hr != value)
    {
        pre_hr = value;
        message_to_algo(E_ALGO_MSG_SET_HR_VALUE, (uint32_t)value);
    }
}

/*
Send GPS signal to algorithm:
struct algo_gps_data{
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
*/
void cwm_set_gps_value(uint8_t *data, uint16_t len)
{
    //max data len = MSG_DATA_MAX_SIZE byes
    data_to_algo(E_ALGO_MSG_SET_GPS_VALUE, data, len);
}

void cwm_reset_daily_info(void)
{
    message_to_algo(E_ALGO_MSG_RESET_DAILY_INFO, 0);
}








#if defined(MESSAGE_SAMPLE_CODE) && (1 == MESSAGE_SAMPLE_CODE)
//DML INTERFACE
void cwm_dml_enable(uint32_t id)
{
    message_to_algo(E_DML_ENABLE, id);
}
void cwm_dml_disable(uint32_t id)
{
    message_to_algo(E_DML_DISABLE, id);
}
void cwm_dml_seting_control(uint32_t id, SettingControl_t *scl)
{
    struct algo_msg_t msg = {0};
    msg.id = E_DML_SETING_CONTROL;
    memcpy(&msg.data[0], &id, 4);
    memcpy(&msg.data[4], scl, 4 * 16);
    data_to_algo(msg.id, msg.data, 4 + 4 * 16);
}
#endif







