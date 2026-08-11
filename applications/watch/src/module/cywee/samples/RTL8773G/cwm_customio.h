#ifndef __CWM_CUSTOMIO_H__
#define __CWM_CUSTOMIO_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "cwm_lib.h"
#include "cwm_lib_dml.h"

#define CWM_HANDUPDOWN_Split 0
#define CWM_DATA_PRE_HANDLE  0
#define CWM_AR_RECALL   0

extern int cwm_app_debug(const char *format, ...);
void cwm_task_init_CRITICAL(void);
void cwm_taskENTER_CRITICAL(uint8_t id);
void cwm_taskEXIT_CRITICAL(uint8_t id);

extern os_api customio_os_api;
extern const uint16_t defautl_odr;
extern const uint8_t baro_tmp_suppot;
extern const uint8_t sleep_merge_en;
extern const uint8_t sleep_start[4];
extern const uint16_t sleep_wake_timeout;

extern int dml_vendor_config[16];
extern uint8_t sleep_nap_state;
extern const int dml_sens_cali_config[16];
extern const int dml_hw_config[16];
extern const int dml_ag_config[16];
extern const int dml_mag_config[16];
extern const int dml_mag_soft_iron[9];
extern const int dml_log_config[16];
extern const int dml_log_debug_config[16];
extern const int dml_drv_init[16];
extern const int dml_sleep_config[16];
extern const int dml_nap_config[16];
extern const int dml_sleep_inactivity_config[16];  /*SCL_INACTIVITY_CONFIG*/
extern const int dml_nap_inactivity_config[16];  /*SCL_INACTIVITY_CONFIG*/
extern const int dml_sleep_set_inactivity_config[16];  /*SCL_SET_INACTIVITY_MODE*/
extern const int dml_nap_set_inactivity_config[16];  /*SCL_SET_INACTIVITY_MODE*/
extern const int dml_activity_config[16];
extern const int dml_pedo_config[16];
extern const int dml_hand_updown_config[16];
extern const int dml_ar_config[16];
extern const int dml_ar_close_config[16];
extern const int dml_elliptical_trainer_config[16];
extern const int dml_treadmill_config[16];//跑步机
extern const int dml_pedo_treadmill_config[16];//跑步机距离校正
extern const int dml_rope_skip_config[16];//跳绳
extern const int dml_rowing_machine_config[16];//划船机
extern const int dml_outdoor_running_config[16];//户外跑步
extern const int dml_outdoor_riding_config[16];//户外骑行
extern const int dml_auto_pause_config[16];//开启自动暂停/恢复
extern const int dml_auto_pause_close_config[16];//关闭自动暂停/恢复
extern const int dml_freetraining_config[16];//自由训练
extern const int dml_abs_static_config[16];/*绝对静止 SCL_ABS_STATIC_CONFIG*/
extern const int dml_swim_config[16];//三轴游泳
extern const int dml_swim_pool_length_config[16];//泳池长度
extern const int dml_open_water_swim_config[16];//开放水域游泳 SCL_SET_ACTIVITY_MODE*/
extern const int
dml_open_water_swim_length_config[16];/*开放水域分段长度 SCL_SET_ACTIVITY_MODE*/
extern const int dml_outdoor_running_tracking_config[16];//户外跑步轨迹


void customio_listen_pre(void);
void customio_listen_after(void);

void customio_read_flash_mag_hard_cali(uint8_t *data, uint32_t len);
void customio_save_flash_mag_hard_cali(uint8_t *data, uint32_t len);
void customio_baro_onoff(uint8_t onoff);
void customio_gps_onoff(uint8_t onoff);

void customio_init(void);
void customio_get_datetime(uint8_t *data);
void customio_hw_sensor_data_out(const dml_sensorData_t *pSdo);
void customio_notify_handup(void);
void customio_notify_handdown(void);
void customio_notify_sedentary(void);//算法输出触发久坐消息
void customio_notify_normal_steps(uint32_t steps);/*计步单位：步数*/
void customio_notify_normal_distance(uint32_t meters);/*距离单位：meters*/
void customio_notify_cal(float kcal, float BMR_kcal);/*日常路里单位：Kcal*/
void customio_notify_sleep_status(uint8_t data_type, uint8_t id);/*通知当前睡眠/小睡状态*/
void customio_notify_sleep_data(uint16_t idx, uint8_t data_type, uint8_t id,
                                uint8_t *data); /*睡眠数据*/
void customio_notify_ar_sport(uint8_t ar_alert_type);//AR种类输出
void customio_notify_elliptical_trainer_sport(float *f);//椭圆机数据输出
void customio_notify_treadmill_sport(float *f);//跑步机数据输出
void customio_notify_outdoor_running_sport(float *f);//算法输出户外健走/跑步数据
void customio_notify_out_riding_sport(float *f);//算法输出骑行数据(卡路里)
void customio_notify_set_rowing_machine_sport(float *f);//算法输出划船机数据
void customio_notify_rope_skipping_sport(float *f);//算法输出跳绳数据
void customio_notify_out_free_training_sport(float *f);/*算法输出自由训练数据*/
void customio_notify_abs_static(void);//算法输出触发绝对静止消息
void customio_notify_swim_data(float *f);/*算法输出游泳数据*/
void customio_notify_out_run_tracking(float *f);/*算法输出个户外跑步轨迹数据*/
void customio_notify_Sens_Reg_output(int32_t sens_reg);
void customio_notify_zone_time(float mhr_zone_1, float mhr_zone_2, float mhr_zone_3,
                               float mhr_zone_4, float mhr_zone_5, float mhr_zone_6);
void customio_notify_open_swim_data(float *f);

#ifdef __cplusplus
}
#endif

#endif



