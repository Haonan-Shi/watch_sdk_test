#ifndef __CWM_CONFIG_H__
#define __CWM_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "cwm_port.h"


void algo_init(void);
uint16_t algo_get_odr(void);
void algo_data_input(void);
void algo_data_handle(void);
void algo_log_debug_ctl(uint32_t ctr);
void algo_hand_updown_ctl(uint32_t ctr);
void algo_sleep_ctl(uint32_t ctr);
void algo_force_req_sleep_data(void);
void algo_set_datetime(uint8_t *data);
void algo_set_user_infio(uint8_t *data);
void algo_set_dev_status(uint32_t status);
void algo_set_wear_status(uint32_t status);
void algo_set_hr_value(uint32_t value);
void algo_reset_daily_infio(void);
void algo_ar_ctl(uint32_t ctr);
void algo_elliptical_trainer_ctl(uint32_t ctr);
void algo_treadmill_ctl(uint32_t ctr);
void algo_treadmill_calibration(uint32_t calibration_distance);
void algo_outdoor_running_ctl(uint32_t ctr);
void algo_rowing_machine_ctl(uint32_t ctr);
void algo_rope_skip_ctl(uint32_t ctr);
void algo_out_riding_ctl(uint32_t ctr);
void algo_return_1001(void);
void algo_activity_pause_ctl(uint32_t ctr);
void algo_freetraining_ctl(uint32_t ctr);
void algo_abs_static_config_ctl(uint32_t ctr);
void algo_swim_ctl(uint32_t ctr);
void algo_set_pool_length(uint32_t pool_length);
void algo_open_water_swim_ctl(uint32_t ctr);
void algo_set_open_swim_length(uint32_t open_swim_length);
void algo_set_gps_data(uint8_t *data);
void algo_outdoor_running_tracking_ctl(uint32_t ctr);
void specific_sport_exit(void);
#ifdef __cplusplus
}
#endif

#endif




