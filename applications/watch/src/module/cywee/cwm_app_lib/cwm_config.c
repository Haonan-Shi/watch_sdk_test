#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "math.h"
#include "cwm_lib.h"
#include "cwm_lib_dml.h"
#include "cwm_config.h"
#include "cwm_customio.h"
#include "cwm_port.h"
#include "cwm_sleep_merge.h"

#if CWM_AR_RECALL
#include "ar_recall.h"
#endif
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
#include "bci.h"
#include "app_obs.h"
#endif

/*版本号说明：CWMP06_0.0.4.0.803.502
CWMP06：表示该项目名
0.0.4.0：表示版本号。
803: 表示版算法类型。
502：表示 dml 类型。
*/
#define ALGO_CONFIG_VERSION "CWMP07-1_0.0.5.24.861.502"
#define ALGO_RES_MAX_COUNT  20


#define SENSOR_DEFAULT  0
#define SENSOR_LP1      1
#define SENSOR_HP       2
#define TREADMIL_LAST_NOTIFY 1

#define SENSOR_BUF_ACC_EN 1
#define SENSOR_BUF_GYRO_EN 0
#define SENSOR_BUF_MAG_EN 0
#define SENSOR_BUF_BARO_EN 0
enum
{
#if defined(SENSOR_BUF_ACC_EN) && (1 == SENSOR_BUF_ACC_EN)
    E_SENSOR_BUF_ID_ACC = 0,
#endif
#if defined(SENSOR_BUF_GYRO_EN) && (1 == SENSOR_BUF_GYRO_EN)
    E_SENSOR_BUF_ID_GYRO,
#endif
#if defined(SENSOR_BUF_MAG_EN) && (1 == SENSOR_BUF_MAG_EN)
    E_SENSOR_BUF_ID_MAG,
#endif
#if defined(SENSOR_BUF_BARO_EN) && (1 == SENSOR_BUF_BARO_EN)
    E_SENSOR_BUF_ID_BARO,
#endif
    E_SENSOR_BUF_ID_MAX,
};
struct buf_t
{
    uint16_t size;
    uint16_t write;
    uint16_t read;
    uint16_t sizeof_frame;
    uint16_t max_size;
    uint8_t *data;
};
#if defined(SENSOR_BUF_ACC_EN) && (1 == SENSOR_BUF_ACC_EN)
#define SENSOR_ACC_SIZEOF_FRAME  12
#define SENSOR_ACC_MAX_FRAMES  52
uint8_t acc_data[SENSOR_ACC_SIZEOF_FRAME * SENSOR_ACC_MAX_FRAMES];
#endif

#if defined(SENSOR_BUF_GYRO_EN) && (1 == SENSOR_BUF_GYRO_EN)
#define SENSOR_GYRO_SIZEOF_FRAME  12
#define SENSOR_GYRO_MAX_FRAMES  52
uint8_t gyro_data[SENSOR_GYRO_SIZEOF_FRAME * SENSOR_GYRO_MAX_FRAMES];
#endif

#if defined(SENSOR_BUF_MAG_EN) && (1 == SENSOR_BUF_MAG_EN)
#define SENSOR_MAG_SIZEOF_FRAME  12
#define SENSOR_MAG_MAX_FRAMES  52
uint8_t mag_data[SENSOR_MAG_SIZEOF_FRAME * SENSOR_MAG_MAX_FRAMES];
#endif

#if defined(SENSOR_BUF_BARO_EN) && (1 == SENSOR_BUF_BARO_EN)
#define SENSOR_BARO_SIZEOF_FRAME  8
#define SENSOR_BARO_MAX_FRAMES  52
uint8_t baro_data[SENSOR_BARO_SIZEOF_FRAME * SENSOR_BARO_MAX_FRAMES];
#endif

struct buf_t buf_table[] =
{
#if defined(SENSOR_BUF_ACC_EN) && (1 == SENSOR_BUF_ACC_EN)
    {0, 0, 0, SENSOR_ACC_SIZEOF_FRAME, SENSOR_ACC_SIZEOF_FRAME * SENSOR_ACC_MAX_FRAMES, acc_data},
#endif
#if defined(SENSOR_BUF_GYRO_EN) && (1 == SENSOR_BUF_GYRO_EN)
    {0, 0, 0, SENSOR_GYRO_SIZEOF_FRAME, SENSOR_GYRO_SIZEOF_FRAME * SENSOR_GYRO_MAX_FRAMES, gyro_data},
#endif
#if defined(SENSOR_BUF_MAG_EN) && (1 == SENSOR_BUF_MAG_EN)
    {0, 0, 0, SENSOR_MAG_SIZEOF_FRAME, SENSOR_MAG_SIZEOF_FRAME * SENSOR_MAG_MAX_FRAMES, mag_data},
#endif
#if defined(SENSOR_BUF_BARO_EN) && (1 == SENSOR_BUF_BARO_EN)
    {0, 0, 0, SENSOR_BARO_SIZEOF_FRAME, SENSOR_BARO_SIZEOF_FRAME * SENSOR_BARO_MAX_FRAMES, baro_data},
#endif
};

struct sensor_setting_t
{
    uint16_t odr;
    uint16_t power_mode;
    uint16_t acc_range;
    uint16_t gyro_range;
};


struct hr_t
{
    uint8_t is_wear;
    uint8_t value;
};

#if CWM_DATA_PRE_HANDLE
struct algo_data_t
{
    uint32_t id;
    float f[16];
};
struct algo_res_t
{
    uint8_t size;
    uint8_t write;
    uint8_t read;
    struct algo_data_t data[ALGO_RES_MAX_COUNT];
};
#endif

struct ag_cali_back_t
{
    // /*工厂校正数据*/
    // uint16_t crc_16;//校验和
    // uint16_t spv_whole_status: 2;//spv 整机校正状态
    // uint16_t spv_pcba_status: 2;//spv pcba 校正状态
    // uint16_t sixface_status: 2;//六面校正状态
    // uint16_t valid:1;//数据可用
    // int32_t ax;
    // int32_t ay;
    // int32_t az;
    // int32_t gx;
    // int32_t gy;
    // int32_t gz;

    /*算法持续校正数据*/
    uint16_t auto_crc_16;//校验和
    uint16_t auto_valid: 1; //数据可用
    int32_t auto_ax;
    int32_t auto_ay;
    int32_t auto_az;
};
struct mag_cali_hard_t
{
    uint16_t crc_16;//校验和
    uint16_t valid: 1; //数据可用
    int32_t x;
    int32_t y;
    int32_t z;
};
struct algo_info_t
{
    struct dml_info dml_info;

    uint32_t dev_status;
    struct hr_t hr;
#if CWM_DATA_PRE_HANDLE
    struct algo_res_t algo_res;//欧拉角和四元素缓存
#endif
    //需要保存到 flash 的数据
    struct mag_cali_hard_t mag_cali_hard;//mag hard 校正参数
};

//跑步机校正
struct scl_treadmill_t
{
    float total_steps;
    float average_step_frequency;
    float uncalibration_disrance;
    float temp_total_steps;
    float temp_average_step_frequency;
    float temp_uncalibration_disrance;
    float treadmill_ground_truth_distance;
};


typedef struct
{
    int32_t activity_mode;
    int32_t sub_number;
    int32_t training_load_0;
    int16_t mhr_zone_1;
    int16_t mhr_zone_2;
    int16_t mhr_zone_3;
    int16_t mhr_zone_4;
    int16_t mhr_zone_5;
    int16_t mhr_zone_6;
} cwm_zone_time_t;


cwm_zone_time_t cwm_zone_time;




//跑步机校正
struct scl_treadmill_t treadmill_set;

//泳池长度
uint32_t cwm_pool_length = 0, cwm_open_swim_lenfgth = 0;

struct algo_info_t algo_dev_info;

static void algo_data_pre_handle(uint32_t id, float *f);


static uint8_t treadmill_flag, elliptical_trainer_flag, rope_skip_flag, rowing_machine_flag,
       out_run_tracking_flag = 0;
static uint32_t sensor_manual_ctr = 0;
////////////////////////////////////////////////////////////////本地接口////////////////////////////////////////////////////////
void algo_set_odr(uint16_t odr);
void algo_agmb_input_for_customer(struct acc_t *acc, struct gyro_t *gyro, struct mag_t *mag,
                                  struct baro_t *baro, uint16_t frames);

static uint16_t check_sum(uint8_t *data, uint32_t len)
{
    if (NULL == data) { return 0; }

    uint16_t sum = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum;
}

static void update_mag_cali_hard_checksum(void)
{
    /*mag hard 数据: 2 + 14  计算校验时，注意 4 字节对齐问题 */
    uint32_t len = sizeof(algo_dev_info.mag_cali_hard) - 2;
    uint8_t *addr = (uint8_t *)&algo_dev_info.mag_cali_hard + 2;
    algo_dev_info.mag_cali_hard.crc_16 = check_sum(addr, len);
}

static void algo_read_mag_hard_cali_from_flash(void)
{
    /*mag hard 校正数据*/
    struct mag_cali_hard_t tmp = {0};
    customio_read_flash_mag_hard_cali((uint8_t *)&tmp, sizeof(tmp));

    /*校正数据: 2 + 14*/
    uint32_t len = sizeof(tmp) - 2;
    uint8_t *addr = (uint8_t *)&tmp + 2;
    uint16_t crc_16 = check_sum(addr, len);
    if (crc_16 != tmp.crc_16)
    {
        algo_dev_info.mag_cali_hard.valid = 0;
        cwm_app_debug("[algo]read mag hard fail %d,%d\n", crc_16, tmp.crc_16);
    }
    else
    {
        memcpy((uint8_t *)&algo_dev_info.mag_cali_hard, (uint8_t *)&tmp, sizeof(tmp));
        cwm_app_debug("[algo]read mag hard success\n");
    }
}

void save_mag_cali_hard(void)
{
    customio_save_flash_mag_hard_cali((uint8_t *)&algo_dev_info.mag_cali_hard,
                                      sizeof(struct mag_cali_hard_t));
}

void get_mag_cali_hard(void)
{
    SettingControl_t scl;
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = 1;
    CWM_SettingControl(SCL_SENS_CALI_CTRL_MAG, &scl);
    algo_dev_info.mag_cali_hard.x = scl.iData[2];
    algo_dev_info.mag_cali_hard.y = scl.iData[3];
    algo_dev_info.mag_cali_hard.z = scl.iData[4];
    algo_dev_info.mag_cali_hard.valid = 1;

    update_mag_cali_hard_checksum();
}

void set_mag_cali_hard(void)
{
    cwm_app_debug("[algo]set_mag_cali_hard %d\n", algo_dev_info.mag_cali_hard.valid);
    if (1 == algo_dev_info.mag_cali_hard.valid)
    {
        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 2;
        scl.iData[2] = algo_dev_info.mag_cali_hard.x;
        scl.iData[3] = algo_dev_info.mag_cali_hard.y;
        scl.iData[4] = algo_dev_info.mag_cali_hard.z;
        scl.iData[5] = dml_mag_soft_iron[0];
        scl.iData[6] = dml_mag_soft_iron[1];
        scl.iData[7] = dml_mag_soft_iron[2];
        scl.iData[8] = dml_mag_soft_iron[3];
        scl.iData[9] = dml_mag_soft_iron[4];
        scl.iData[10] = dml_mag_soft_iron[5];
        scl.iData[11] = dml_mag_soft_iron[6];
        scl.iData[12] = dml_mag_soft_iron[7];
        scl.iData[13] = dml_mag_soft_iron[8];
        CWM_SettingControl(SCL_SENS_CALI_CTRL_MAG, &scl);
    }
}

static uint16_t sensor_data_get_size(uint8_t id)
{
    if (id >= E_SENSOR_BUF_ID_MAX) { return 0; }

    struct buf_t *buf = &buf_table[id];
    return buf->size;
}
static uint16_t sensor_data_get_framesize(uint8_t id)
{
    if (id >= E_SENSOR_BUF_ID_MAX) { return 0; }

    struct buf_t *buf = &buf_table[id];
    return buf->sizeof_frame;
}
static void sensor_data_write(uint8_t id, uint8_t *in_data, uint16_t frames)
{
    // cwm_app_debug("[algo]sensor_data_write %d,%d\n",id,frames);
    if (id >= E_SENSOR_BUF_ID_MAX) { return; }

    struct buf_t *buf = &buf_table[id];
    /*如果在 cwm_algo_task 中调用 cwm_sensor_ag_interrupt_handle，可以不用临界保护 */
    cwm_taskENTER_CRITICAL(id);
    for (uint16_t i = 0; i < frames; i++)
    {
        memcpy(&buf->data[buf->write], &in_data[i * buf->sizeof_frame], buf->sizeof_frame);
        buf->write += buf->sizeof_frame;
        buf->size += buf->sizeof_frame;

        if (buf->write >= buf->max_size)
        {
            buf->write = 0;
        }
        if (buf->size >= buf->max_size)
        {
            buf->size = buf->max_size;
            buf->read = buf->write;
        }
    }
    /*如果在 cwm_algo_task 中调用 cwm_sensor_ag_interrupt_handle，可以不用临界保护 */
    cwm_taskEXIT_CRITICAL(id);
}
void cwm_acc_buf_in(uint8_t *data, uint16_t frames)
{
#if defined(SENSOR_BUF_ACC_EN) && (1 == SENSOR_BUF_ACC_EN)
    sensor_data_write(E_SENSOR_BUF_ID_ACC, data, frames);
#endif
}
void cwm_gyro_buf_in(uint8_t *data, uint16_t frames)
{
#if defined(SENSOR_BUF_GYRO_EN) && (1 == SENSOR_BUF_GYRO_EN)
    sensor_data_write(E_SENSOR_BUF_ID_GYRO, data, frames);
#endif
}
void cwm_mag_buf_in(uint8_t *data, uint16_t frames)
{
#if defined(SENSOR_BUF_MAG_EN) && (1 == SENSOR_BUF_MAG_EN)
    sensor_data_write(E_SENSOR_BUF_ID_MAG, data, frames);
#endif
}
void cwm_baro_buf_in(uint8_t *data, uint16_t frames)
{
#if defined(SENSOR_BUF_BARO_EN) && (1 == SENSOR_BUF_BARO_EN)
    sensor_data_write(E_SENSOR_BUF_ID_BARO, data, frames);
#endif
}
void cwm_sensor_ag_interrupt_handle(void)
{
#if defined(SENSOR_BUF_GYRO_EN) && (1 == SENSOR_BUF_GYRO_EN)
    // cwm_app_debug("[algo]CWM_Dml_process3: 1,1\n");
    CWM_Dml_process3(1, 1);
#elif defined(SENSOR_BUF_ACC_EN) && (1 == SENSOR_BUF_ACC_EN)
    // cwm_app_debug("[algo]CWM_Dml_process3: 2,1\n");
    CWM_Dml_process3(2, 1);
#endif

}
void cwm_sensor_mag_interrupt_handle(void)
{
#if defined(SENSOR_BUF_MAG_EN) && (1 == SENSOR_BUF_MAG_EN)
    // cwm_app_debug("[algo]CWM_Dml_process3: 3,1\n");
    CWM_Dml_process3(3, 1);
#endif
}
static void sensor_data_read(uint8_t id, uint8_t *one_frame)
{
    // cwm_app_debug("[algo]sensor_data_read %d\n",id);

    if (id >= E_SENSOR_BUF_ID_MAX) { return; }
    if (NULL == one_frame) { return; }

    struct buf_t *buf = &buf_table[id];

    cwm_taskENTER_CRITICAL(id);
    if (sensor_data_get_size(id))
    {
        memcpy(one_frame, &buf->data[buf->read], buf->sizeof_frame);
        buf->read += buf->sizeof_frame;
        if (buf->read >= buf->max_size)
        {
            buf->read = 0;
        }
        buf->size -= buf->sizeof_frame;
    }
    cwm_taskEXIT_CRITICAL(id);
}
#if CWM_DATA_PRE_HANDLE
static void algo_res_write(uint32_t id, float *f)
{
    struct algo_res_t *buf = &algo_dev_info.algo_res;
    buf->data[buf->write].id = id;
    memcpy(buf->data[buf->write++].f, f, 4 * 16);
    buf->size++;
    if (buf->write >= ALGO_RES_MAX_COUNT)
    {
        buf->write = 0;
    }
    if (buf->size >= ALGO_RES_MAX_COUNT)
    {
        buf->size = ALGO_RES_MAX_COUNT;
        buf->read = buf->write;
    }
}

static void algo_res_read(void)
{
    struct algo_res_t *buf = &algo_dev_info.algo_res;
    for (uint8_t i = 0; i < buf->size; i++)
    {
        uint32_t id = buf->data[buf->read].id;
        float *f = buf->data[buf->read++].f;

        algo_data_pre_handle(id, f);

        if (buf->read >= ALGO_RES_MAX_COUNT)
        {
            buf->read = 0;
        }
    }
    buf->size = 0;
}
#endif

#if TREADMIL_LAST_NOTIFY
static float treadmill_sport_last_notify[16] = {0};

void set_last_treadmill_sport(float *f)
{
    memcpy(treadmill_sport_last_notify, f, sizeof(treadmill_sport_last_notify));
}
#endif


//100X
void set_cwm_100X(float *f)
{
    if (1002 == f[0])
    {
        if (treadmill_flag)
        {
            treadmill_set.temp_total_steps = f[1];
            customio_notify_treadmill_sport(f);
#if TREADMIL_LAST_NOTIFY
            set_last_treadmill_sport(f);
#endif
        }
    }
    else if (1003 == f[0])
    {
        customio_notify_outdoor_running_sport(f);
    }
    else if (1009 == f[0]) //GPS轨迹优化
    {
        customio_notify_out_run_tracking(f);
    }
    else if (100 == f[0])
    {
        if (treadmill_flag)
        {
            treadmill_set.temp_average_step_frequency = f[1];
            treadmill_set.temp_uncalibration_disrance = f[9];
            customio_notify_treadmill_sport(f);
        }
        else if (out_run_tracking_flag)
        {
            customio_notify_out_run_tracking(f);
        }
        else
        {
            customio_notify_outdoor_running_sport(f);
        }
    }
    else if (1 == f[0])  //GPS轨迹优化
    {
        customio_notify_out_run_tracking(f);
    }
}

//200X
void set_swim_data(float *f)
{
    customio_notify_swim_data(f);
}

//2002,2006
void set_open_swim_data(float *f)
{
    customio_notify_open_swim_data(f);
}

//300X
void set_biking_data(float *f)
{
    customio_notify_out_riding_sport(f);
}


//1008
void set_cwm_1008(float *f)
{
    customio_notify_out_free_training_sport(f);
}

//600X
void set_cwm_600X(float *f)
{
    if (6001 == f[0])
    {
        if (rowing_machine_flag)
        {
            customio_notify_set_rowing_machine_sport(f);
        }
        else if (rope_skip_flag)
        {
            customio_notify_rope_skipping_sport(f);
        }
        else if (elliptical_trainer_flag)
        {
            customio_notify_elliptical_trainer_sport(f);
        }
    }
    else if (150 == f[0])
    {
        if (rowing_machine_flag)
        {
            customio_notify_set_rowing_machine_sport(f);
        }
        else if (rope_skip_flag)
        {
            customio_notify_rope_skipping_sport(f);
        }
        else if (elliptical_trainer_flag)
        {
            customio_notify_elliptical_trainer_sport(f);
        }
    }
}

static void algo_data_pre_handle(uint32_t id, float *f)
{
    switch (id)
    {
    case 9://IDX_ALGO_WATCH_HANDUP:/*抬腕放腕算法输出*/
        /*此处添加客户接口：将算法结果传给客户*/
        if (1 == f[1])
        {
            customio_notify_handup();
        }
        else if (2 == f[1])
        {
            customio_notify_handdown();
        }
        break;

    case 14://IDX_ALGO_ACTIVITY_OUTPUT:/*活动算法输出*/
        if (1001 == f[0])
        {
            /*此处添加客户接口：将算法结果传给客户*/
            customio_notify_normal_steps(f[1]);
            customio_notify_normal_distance(f[2]);
            customio_notify_cal(f[3], f[8]);
        }
        else if (1002 == f[0] || 1003 == f[0] || 1009 == f[0] || 100 == f[0] || 1 == f[0])
        {
            set_cwm_100X(f);
        }
        else if (2004 == f[0] || 110 == f[0] ||
                 111 == f[1])  //2004三轴游泳:划水次数、划水频率、平均配速、累计游泳趟数、累计游泳时长、swolf，结束显示泳姿
        {
            set_swim_data(f);
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
            extern void sport_set_swim_data(const float s[16]);
            sport_set_swim_data(f);
#endif
        }
        else if (2002 == f[0] || 114 == f[0] || 115 == f[0] || 2006 == f[0]) //户外游泳
        {
            set_open_swim_data(f);
        }
        else if (3001 == f[0] || 120 == f[0])  //户外骑行
        {
            set_biking_data(f);
        }
        else if (1008 == f[0]) //自由训练
        {
            set_cwm_1008(f);
        }
        else if (6001 == f[0] || 150 == f[0])  //6001运动
        {
            set_cwm_600X(f);
        }
        else if (102 == f[0]) //AR输出结果
        {
            customio_notify_ar_sport(f[1]);
        }
        else if (106 == f[0]) //AR回溯sensor输出
        {
            customio_notify_Sens_Reg_output((int32_t)f[1]);
        }
        break;
    }
}

static void set_sensor_agm(uint8_t sensor, struct sensor_setting_t *setting)
{
    if (sensor)
    {
        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 2;
        CWM_SettingControl(SCL_DML_DRV_AG_CONFIG, &scl);
        scl.iData[1] = 1;
        scl.iData[8] = setting->odr;
        scl.iData[9] = setting->acc_range;
        scl.iData[10] = setting->gyro_range;
        CWM_SettingControl(SCL_DML_DRV_AG_CONFIG, &scl);
        algo_set_odr(scl.iData[8]);
        algo_dev_info.dml_info.acc_rang = scl.iData[9];
        algo_dev_info.dml_info.gyro_rang = scl.iData[10];

        // memset(&scl, 0, sizeof(scl));
        // scl.iData[0] = 1;
        // scl.iData[1] = setting->odr;
        // CWM_SettingControl(SCL_ALGO_PROC_CONFIG, &scl);//设置传感器的ODR，启用重采样流程

        // memset(&scl, 0, sizeof(scl));
        // if(SENSOR_DEFAULT == setting->power_mode){
        //     memcpy(&scl,dml_ag_pref_config_default,sizeof(scl));
        //     CWM_SettingControl(SCL_DML_DRV_AG_PERF_CONFIG, &scl);
        // }
        // else if(SENSOR_STANDBY == setting->power_mode){
        //     memcpy(&scl,dml_ag_pref_config_standby,sizeof(scl));
        //     CWM_SettingControl(SCL_DML_DRV_AG_PERF_CONFIG, &scl);
        // }



        memcpy(&scl, dml_mag_config, sizeof(scl));
        CWM_SettingControl(SCL_DML_DRV_M_CONFIG, &scl);


        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 1; // enable DT calib
        CWM_SettingControl(SCL_INPUT_DT_CONFIG, &scl);

        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 1;
        scl.iData[2] = sensor;
        CWM_SettingControl(SCL_DML_DRV_ENABLE, &scl);
    }
    else
    {
        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 1;
        scl.iData[2] = 0;
        CWM_SettingControl(SCL_DML_DRV_ENABLE, &scl);

        //关闭所有用到的算法
        CWM_Sensor_Disable(IDX_ACCEL);
        CWM_Sensor_Disable(IDX_GYRO);
        CWM_Sensor_Disable(IDX_MAG);
    }
}

static void algo_datetime_input(void)
{
    struct algo_datetime date;
    customio_get_datetime((uint8_t *)&date);

    SettingControl_t scl;
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = date.year;
    scl.iData[2] = date.mon;
    scl.iData[3] = date.day;
    scl.iData[4] = date.hour;
    scl.iData[5] = date.min;
    scl.iData[6] = date.sec;
    CWM_SettingControl(SCL_DATE_TIME, &scl);

    cwm_app_debug("[algo]algo_datetime_input:%d-%d-%d:%d:%d:%d\n",
                  date.year,
                  date.mon,
                  date.day,
                  date.hour,
                  date.min,
                  date.sec);
}
static void algo_agmb_input(void)
{
    uint16_t acc_frames = 0;
#if defined(SENSOR_BUF_ACC_EN) && (1 == SENSOR_BUF_ACC_EN)
    acc_frames = sensor_data_get_size(E_SENSOR_BUF_ID_ACC) / sensor_data_get_framesize(
                     E_SENSOR_BUF_ID_ACC);

    /*必须有 acc 数据才行，否则无法对齐*/
    if (!acc_frames) { return; }
#endif
#if defined(SENSOR_BUF_GYRO_EN) && (1 == SENSOR_BUF_GYRO_EN)
    uint16_t gyro_frames = sensor_data_get_size(E_SENSOR_BUF_ID_GYRO) / sensor_data_get_framesize(
                               E_SENSOR_BUF_ID_GYRO);
#endif
#if defined(SENSOR_BUF_MAG_EN) && (1 == SENSOR_BUF_MAG_EN)
    uint16_t mag_frames = sensor_data_get_size(E_SENSOR_BUF_ID_MAG) / sensor_data_get_framesize(
                              E_SENSOR_BUF_ID_MAG);
    uint8_t tmp_mag_data[SENSOR_MAG_SIZEOF_FRAME * SENSOR_MAG_MAX_FRAMES];
    uint16_t mag_nums = 0;

    if (!mag_frames)
    {
        /*不做任何操作，也不会传给算法*/
    }
    else if (mag_frames > acc_frames)
    {
        /*数据对齐方法：A/M=D, A%M=R,
            (1) A=M*D+R;
            (2) A=xD+y(D+1)
                 =(x+y)D+y
            ==> x=M-R; y=R
        如：7/4=1 余 3，7 可以拆分为 1+2+2+2，所有的数据都可以如此拆分
        如：7/1=7 余 0，7 可以拆分为 7，地磁和气压计应该取最后一个才对，因为这个值才能表示当前的地磁和气压
        */
        uint8_t mag_idx[SENSOR_MAG_MAX_FRAMES] = {0};
        uint16_t D = mag_frames / acc_frames;
        uint16_t R = mag_frames % acc_frames;
        uint16_t x = acc_frames - R;
        uint16_t y = R;
        uint16_t idx_st = 0;

        for (uint16_t i = 0; i < mag_frames;)
        {
            if (x)
            {
                idx_st += D;
                mag_idx[idx_st - 1] = 1;
                x--;
                i++;
            }
            if (y)
            {
                idx_st += D + 1;
                mag_idx[idx_st - 1] = 1;
                y--;
                i++;
            }
            if (!(x + y))
            {
                break;
            }
        }
        for (uint16_t i = 0; i < mag_frames; i++)
        {
            if (mag_idx[i])
            {
                /*重采样后的数据*/
                uint8_t *dest = (uint8_t *)&tmp_mag_data[mag_nums * SENSOR_MAG_SIZEOF_FRAME];
                sensor_data_read(E_SENSOR_BUF_ID_MAG, dest);
                mag_nums++;
            }
            else
            {
                /*丢弃掉的数据*/
                struct mag_t mag_f = {0};
                sensor_data_read(E_SENSOR_BUF_ID_MAG, (uint8_t *)&mag_f);
            }
        }
    }
    else if (mag_frames < acc_frames)
    {
        uint8_t mag_idx[SENSOR_MAG_MAX_FRAMES] = {0};
        uint16_t D = acc_frames / mag_frames;
        uint16_t R = acc_frames % mag_frames;
        uint16_t x = mag_frames - R;
        uint16_t y = R;
        uint16_t idx_st = 0;

        for (uint16_t i = 0; i < acc_frames;)
        {
            if (x)
            {
                // idx_st += D;
                // mag_idx[idx_st - D] = 1;/*需要确保首位置有数据，这样操作较简单,所以全部移动 D*/
                mag_idx[idx_st] = 1;/*需要确保首位置有数据，这样操作较简单,所以全部移动 D*/
                idx_st += D;
                x--;
                i++;
            }
            if (y)
            {
                // idx_st += D+1;
                // mag_idx[idx_st - D-1] = 1;/*需要确保首位置有数据，这样操作较简单,所以全部移动 D*/
                mag_idx[idx_st] = 1;/*需要确保首位置有数据，这样操作较简单,所以全部移动 D*/
                idx_st += D + 1;
                y--;
                i++;
            }
            if (!(x + y))
            {
                break;
            }
        }
        for (uint16_t i = 0; i < acc_frames; i++)
        {
            if (mag_idx[i])
            {
                /*重采样后的数据*/
                uint8_t *dest = (uint8_t *)&tmp_mag_data[i * SENSOR_MAG_SIZEOF_FRAME];
                sensor_data_read(E_SENSOR_BUF_ID_MAG, dest);
            }
            else
            {
                /*需要 copy 之前的数据*/
                uint8_t *dest = (uint8_t *)&tmp_mag_data[i * SENSOR_MAG_SIZEOF_FRAME];
                uint8_t *pre  = (uint8_t *)&tmp_mag_data[(i - 1) * SENSOR_MAG_SIZEOF_FRAME];
                memcpy(dest, pre, SENSOR_MAG_SIZEOF_FRAME);
            }
        }
    }
    else
    {
        for (uint16_t i = 0; i < mag_frames; i++)
        {
            sensor_data_read(E_SENSOR_BUF_ID_MAG, (uint8_t *)&tmp_mag_data[i * SENSOR_MAG_SIZEOF_FRAME]);
        }
    }
#endif
#if defined(SENSOR_BUF_BARO_EN) && (1 == SENSOR_BUF_BARO_EN)
    uint16_t baro_frames = sensor_data_get_size(E_SENSOR_BUF_ID_BARO) / sensor_data_get_framesize(
                               E_SENSOR_BUF_ID_BARO);
    uint8_t tmp_baro_data[SENSOR_BARO_SIZEOF_FRAME * SENSOR_BARO_MAX_FRAMES];
    uint16_t baro_nums = 0;

    if (!baro_frames)
    {
        /*不做任何操作，也不会传给算法*/
    }
    else if (baro_frames > acc_frames)
    {
        /*数据对齐方法：A/M=D, A%M=R,
            (1) A=M*D+R;
            (2) A=xD+y(D+1)
                 =(x+y)D+y
            ==> x=M-R; y=R
        如：7/4=1 余 3，7 可以拆分为 1+2+2+2，所有的数据都可以如此拆分
        如：7/1=7 余 0，7 可以拆分为 7，地磁和气压计应该取最后一个才对，因为这个值才能表示当前的地磁和气压
        */
        uint8_t baro_idx[SENSOR_BARO_MAX_FRAMES] = {0};
        uint16_t D = baro_frames / acc_frames;
        uint16_t R = baro_frames % acc_frames;
        uint16_t x = acc_frames - R;
        uint16_t y = R;
        uint16_t idx_st = 0;

        for (uint16_t i = 0; i < baro_frames;)
        {
            if (x)
            {
                idx_st += D;
                baro_idx[idx_st - 1] = 1;
                x--;
                i++;
            }
            if (y)
            {
                idx_st += D + 1;
                baro_idx[idx_st - 1] = 1;
                y--;
                i++;
            }
            if (!(x + y))
            {
                break;
            }
        }
        for (uint16_t i = 0; i < baro_frames; i++)
        {
            if (baro_idx[i])
            {
                /*重采样后的数据*/
                uint8_t *dest = (uint8_t *)&tmp_baro_data[baro_nums * SENSOR_BARO_SIZEOF_FRAME];
                sensor_data_read(E_SENSOR_BUF_ID_BARO, dest);
                baro_nums++;
            }
            else
            {
                /*丢弃掉的数据*/
                struct baro_t baro_f = {0};
                sensor_data_read(E_SENSOR_BUF_ID_BARO, (uint8_t *)&baro_f);
            }
        }
    }
    else if (baro_frames < acc_frames)
    {
        uint8_t baro_idx[SENSOR_BARO_MAX_FRAMES] = {0};
        uint16_t D = acc_frames / baro_frames;
        uint16_t R = acc_frames % baro_frames;
        uint16_t x = baro_frames - R;
        uint16_t y = R;
        uint16_t idx_st = 0;

        for (uint16_t i = 0; i < acc_frames;)
        {
            if (x)
            {
                // idx_st += D;
                // baro_idx[idx_st - D] = 1;/*需要确保首位置有数据，这样操作较简单,所以全部移动 D*/
                baro_idx[idx_st] = 1;/*需要确保首位置有数据，这样操作较简单,所以全部移动 D*/
                idx_st += D;
                x--;
                i++;
            }
            if (y)
            {
                // idx_st += D+1;
                // baro_idx[idx_st - D-1] = 1;/*需要确保首位置有数据，这样操作较简单,所以全部移动 D*/
                baro_idx[idx_st] = 1;/*需要确保首位置有数据，这样操作较简单,所以全部移动 D*/
                idx_st += D + 1;
                y--;
                i++;
            }
            if (!(x + y))
            {
                break;
            }
        }
        for (uint16_t i = 0; i < acc_frames; i++)
        {
            if (baro_idx[i])
            {
                /*重采样后的数据*/
                uint8_t *dest = (uint8_t *)&tmp_baro_data[i * SENSOR_BARO_SIZEOF_FRAME];
                sensor_data_read(E_SENSOR_BUF_ID_BARO, dest);
            }
            else
            {
                /*需要 copy 之前的数据*/
                uint8_t *dest = (uint8_t *)&tmp_baro_data[i * SENSOR_BARO_SIZEOF_FRAME];
                uint8_t *pre  = (uint8_t *)&tmp_baro_data[(i - 1) * SENSOR_BARO_SIZEOF_FRAME];
                memcpy(dest, pre, SENSOR_BARO_SIZEOF_FRAME);
            }
        }
    }
    else
    {
        for (uint16_t i = 0; i < baro_frames; i++)
        {
            sensor_data_read(E_SENSOR_BUF_ID_BARO, (uint8_t *)&tmp_baro_data[i * SENSOR_BARO_SIZEOF_FRAME]);
        }
    }
#endif

    for (uint16_t i = 0; i < acc_frames; i++)
    {
        struct acc_t *acc_p = NULL;
        struct gyro_t *gyro_P = NULL;
        struct mag_t *mag_p = NULL;
        struct baro_t *baro_p = NULL;

#if defined(SENSOR_BUF_GYRO_EN) && (1 == SENSOR_BUF_GYRO_EN)
        struct gyro_t gyro_f = {0};
        if (gyro_frames)
        {
            sensor_data_read(E_SENSOR_BUF_ID_GYRO, (uint8_t *)&gyro_f);
            gyro_P = &gyro_f;
        }

#endif
#if defined(SENSOR_BUF_MAG_EN) && (1 == SENSOR_BUF_MAG_EN)
        if (mag_frames)
        {
            mag_p = (struct mag_t *)&tmp_mag_data[i * SENSOR_MAG_SIZEOF_FRAME];
        }
#endif
#if defined(SENSOR_BUF_BARO_EN) && (1 == SENSOR_BUF_BARO_EN)
        if (baro_frames)
        {
            baro_p = (struct baro_t *)&tmp_baro_data[i * SENSOR_BARO_SIZEOF_FRAME];
        }
#endif
#if defined(SENSOR_BUF_ACC_EN) && (1 == SENSOR_BUF_ACC_EN)
        struct acc_t acc_f = {0};
        sensor_data_read(E_SENSOR_BUF_ID_ACC, (uint8_t *)&acc_f);
        acc_p = &acc_f;
#endif

#if defined(SENSOR_BUF_ACC_EN) && (1 == SENSOR_BUF_ACC_EN)
        algo_agmb_input_for_customer(acc_p, gyro_P, mag_p, baro_p, 1);

#endif
    }
}

/*
入参：acc、gyro、mag、baro 是对齐后的数据
    frames 是对齐后数据的帧数量
*/
void algo_agmb_input_for_customer(struct acc_t *acc, struct gyro_t *gyro,
                                  struct mag_t *mag, struct baro_t *baro, uint16_t frames)
{
    if ((0 == frames) || (NULL == acc)) { return; }

    for (uint16_t i = 0; i < frames; i++)
    {
        if (NULL != gyro)
        {
            CustomSensorData csd;
            memset(&csd, 0, sizeof(csd));
            csd.sensorType = CUSTOM_GYRO;
            csd.fData[0] = gyro[i].gx;
            csd.fData[1] = gyro[i].gy;
            csd.fData[2] = gyro[i].gz;
            CWM_CustomSensorInput(&csd);
        }

        if (NULL != mag)
        {
            CustomSensorData csd;
            memset(&csd, 0, sizeof(csd));
            csd.sensorType = CUSTOM_MAG;
            csd.fData[4] = mag[i].x;
            csd.fData[5] = mag[i].y;
            csd.fData[6] = mag[i].z;
            CWM_CustomSensorInput(&csd);
        }

        if (NULL != baro)
        {
            CustomSensorData csd;
            memset(&csd, 0, sizeof(csd));
            csd.sensorType = CUSTOM_BARO;
            csd.fData[0] = baro[i].pres / 100;
            if (baro_tmp_suppot)
            {
                csd.fData[1] = 1;
                csd.fData[2] = baro[i].tmp;
            }
            CWM_CustomSensorInput(&csd);
        }

        if (NULL != acc)
        {
            CustomSensorData csd;
            memset(&csd, 0, sizeof(csd));
            csd.sensorType = CUSTOM_ACC;
            csd.fData[0] = acc[i].ax;
            csd.fData[1] = acc[i].ay;
            csd.fData[2] = acc[i].az;
            CWM_CustomSensorInput(&csd);
            CWM_process2((1000 / algo_get_odr()) * 1000);
        }
    }
}

static void algo_set_1001(uint8_t en)
{
    if (en)
    {
        SettingControl_t scl;
        memcpy(&scl, dml_activity_config, sizeof(scl));
        CWM_SettingControl(SCL_ACTIVITY_CONFIG, &scl);

        //step count, balance mode
        memcpy(&scl, dml_pedo_config, sizeof(scl));
        CWM_SettingControl(SCL_PEDO_CONFIG, &scl);

        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 1001;
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);
        CWM_Sensor_Enable(14);
    }
    else
    {
        CWM_Sensor_Disable(14);
    }
}
static void OS_algo_listen(pSensorEVT_t sensorEVT)
{
    float *f = sensorEVT->fData;
    switch (sensorEVT->sensorType)
    {
    case IDX_REQUEST_SENSOR:
        {
            /*根据算法需求，输出 sensor 的开关需求。
            (0: acc, 1: gyro, 2: mag, 3: baro, 4: temperature, 5: heart_rate, 6: GNSS(GPS))*/
            uint32_t sensor = f[1];
            uint32_t on_off = f[2];

            if (sensor <= CUSTOM_GNSS)
            {
                if (on_off)
                {
                    sensor_manual_ctr |= ((uint32_t)1) << sensor;
                }
                else
                {
                    sensor_manual_ctr &= ~(((uint32_t)1) << sensor) ;
                }
            }

            cwm_app_debug("[algo]IDX_REQUEST_SENSOR: %f, %f\n",
                          f[1],
                          f[2]);
        }
        break;
    case IDX_ACCEL:
        break;
    case IDX_GYRO:
        break;
    case IDX_ALGO_SENS_CALIBRATION:
        break;
    case 9://IDX_ALGO_WATCH_HANDUP:/*抬腕放腕算法输出*/
#if CWM_DATA_PRE_HANDLE
        algo_res_write(9, f); //IDX_ALGO_WATCH_HANDUP
#else
        algo_data_pre_handle(9, f);
#endif
        break;
    case 14://IDX_ALGO_ACTIVITY_OUTPUT:/*活动算法输出*/
#if CWM_DATA_PRE_HANDLE
        algo_res_write(14, f); //IDX_ALGO_ACTIVITY_OUTPUT
#else
        algo_data_pre_handle(14, f);
#endif

#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
        sport_active_mode_set_data(sensorEVT->fData);
        OBS_NOTIFY(OBS_EVT_ALG_SLEEP, sensorEVT->fData);
        extern bci_t bci_alg_tx_no_ack(uint16_t idx, const float * dt, uint16_t len);
        bci_alg_tx_no_ack(14, sensorEVT->fData, 16);
#endif
        break;
    case 19:
        customio_notify_abs_static();
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
        OBS_NOTIFY(OBS_EVT_ALG_ABS, sensorEVT->fData);
        extern bci_t bci_alg_tx_no_ack(uint16_t idx, const float * dt, uint16_t len);
        bci_alg_tx_no_ack(IDX_ALGO_ABSOLUTE_STATIC, sensorEVT->fData, 16);
        cwm_app_debug("@@@@@@IDX_ALGO_ABSOLUTE_STATIC\n");
#endif
        break;

    case 22://IDX_ALGO_INACTIVITY_OUTPUT:/*非活动算法输出。睡眠数据特殊：在出睡瞬间，会输出大量的睡眠数据 50 多条，不能在这里缓存*/
        if ((21 == f[0]) || (22 == f[0]) || (31 == f[0]) || (32 == f[0]))
        {
            customio_notify_sleep_status(f[0], f[2]);

            if (sleep_merge_en)
            {
                // nap 时，sleep merge 不输出数据。但是仍然需要将算法数据传给给 sleep merge
                // 原因： sleep merge 会根据算法状态清 buffer
                sleep_merge_input(f);

#if (SLEEPMERGE_OUTPUTNAP_CumulativeResults == 0)
                static uint16_t index = 0;
                if (32 == f[0])
                {
                    struct sleep_total_t tmp;
                    tmp.mins_total      = f[1];
                    tmp.mins_light_sleep = f[2];
                    tmp.mins_deep_sleep = f[3];
                    tmp.mins_wake_sleep = f[4];
                    tmp.mins_blink_sleep = f[5];
                    customio_notify_sleep_data(index++, f[0], 0, (uint8_t *)&tmp);
                    index = 0;
                }
                else if (22 == f[0])
                {
                    struct sleep_datetime tmp;
                    tmp.year    = f[7];
                    tmp.mon     = f[3];
                    tmp.day     = f[4];
                    tmp.hour    = f[5];
                    tmp.min     = f[6];
                    customio_notify_sleep_data(index++, f[0], f[2], (uint8_t *)&tmp);
                }
#endif
            }
            else
            {
                static uint16_t index = 0;
                if ((31 == f[0]) || (32 == f[0]))
                {
                    struct sleep_total_t tmp;
                    tmp.mins_total      = f[1];
                    tmp.mins_light_sleep = f[2];
                    tmp.mins_deep_sleep = f[3];
                    tmp.mins_wake_sleep = f[4];
                    tmp.mins_blink_sleep = f[5];
                    customio_notify_sleep_data(index++, f[0], 0, (uint8_t *)&tmp);
                    index = 0;
                }
                else
                {
                    struct sleep_datetime tmp;
                    tmp.year    = f[7];
                    tmp.mon     = f[3];
                    tmp.day     = f[4];
                    tmp.hour    = f[5];
                    tmp.min     = f[6];
                    customio_notify_sleep_data(index++, f[0], f[2], (uint8_t *)&tmp);
                }
            }
        }
        else if ((20 == f[0]) && (1 == f[2]))
        {
            customio_notify_sedentary();
        }
        else
        {
#if CWM_DATA_PRE_HANDLE
            algo_res_write(22, f); //IDX_ALGO_INACTIVITY_OUTPUT
#else
            algo_data_pre_handle(22, f);
#endif
        }
#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
        extern bci_t bci_alg_tx_no_ack(uint16_t idx, const float * dt, uint16_t len);
        bci_alg_tx_no_ack(22, sensorEVT->fData, 16);
#endif
        break;
    default:
        break;
    }
}



static void dml_algo_init(void)
{
    int i = 0;
    SettingControl_t scl;

    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    CWM_SettingControl(SCL_GET_LIB_INFO, &scl);
    cwm_app_debug("[algo] algo version:%d.%d.%d.%d product:%d model:%d\n", scl.iData[1], scl.iData[2],
                  scl.iData[3], scl.iData[4], scl.iData[5], scl.iData[6]);
    algo_dev_info.dml_info.algo_version[0] = scl.iData[1];
    algo_dev_info.dml_info.algo_version[1] = scl.iData[2];
    algo_dev_info.dml_info.algo_version[2] = scl.iData[3];
    algo_dev_info.dml_info.algo_version[3] = scl.iData[4];
    algo_dev_info.dml_info.algo_version[4] = scl.iData[5];
    algo_dev_info.dml_info.algo_version[5] = scl.iData[6];

    /* -----------------algo_dml_init------------------------ */
    CWM_LibPreInit(&customio_os_api);

    /* 设置MCU芯片信息, 必须在 CWM_LibPreInit() 之后， CWM_LibPostInit() 之前设置 */
    memcpy(&scl, dml_vendor_config, sizeof(scl));
    CWM_SettingControl(SCL_CHIP_VENDOR_CONFIG, &scl);

    memcpy(&scl, dml_sens_cali_config, sizeof(scl));
    CWM_SettingControl(SCL_SENS_CALI_CONFIG, &scl);

    customio_listen_pre();
    CWM_LibPostInit(OS_algo_listen);
    customio_listen_after();

#if CWM_AR_RECALL
    CWM_LibPostInitEx();
    cwm_ar_recall_get_timens();
    t_ar_param ar_param =
    {
        .ar_walk_id = 1009,
        .ar_run_id = 1009,
        .ar_biking_id = 3001,
        .ar_rowing_id = 6001,
        .ar_elliptical_id = 6001,
    };
    cwm_ar_init(&ar_param);
#endif


    memset(&scl, 0, sizeof(scl));
    memcpy(&scl, dml_log_config, sizeof(scl));
    CWM_SettingControl(SCL_LOG, &scl);

    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = -1;  //全部栏位都会设定
    scl.iData[5] = 0;   // acc_noice ,默认为 2600
    scl.iData[7] = 0;   // acc_bias  ，默认为 200000
    CWM_SettingControl(SCL_SENS_CALI_SET_A, &scl);

    CWM_Sensor_Enable(IDX_ALGO_SENS_CALIBRATION);
    CWM_Sensor_Enable(IDX_REQUEST_SENSOR);
    CWM_Dml_LibInit();

    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    CWM_SettingControl(SCL_DML_GET_LIB_INFO, &scl);
    cwm_app_debug("[algo] dml version:%d.%d.%d.%d product:%d model:%d\n", scl.iData[1], scl.iData[2],
                  scl.iData[3], scl.iData[4], scl.iData[5], scl.iData[6]);
    algo_dev_info.dml_info.dml_version[0] = scl.iData[1];
    algo_dev_info.dml_info.dml_version[1] = scl.iData[2];
    algo_dev_info.dml_info.dml_version[2] = scl.iData[3];
    algo_dev_info.dml_info.dml_version[3] = scl.iData[4];
    algo_dev_info.dml_info.dml_version[4] = scl.iData[5];
    algo_dev_info.dml_info.dml_version[5] = scl.iData[6];

    memset(&scl, 0, sizeof(scl));
    memcpy(&scl, dml_log_debug_config, sizeof(scl));
    CWM_SettingControl(SCL_DML_DEBUG, &scl);

    char chipInfo[64];
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = 1;
    scl.iData[2] = (int)chipInfo;
    scl.iData[3] = sizeof(chipInfo);
    scl.iData[4] = 0;
    scl.iData[5] = 0;
    scl.iData[6] = 0;
    CWM_SettingControl(SCL_GET_CHIP_INFO, &scl);

    cwm_app_debug("[algo] have_security = %d.%d ret_buff_size = %d  chipInfo = %s\n", scl.iData[5],
                  scl.iData[6], scl.iData[4], chipInfo);
    cwm_app_debug("[algo] chip_settings = %d, %d, %d\n", scl.iData[9], scl.iData[10], scl.iData[11]);
    if (scl.iData[5] == 1)
    {
        algo_dev_info.dml_info.has_key = 1;
    }
    else
    {
        algo_dev_info.dml_info.has_key = 0;
    }

    CWM_Dml_sensorDataOutput_register(customio_hw_sensor_data_out);

    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = defautl_odr;
    CWM_SettingControl(SCL_ALGO_PROC_CONFIG, &scl);

    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = 1;        // enable DT calib
    CWM_SettingControl(SCL_INPUT_DT_CONFIG, &scl);

    memcpy(&scl, dml_hw_config, sizeof(scl));
    CWM_SettingControl(SCL_DML_DRV_HW_CONFIG, &scl);

    memcpy(&scl, dml_drv_init, sizeof(scl));
    CWM_SettingControl(SCL_DML_DRV_INIT, &scl);

    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    CWM_SettingControl(SCL_DML_GET_INITED_LIST, &scl);
    cwm_app_debug("[algo] DML find device = %d\n",
                  scl.iData[2]);  //找到硬體的數量    (-1: 未初始化, 0: 失敗, 1 - 8)
    for (i = 0; i < scl.iData[2]; i ++)
    {
        cwm_app_debug("[algo] DML device = [%d]: hw_id=%d hw_attr=%d\n", i, scl.iData[6 + i * 2],
                      scl.iData[7 + i * 2]);
    }

    memcpy(&scl, dml_ag_config, sizeof(scl));
    CWM_SettingControl(SCL_DML_DRV_AG_CONFIG, &scl);

    algo_set_odr(scl.iData[8]);
    algo_dev_info.dml_info.acc_rang = dml_ag_config[9];
    algo_dev_info.dml_info.gyro_rang = dml_ag_config[10];

    memcpy(&scl, dml_mag_config, sizeof(scl));
    CWM_SettingControl(SCL_DML_DRV_M_CONFIG, &scl);

    /*开机 sensor 默认关闭*/
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] =
        1;   //[in/out] enable_method (default: manual_enable)   (0: default, 1: manual_enable, 2: auto_enable, -1: 取得參數值)
    scl.iData[2] =
        0;   // [in/out] manual_enable_sensorData (default: all_off) ( bit control 0: manual_all_off, 1: acc, 2: gyro, 4: mag, 8: acc_temp)
    CWM_SettingControl(SCL_DML_DRV_ENABLE, &scl);

    algo_datetime_input();

    //sleep config
    memcpy(&scl, dml_sleep_config, sizeof(scl));
    CWM_SettingControl(SCL_SLEEP_CONFIG, &scl);

    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = 0;
    scl.iData[4] = 0;   //解除久坐等级
    CWM_SettingControl(SCL_SEDENTARY, &scl);

    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = 0;
    CWM_SettingControl(SCL_SET_INACTIVITY_MODE, &scl);
    //AR回溯的SENS_REQ OUTPUT
    memset(&scl, 0, 16 * sizeof(int32_t));
    scl.iData[0] = 1;
    scl.iData[11] = 1;
    CWM_SettingControl(SCL_WI_SENS_REQ_CFG, &scl);

    algo_set_1001(1);

#if (CWM_HANDUPDOWN_Split == 0)
    memcpy(&scl, dml_hand_updown_config, sizeof(scl));
    CWM_SettingControl(SCL_HAND_UPDOWN_CONFIG, &scl);
    CWM_Sensor_Enable(9);
#endif

    /*开机默认开启算法*/
    // CWM_Sensor_Enable(IDX_ACCEL);
    // CWM_Sensor_Enable(IDX_HEARTRATE);
    // CWM_Sensor_Enable(IDX_MAG);
    /*fae 经验：前面设置了 關閉所有算法，这里开启 22 算法也不会输出*/
    CWM_Sensor_Enable(22);

    if (sleep_merge_en)
    {
        sleep_merge_init();
        sleep_merge_set_config(sleep_start[0], sleep_start[1], sleep_start[2], sleep_start[3],
                               sleep_wake_timeout);
    }
}

////////////////////////////////////////////////////////////////外部接口相关////////////////////////////////////////////////////////
struct dml_info *get_dml_info(void)
{
    return &algo_dev_info.dml_info;
}
uint16_t algo_get_odr(void)
{
    return algo_dev_info.dml_info.odr;
}
void algo_set_odr(uint16_t odr)
{
    if (odr)
    {
        algo_dev_info.dml_info.odr = odr;
    }
    else
    {
        algo_dev_info.dml_info.odr = 26;
    }
}

void algo_init(void)
{
    memset((uint8_t *)&algo_dev_info, 0, sizeof(algo_dev_info));

    algo_read_mag_hard_cali_from_flash();

    cwm_app_debug("[algo]config version %s\n", ALGO_CONFIG_VERSION);
    algo_dev_info.dml_info.fw_version = ALGO_CONFIG_VERSION;

    dml_algo_init();
}
void algo_data_input(void)
{
    algo_agmb_input();
}
void algo_sensor_manual_ctr(void)
{
    cwm_app_debug("[algo]sensor:0x%0x\n", sensor_manual_ctr);

    /*开关 acc、gyro、mag*/
    static uint8_t agm_en_back = 0;
    uint8_t agm_en = 0x000000ff & sensor_manual_ctr & (1 + 2 + 4);
    if (agm_en_back != agm_en)
    {
        agm_en_back = agm_en;

        struct sensor_setting_t setting;
        setting.odr = defautl_odr;
        setting.power_mode = SENSOR_DEFAULT;
        setting.acc_range = dml_ag_config[9];
        setting.gyro_range = dml_ag_config[10];
        set_sensor_agm(agm_en, &setting);
    }

    /*开关 baro*/
    static uint8_t baro_en_back = 0;
    uint8_t baro_en = 0;
    if (0x000000ff & sensor_manual_ctr & (8))
    {
        baro_en = 1;
    }
    if (baro_en_back != baro_en)
    {
        baro_en_back = baro_en;
        customio_baro_onoff(baro_en);
    }

    /*开关 gps*/
    static uint8_t gps_en_back = 0;
    uint8_t gps_en = 0;
    if (0x000000ff & sensor_manual_ctr & (64))
    {
        gps_en = 1;
    }
    if (gps_en_back != gps_en)
    {
        gps_en_back = gps_en;
        customio_gps_onoff(gps_en);
    }
}
void algo_data_handle(void)
{
    if (sleep_merge_en)
    {
        /*获取睡眠拼接结果*/
        sleep_merge_output();
    }

#if CWM_DATA_PRE_HANDLE
    /*算法结果传给客户*/
    algo_res_read();
#endif

    /*sensor 开关*/
    algo_sensor_manual_ctr();
}

void algo_log_debug_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_log_debug_ctl ctr = %d\n", ctr);
    SettingControl_t scl;
    if (ctr)
    {
        memset(&scl, 0, sizeof(scl));
        memcpy(&scl, dml_log_config, sizeof(scl));
        CWM_SettingControl(SCL_LOG, &scl);

        memset(&scl, 0, sizeof(scl));
        memcpy(&scl, dml_log_debug_config, sizeof(scl));
        CWM_SettingControl(SCL_DML_DEBUG, &scl);
    }
    else
    {
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        CWM_SettingControl(SCL_LOG, &scl);

        memset(&scl, 0, sizeof(scl));
        memcpy(&scl, dml_log_debug_config, sizeof(scl));
        scl.iData[1] = 0;
        CWM_SettingControl(SCL_DML_DEBUG, &scl);
    }

}

void algo_hand_updown_ctl(uint32_t ctr)
{
#if (CWM_HANDUPDOWN_Split == 0)
    cwm_app_debug("[algo]algo_hand_updown_ctl = %d\n", ctr);
    if (ctr)
    {
        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        memcpy(&scl, dml_hand_updown_config, sizeof(scl));
        CWM_SettingControl(SCL_HAND_UPDOWN_CONFIG, &scl);
        CWM_Sensor_Enable(9);//IDX_ALGO_WATCH_HANDUP
    }
    else
    {
        CWM_Sensor_Disable(9);//IDX_ALGO_WATCH_HANDUP
    }
#endif
}


/*(ctrl:0 resume, 1:pause)*/
void algo_activity_pause_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_activity_pause_ctl = %d\n", ctr);
    if (ctr)
    {
        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 1;
        CWM_SettingControl(SCL_ACTIVITY_PAUSE, &scl);
    }
    else
    {
        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 0;
        CWM_SettingControl(SCL_ACTIVITY_PAUSE, &scl);
    }

}


//AR控制
void algo_ar_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_ar_ctl = %d\n", ctr);
    if (ctr)
    {
        SettingControl_t scl;
        memset(&scl, 0, 16 * sizeof(int32_t));
        scl.iData[0] = 1;
        scl.iData[1] = 1;
        CWM_SettingControl(SCL_WATCH_INTF_RESET, &scl);

        memset(&scl, 0, 16 * sizeof(int32_t));
        memcpy(&scl, dml_ar_config, sizeof(scl));
        CWM_SettingControl(SCL_AR_ALERT_CONFIG, &scl);

        memset(&scl, 0, 16 * sizeof(int32_t));
        scl.iData[0] = 1;
        scl.iData[1] = 1001;
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);

    }
    else
    {
        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        memcpy(&scl, dml_ar_close_config, sizeof(scl));
        CWM_SettingControl(SCL_AR_ALERT_CONFIG, &scl);
    }
}

//跑步机控制
void algo_treadmill_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_treadmill_ctl = %d\n", ctr);
    if (ctr)
    {
        treadmill_flag = 1;

        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        int dml_pedo_treadmill_calibration_config[16] = {0};
        memcpy(dml_pedo_treadmill_calibration_config, dml_pedo_treadmill_config, 64);
        dml_pedo_treadmill_calibration_config[5] = treadmill_set.total_steps;
        dml_pedo_treadmill_calibration_config[6] = treadmill_set.average_step_frequency;
        dml_pedo_treadmill_calibration_config[7] = treadmill_set.uncalibration_disrance;
        dml_pedo_treadmill_calibration_config[8] = treadmill_set.treadmill_ground_truth_distance;

        memcpy(&scl, dml_pedo_treadmill_calibration_config, 64);
        CWM_SettingControl(SCL_PEDO_CONFIG, &scl);

        memcpy(&scl, dml_treadmill_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);

    }
    else
    {
        specific_sport_exit();
    }
}
//跑步机实际距离
void algo_treadmill_calibration(uint32_t calibration_distance)
{
    treadmill_set.treadmill_ground_truth_distance = calibration_distance;
    treadmill_set.total_steps = treadmill_set.temp_total_steps;
    treadmill_set.average_step_frequency = treadmill_set.temp_average_step_frequency;
    treadmill_set.uncalibration_disrance = treadmill_set.temp_uncalibration_disrance;
#if TREADMIL_LAST_NOTIFY
    treadmill_sport_last_notify[2] = treadmill_set.treadmill_ground_truth_distance; //距离
    customio_notify_treadmill_sport(treadmill_sport_last_notify);
#endif
}

//设置泳池长度
void algo_set_pool_length(uint32_t pool_length)
{
    cwm_pool_length = pool_length;
}

//户外跑步控制
void algo_outdoor_running_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_outdoor_running_ctl = %d\n", ctr);
    if (ctr)
    {
        SettingControl_t scl;
        memset(&scl, 0, 16 * sizeof(int32_t));
        scl.iData[0] = 1;
        scl.iData[4] = 1;
        CWM_SettingControl(SCL_PEDO_CONFIG, &scl);

        memset(&scl, 0, 16 * sizeof(int32_t));
        memcpy(&scl, dml_outdoor_running_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);

    }
    else
    {
        specific_sport_exit();
    }
}


//椭圆机控制
void algo_elliptical_trainer_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_elliptical_trainer_ctl = %d\n", ctr);
    if (ctr)
    {
        elliptical_trainer_flag = 1;

        SettingControl_t scl;
        memcpy(&scl, dml_elliptical_trainer_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);
        //enable extra info
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 1;
        CWM_SettingControl(SCL_WM_CONFIG, &scl);

    }
    else
    {
        specific_sport_exit();
    }
}

//划船机控制
void algo_rowing_machine_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_rowing_machine_ctl = %d\n", ctr);
    if (ctr)
    {
        rowing_machine_flag = 1;

        SettingControl_t scl;
        memcpy(&scl, dml_rowing_machine_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);

        //enable extra info
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 1;
        CWM_SettingControl(SCL_WM_CONFIG, &scl);

    }
    else
    {
        specific_sport_exit();
    }
}

//跳绳控制
void algo_rope_skip_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_rowing_machine_ctl = %d\n", ctr);
    if (ctr)
    {
        rope_skip_flag = 1;

        SettingControl_t scl;
        memcpy(&scl, dml_rope_skip_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);
        //enable extra info
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 1;
        CWM_SettingControl(SCL_WM_CONFIG, &scl);

    }
    else
    {
        specific_sport_exit();
    }
}

//户外骑行改为5001
void algo_out_riding_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_out_riding_ctl = %d\n", ctr);
    if (ctr)
    {
        SettingControl_t scl;
        //骑行设置
        memset(&scl, 0, 16 * sizeof(int32_t));
        scl.iData[0] = 1;
        scl.iData[1] = 1;
        CWM_SettingControl(SCL_BIKING_CONFIG, &scl);

        memcpy(&scl, dml_outdoor_riding_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);

    }
    else
    {
        specific_sport_exit();
    }
}

void algo_freetraining_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_freetraining_ctl = %d\n", ctr);
    if (ctr)
    {
        SettingControl_t scl;
        memset(&scl, 0, sizeof(scl));
        memcpy(&scl, dml_freetraining_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);
    }
    else
    {

        specific_sport_exit();
    }
}
//绝对静止
void algo_abs_static_config_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_abs_static_config_ctl = %d\n", ctr);
    if (ctr)
    {
        SettingControl_t scl;

        memset(&scl, 0, 16 * sizeof(int32_t));
        memcpy(&scl, dml_abs_static_config, sizeof(scl));
        CWM_SettingControl(SCL_ABS_STATIC_CONFIG, &scl);
        CWM_Sensor_Enable(IDX_ALGO_ABSOLUTE_STATIC);
    }
    else
    {
        CWM_Sensor_Disable(IDX_ALGO_ABSOLUTE_STATIC);
    }
}

//户外跑步轨迹1009
void algo_outdoor_running_tracking_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_outdoor_running_tracking_ctl = %d\n", ctr);

    if (ctr)
    {
        SettingControl_t scl;
        //extra info
        out_run_tracking_flag = 1;
        memset(&scl, 0, 16 * sizeof(int32_t));
        scl.iData[0] = 1;
        scl.iData[4] = 1;
        CWM_SettingControl(SCL_PEDO_CONFIG, &scl);

        memset(&scl, 0, 16 * sizeof(int32_t));
        memcpy(&scl, dml_outdoor_running_tracking_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);
    }
    else
    {
        specific_sport_exit();
    }
}


//泳池游泳
void algo_swim_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_swim_ctl = %d\n", ctr);

    if (ctr)
    {
        SettingControl_t scl;

        set_mag_cali_hard();

        memcpy(&scl, dml_swim_pool_length_config, sizeof(scl));
        scl.iData[1] = cwm_pool_length;
        CWM_SettingControl(SCL_SWIM_CONFIG, &scl);

        memset(&scl, 0, 16 * sizeof(int32_t));
        memcpy(&scl, dml_swim_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);
    }
    else
    {
        SettingControl_t scl;

        get_mag_cali_hard();

        memset(&scl, 0, 16 * sizeof(int32_t));
        scl.iData[0] = 1;
        CWM_SettingControl(SCL_REQ_SWIM_EXIT, &scl);

        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        CWM_SettingControl(SCL_REQ_ACTIVITY_EXIT, &scl);

        memset(&scl, 0, 16 * sizeof(int32_t));
        scl.iData[0] = 1;
        scl.iData[1] = 1001;
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);
    }
}

//设置开放水域分段长度
void algo_set_open_swim_length(uint32_t open_swim_length)
{
    cwm_open_swim_lenfgth = open_swim_length;
}

//开放水域游泳
void algo_open_water_swim_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_open_water_swim_ctl = %d\n", ctr);

    if (ctr)
    {
        SettingControl_t scl;

        set_mag_cali_hard();

        memcpy(&scl, dml_open_water_swim_length_config, sizeof(scl));
        scl.iData[4] = cwm_open_swim_lenfgth;
        CWM_SettingControl(SCL_SWIM_CONFIG, &scl);

        memset(&scl, 0, 16 * sizeof(int32_t));
        memcpy(&scl, dml_open_water_swim_config, sizeof(scl));
        CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);
    }
    else
    {
        SettingControl_t scl;

        get_mag_cali_hard();

        memset(&scl, 0, 16 * sizeof(int32_t));
        scl.iData[0] = 1;
        CWM_SettingControl(SCL_REQ_SWIM_EXIT, &scl);
        specific_sport_exit();
    }
}





//专项运动退出，清除标志位，主要用于return_1001接口使用
void specific_sport_exit(void)
{
    SettingControl_t scl;
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    CWM_SettingControl(SCL_REQ_ACTIVITY_EXIT, &scl);

#if CWM_AR_RECALL
    CWM_SportFromAR(0);
#endif

    //输出心率区间
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    CWM_SettingControl(SCL_ACT_ZONE_TIME, &scl);
    cwm_zone_time.mhr_zone_1 = scl.iData[1];
    cwm_zone_time.mhr_zone_2 = scl.iData[2];
    cwm_zone_time.mhr_zone_3 = scl.iData[3];
    cwm_zone_time.mhr_zone_4 = scl.iData[4];
    cwm_zone_time.mhr_zone_5 = scl.iData[5];
    cwm_zone_time.mhr_zone_6 = scl.iData[6];
    customio_notify_zone_time(cwm_zone_time.mhr_zone_1, cwm_zone_time.mhr_zone_2,
                              cwm_zone_time.mhr_zone_3, cwm_zone_time.mhr_zone_4, cwm_zone_time.mhr_zone_5,
                              cwm_zone_time.mhr_zone_6);


    memset(&scl, 0, 16 * sizeof(int32_t));
    scl.iData[0] = 1;
    scl.iData[1] = 1001;
    CWM_SettingControl(SCL_SET_ACTIVITY_MODE, &scl);

    treadmill_flag = 0;
    elliptical_trainer_flag = 0;
    rope_skip_flag = 0;
    rowing_machine_flag = 0;
    out_run_tracking_flag = 0;
}

void algo_return_1001(void)
{
    specific_sport_exit();
}

void algo_sleep_ctl(uint32_t ctr)
{
    cwm_app_debug("[algo]algo_sleep_ctl = %d\n", ctr);
    SettingControl_t scl;

    switch (ctr)
    {
    case E_ALGO_SLEEP:
        {
            algo_datetime_input();

            //sleep config
            memcpy(&scl, dml_sleep_config, sizeof(scl));
            CWM_SettingControl(SCL_SLEEP_CONFIG, &scl);

            memcpy(&scl, dml_sleep_inactivity_config, sizeof(scl));
            CWM_SettingControl(SCL_INACTIVITY_CONFIG, &scl);

            memcpy(&scl, dml_sleep_set_inactivity_config, sizeof(scl));
            CWM_SettingControl(SCL_SET_INACTIVITY_MODE, &scl);

#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
            scl_cfg_mem_free(0, SCL_SET_INACTIVITY_MODE);
            scl_cfg_t scl_cfg;
            scl_cfg.cid = SCL_SET_INACTIVITY_MODE;
            scl_cfg.type = 0;
            cwm_app_debug("SLEEPscl.iData[0]=%d\n", scl.iData[0]);
            cwm_app_debug("SLEEPscl.iData[1]=%d\n", scl.iData[1]);
            memcpy(&scl_cfg.data, &scl, 64);
            extern void app_scl_cfg_save(scl_cfg_t *p);
            app_scl_cfg_save(&scl_cfg);
#endif
        }
        break;

    case E_ALGO_NAP: //小睡久坐
        {
            algo_datetime_input();

            //sleep config
            memcpy(&scl, dml_nap_config, sizeof(scl));
            CWM_SettingControl(SCL_SLEEP_CONFIG, &scl);

            memcpy(&scl, dml_nap_inactivity_config, sizeof(scl));
            CWM_SettingControl(SCL_INACTIVITY_CONFIG, &scl);

            memcpy(&scl, dml_nap_set_inactivity_config, sizeof(scl));
            CWM_SettingControl(SCL_SET_INACTIVITY_MODE, &scl);

#if defined(SIM_CUSTOMIO_FOR_TEST) && (SIM_CUSTOMIO_FOR_TEST)
            scl_cfg_mem_free(0, SCL_SET_INACTIVITY_MODE);
            scl_cfg_t scl_cfg;
            scl_cfg.cid = SCL_SET_INACTIVITY_MODE;
            scl_cfg.type = 0;
            cwm_app_debug("NAPscl.iData[0]=%d\n", scl.iData[0]);
            cwm_app_debug("NAPscl.iData[1]=%d\n", scl.iData[1]);
            memcpy(&scl_cfg.data, &scl, 64);
            extern void app_scl_cfg_save(scl_cfg_t *p);
            app_scl_cfg_save(&scl_cfg);
#endif
        }
        break;
    case E_ALGO_SLEEP_DIS:
        memset(&scl, 0, sizeof(scl));
        scl.iData[0] = 1;
        scl.iData[1] = 0;
        CWM_SettingControl(SCL_SET_INACTIVITY_MODE, &scl);
        break;
    }
}

void algo_force_req_sleep_data(void)
{
    cwm_app_debug("[algo]algo_force_req_sleep_data\n");

    /*强制睡眠输出数据，並中止此段睡眠偵測
    fae 反馈用户场景：客户查看UI睡眠数据时是清醒状态，所以睡眠必须中断。
    */
    SettingControl_t scl;
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = 1;
    CWM_SettingControl(SCL_REQ_SLEEPING_DATA, &scl);
}

void algo_set_datetime(uint8_t *data)
{
    struct algo_datetime *date = (struct algo_datetime *)data;

    SettingControl_t scl;
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = date->year;
    scl.iData[2] = date->mon;
    scl.iData[3] = date->day;
    scl.iData[4] = date->hour;
    scl.iData[5] = date->min;
    scl.iData[6] = date->sec;
    CWM_SettingControl(SCL_DATE_TIME, &scl);

    cwm_app_debug("[algo]algo_set_datetime:%d-%d-%d:%d:%d:%d\n",
                  date->year,
                  date->mon,
                  date->day,
                  date->hour,
                  date->min,
                  date->sec);
}
void algo_set_user_infio(uint8_t *data)
{
    struct algo_user_info *tmp = (struct algo_user_info *)data;
    SettingControl_t scl;
    memset(&scl, 0, sizeof(scl));
    scl.iData[0] = 1;
    scl.iData[1] = tmp->age;
    scl.iData[2] = tmp->gender;
    scl.iData[3] = tmp->weiht;
    scl.iData[4] = tmp->height;
    scl.iData[5] = tmp->hand;
    CWM_SettingControl(SCL_USER_INFO, &scl);

    cwm_app_debug("[algo]algo_set_user_infio:age(%d)gender(%s)weight(%d)height(%d)hand(%s)\n",
                  tmp->age,
                  (tmp->gender == 0) ? "woman" : "man",
                  tmp->weiht,
                  tmp->height,
                  (tmp->hand == 0) ? "left" : "right");
}

void algo_set_gps_data(uint8_t *data)
{
    struct algo_gps_data *tmp = (struct algo_gps_data *)data;
    CustomSensorData csd;
    memset(&csd, 0, sizeof(CustomSensorData));
    csd.sensorType = CUSTOM_GNSS;
    csd.dData[0] = tmp->latitude; //latitude (degrees)  (range: -90~90)
    csd.dData[1] = tmp->longitude; //longitude (degrees) (range: -180~180)
    csd.dData[2] = tmp->altitude;
    csd.dData[3] = tmp->bearing;
    csd.dData[4] = tmp->speed;
    csd.dData[5] = tmp->Horizontal_Accuracy;
    csd.dData[6] = tmp->Vertical_Accuracy;
    csd.dData[7] = tmp->HDOP;
    csd.dData[8] = tmp->VDOP;
//  csd.dData[9]=tmp->number_of_satellites;
//  csd.dData[10]=tmp->NMEA_latitude;
//  csd.dData[11]=tmp->NMEA_longitud;
    CWM_CustomSensorInput(&csd);

    cwm_app_debug("[app_gps]gps_latitude=%f\n", tmp->latitude);
    cwm_app_debug("[app_gps]gps_longitude=%f\n", tmp->longitude);
}




void algo_set_dev_status(uint32_t status)
{
    static uint32_t pre_status = 0xffffffff;

    uint8_t bits = status / 2;
    uint8_t off = status % 2;

    if (off)
    {
        algo_dev_info.dev_status &= ~(((uint32_t)1) << bits);
    }
    else
    {
        algo_dev_info.dev_status |= ((uint32_t)1) << bits;
    }

    if (pre_status != algo_dev_info.dev_status)
    {
        pre_status = algo_dev_info.dev_status;

        CustomSensorData csd;
        memset(&csd, 0, sizeof(CustomSensorData));
        csd.sensorType = CUSTOM_ON_CHARGING;
        csd.fData[0] = algo_dev_info.dev_status;
        CWM_CustomSensorInput(&csd);
        cwm_app_debug("[algo]algo_set_dev_status = 0x%0x\n", algo_dev_info.dev_status);
    }
}
void algo_set_wear_status(uint32_t status)
{
    CustomSensorData csd = {0};

    csd.sensorType = CUSTOM_OFFBODY_DETECT;
    csd.fData[0] = status;
    CWM_CustomSensorInput(&csd);
    // cwm_app_debug("[algo]algo_set_wear_status %d\n",(uint16_t)csd.fData[0]);
}
void algo_set_hr_value(uint32_t value)
{
    CustomSensorData csd = {0};

    csd.sensorType = CUSTOM_HEARTRATE;
    csd.fData[0] = value;
    CWM_CustomSensorInput(&csd);
}
void algo_reset_daily_infio(void)
{
    cwm_app_debug("[algo]algo_reset_daily_infio\n");

    /*旧的算法使用接口 SCL_PEDO_RESET */
    /*新的算法使用接口 SCL_RESET_DAILY_INFO*/
    SettingControl_t scl;
    memset(&scl, 0, sizeof(scl));
    CWM_SettingControl(4, &scl);

    /*p7xx 版本只支持下面的做法*/
    // algo_set_1001(0);
    // algo_set_1001(1);
}



void algo_save_before_poweroff(void)
{
    cwm_app_debug("[algo]algo_save_before_poweroff\n");

    get_mag_cali_hard();
    save_mag_cali_hard();
}

