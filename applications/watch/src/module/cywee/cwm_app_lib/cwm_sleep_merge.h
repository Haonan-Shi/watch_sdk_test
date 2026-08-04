#ifndef __CWM_SLEEP_MERGE_H__
#define __CWM_SLEEP_MERGE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "cwm_port.h"

#define SLEEPMERGE_OUTPUTNAP_CumulativeResults 0

/*
获取睡眠/小睡起始时间：
uint8_t sshour：睡眠起始时间：小时
uint8_t ssmin：睡眠起始时间：分钟
uint8_t nshour：小睡起始时间：小时
uint8_t nsmin：小睡起始时间：分钟
*/
void sleep_merge_set_config(uint8_t sshour, uint8_t ssmin, uint8_t nshour, uint8_t nsmin,
                            uint16_t wake_timeout);
/*
设置睡眠/小睡起始时间：
uint8_t sshour：睡眠起始时间：小时
uint8_t ssmin：睡眠起始时间：分钟
uint8_t nshour：小睡起始时间：小时
uint8_t nsmin：小睡起始时间：分钟
*/
void sleep_merge_get_config(uint8_t *sshour, uint8_t *ssmin, uint8_t *nshour, uint8_t *nsmin,
                            uint16_t *wake_timeout);
/*
sleep merge 初始化
*/
void sleep_merge_init(void);
/*
f：算法数据指针，指向 float data[16]
*/
void sleep_merge_input(float *f);
/*
内部会调用 customio_notify_sleep_data 输出拼接数据
*/
void sleep_merge_output(void);


#ifdef __cplusplus
}
#endif


#endif


