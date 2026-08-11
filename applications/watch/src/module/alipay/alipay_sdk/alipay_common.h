#ifndef __ALIPAY_COMMON_H__
#define __ALIPAY_COMMON_H__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define PARAM_IN
#define PARAM_OUT
#define PARAM_INOUT

#define INPUT_PARAM
#define OUTPUT_PARAM
#define INOUT_PARAM


#define __alipay_depricated__
#define __readonly
#define __readwrite

#ifdef __cplusplus
#define BEGIN_C_DECL extern "C" {
#else
#define BEGIN_C_DECL
#endif

#ifdef __cplusplus
#define END_C_DECL }
#else
#define END_C_DECL
#endif

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#if 0 // TODO precheck definition conflict
typedef unsigned short     uint16_t;
typedef unsigned char      uint8_t;
typedef long long          int64_t;
typedef unsigned long long uint64_t;
typedef int                int32_t;
typedef unsigned int       uint32_t;
#endif

typedef int32_t retval_e;
#define ALIPAY_RV_OK 0 // 成功，其他值为失败
#define ALIPAY_RV_WRONG_PARAM 19
#define ALIPAY_RV_HAL_ERROR 57 // HAL层错误码

/**
 * 绑定状态相关
 */
typedef int32_t binding_status_e;
#define ALIPAY_STATUS_BINDING_FAIL  0    // 绑定失败
#define ALIPAY_STATUS_UNBINDED      1    // 解绑
#define ALIPAY_STATUS_START_BINDING 2    // 开始绑定
#define ALIPAY_STATUS_BINDING_OK    0xa5 // 绑定成功(已绑定)
#define ALIPAY_STATUS_UNKNOWN       0xff // 未知状态

/**
 * 支付结果相关
 */
#if 0
typedef enum
{
    ALIPAY_PAYMENT_STATUS_SUCCESS          = 0,
    ALIPAY_PAYMENT_STATUS_FAIL             = 1,
    ALIPAY_PAYMENT_STATUS_NOTHING          = 2,
    ALIPAY_PAYMENT_STATUS_UNBIND_BY_USER   = 3,
    ALIPAY_PAYMENT_STATUS_DISABLED_BY_USER = 4,
    ALIPAY_PAYMENT_STATUS_UNKNOWN          = 5,
    ALIPAY_PAYMENT_STATUS_PROCESSING       = 6,
} alipay_payment_status_e;

typedef enum
{
    ALIPAY_FAIL_RES_AMOUNT_LIMIT       = 0,  //超出设置的支付额度
    ALIPAY_FAIL_RES_BALANCE_NOT_ENOUGH = 1,  //零花钱不足，请存入
    ALIPAY_FAIL_RES_COMMON_FAIL        = 2,  //支付失败
    ALIPAY_FAIL_RES_ACCOUNT_NOT_EXIST  = 3,  //零花钱不存在
    ALIPAY_FAIL_RES_CASE_NOT_SUPPORT   = 4,  //该场景不支持使用智能设备支付,显示“不支持使用该设备进行支付，请使用手机支付宝支付”
} alipay_scan_pay_fail_res_e;
#endif

#if !defined(__cplusplus)
#ifndef bool
typedef unsigned char bool;
#endif
#ifndef false
#define false ((bool)0)
#define true  ((bool)1)
#endif
#endif

#define ALIPAY_WEAK_SYMBOL __attribute__((weak))

#endif
