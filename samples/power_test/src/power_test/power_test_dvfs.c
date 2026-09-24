/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl876x.h"
#include "trace.h"
#include "board.h"
#include "os_mem.h"
#include "app_msg.h"
#include "power_test.h"
#include "pm.h"

#ifdef CONFIG_SOC_SERIES_RTL8773D
#include "dvfs_api.h"
#include "pll.h"
#include "rtl876x_mclk.h"
#include "clock_manager.h"

#define OS_GLOBAL_CONFIG_SIZE 64
typedef struct t_heap_cfg
{
    uint32_t start_offset : 17; //(0 ~ 0x400000)
    uint32_t size : 15;//(0 ~ 0x100000)
} T_HEAP_CFG;
typedef union
{
    uint8_t value[OS_GLOBAL_CONFIG_SIZE];

    struct
    {
        uint32_t getStackHighWaterMark : 1;         /* 0 for release version, 1 for debug version */
        uint32_t checkForStackOverflow : 1;         /* 0 for release version, 1 for debug version */
        uint32_t printAllLogBeforeEnterLowpower : 1;    /* 0 for release version, 1 for debug version */
        uint32_t dumpMemoryWhenHardFault : 1;       /* 0 for release version, 1 for debug version */
        uint32_t dumpMemoryUsage : 1;               /* 0 for release version, 1 for debug version */
        uint32_t enableASSERT: 1;                   /* 0 for release version, 1 for debug version */
        uint32_t mallocFailAssert: 1;               /* default = 0 */
        uint32_t timerCreatFailAssert : 1;          /* default = 0 */
        uint32_t cpu_sleep_en: 1;                   /* default = 0 */
        uint32_t ram_always_on : 1;                 /* config all ram always on */
        uint32_t heap_contiguousy: 1;
        uint32_t trace_profiling: 1;
        uint32_t beforeDumpwdgTimeout: 4;              /* default = 8 (8s) */
        uint32_t maxTaskCountForDebug: 5;           /* default = 8 */
        uint32_t stack_ram_type: 3;                 /* default data on */

        uint32_t timerMaxNumber : 8;                /* default = 0x30 */
        uint32_t timerQueueLength : 8;              /* default = 0x20 */

        uint32_t wdgTimeoutMs : 18;                 /* defualt 4000ms, max 262143ms*/
        uint32_t wdgAonBackup : 1;                  /* defualt 1*/
        uint32_t wdgEnableInRom : 1;                /* 1 for release version, 0 for debug version */
        uint32_t wdgResetInCS : 1;                  /* 0 for release version, 1 for debug version. */
        uint32_t wdgResetInIdle : 1;                /* 1 for release version, 0 for debug version. */
        uint32_t wdgMode : 2;                       /* 0: interrupt CPU,     1: reset all but aon
                                                    2: reset core domain, 3: reset all */

        uint32_t common_sram0_start_addr;
        uint32_t common_sram0_size;
        uint32_t common_sram1_size;
        T_HEAP_CFG heapDataOn;                      //soc mode
        T_HEAP_CFG heapDataOff;
        T_HEAP_CFG heapBufferOn;
        T_HEAP_CFG heapBufferOff;
        T_HEAP_CFG heapCommonOn;
        T_HEAP_CFG heapCommonOff;
        T_HEAP_CFG heapCommon2On;
        T_HEAP_CFG heapCommon2Off;

        uint32_t common2BlockDSPShared;             /* number of 8K sram shared to dsp */

        uint16_t idle_task_stack_size;              /* measured in bytes, default 256 * 4 bytes */
        uint16_t timer_task_stack_size;             /* measured in bytes, default 256 * 4 bytes */
        uint16_t lower_task_stack_size;             /* measured in bytes, default 768 * 4 bytes */
        uint16_t recordMemoryUsage : 1;
        uint16_t reserved : 15;
    };
} __attribute__((packed)) OS_GLOBAL_CONFIG;

extern OS_GLOBAL_CONFIG os_cfg;
#else

typedef union _HEAP_TYPE_MASK
{
    struct
    {
        uint8_t heap_data_on_mask : 1;
        uint8_t heap_data_off_mask : 1;
        uint8_t heap_buffer_on_mask : 1;
        uint8_t heap_buffer_off_mask : 1;
        uint8_t heap_dtcm0_mask : 1;
        uint8_t heap_itcm1_mask : 1;
        uint8_t heap_dsp_share_mask : 1;
        uint8_t reserved : 1;
    };
    uint8_t heap_type_mask;
} HEAP_TYPE_MASK;
typedef struct
{
    uint32_t wdgTimeoutMs : 23;            /* seconds, default = 4s */
    uint32_t wdgMode : 2;            /* 0: interrupt CPU,     1: reset all but aon
                                       2: reset core domain, 3: reset all */
    uint32_t wdgEnableInRom : 1;     /* 1 for release version, 0 for debug version. default = 0 */
    uint32_t wdgResetInCS :  1;     /* 0 for release version, 1 for debug version. default = 0 */
    uint32_t wdgResetInIdle :  1;     /* 1 for release version, 0 for debug version. default = 0 */
    uint32_t wdgAonBackup : 1;
    uint32_t cpu_sleep_en: 1;                   /* default = 0 */
    uint32_t checkForStackOverflow : 1;         /* 0 for release version, 1 for debug version */
    uint32_t printAllLogBeforeEnterLowpower : 1;    /* 0 for release version, 1 for debug version */

    uint8_t dumpMemoryWhenHardFault : 1;       /* 0 for release version, 1 for debug version */
    uint8_t dumpMemoryUsage : 1;               /* 0 for release version, 1 for debug version */
    uint8_t enableASSERT: 1;                   /* 0 for release version, 1 for debug version */
    uint8_t mallocFailAssert : 1;
    uint8_t stack_ram_type: 3;
    uint8_t ram_always_on : 1;          /* config all ram always on */

    uint8_t heap_contiguousy : 1;
    uint8_t enable_malloc_track: 1;
    uint8_t reserved : 6;


    uint8_t  timerMaxNumber;                   /* default = 0x30 */
    uint8_t  timerQueueLength;                 /* default = 0x20 */

    uint32_t heapDataONAddr;
    uint32_t heapDataONSize;
    uint32_t heapBufferONSize;
    uint32_t heapITCM1Addr;
    HEAP_TYPE_MASK heap_mask;

    uint8_t bufferBlockDSPShared;

    uint16_t idle_task_stack_size;             /* measured in bytes, default 256 * 4 bytes */
    uint16_t timer_task_stack_size;            /* measured in bytes, default 256 * 4 bytes */
    uint16_t lower_task_stack_size;            /* measured in bytes, default 768 * 4 bytes */
} OS_GLOBAL_CFG;

extern OS_GLOBAL_CFG os_cfg;

#ifndef CONFIG_SOC_SERIES_RTL8773E
typedef enum
{
    DVFS_NORMAL_VDD                 = 0,
} DVFSVDDType;
typedef enum
{
    DVFS_VDD_1V1                    = 0,
    DVFS_VDD_0V9                    = 1,
} DVFSVDDMode;

typedef enum
{
    DVFS_SUCCESS                    = 0x0,
    DVFS_BUSY                       = 0x1,
    DVFS_VOLTAGE_FAIL               = 0x2,
    DVFS_CONDITION_FAIL             = 0x4,
    DVFS_SRAM_FAIL                  = 0x8,
    DVFS_NOT_SUPPORT                = 0x10,
} DVFSErrorCode;

extern DVFSErrorCode(*dvfs_set_mode)(DVFSVDDType, DVFSVDDMode);
#endif
#endif

extern void (*set_clk_32k_power_in_powerdown)(bool);

#ifndef CONFIG_SOC_SERIES_RTL8773E
void power_test_set_dvfs(uint16_t action, uint8_t *buf)
{
    DVFSErrorCode error_code = DVFS_SUCCESS;

#ifdef CONFIG_SOC_SERIES_RTL8773D
    /* open dsp clock, pro3 can't close when change cpu clock because will access dsp register */
    bool is_dsp_clk_en = SYSBLKCTRL->u_208.BITS_208.r_DSP_CLK_SRC_EN;
    if (is_dsp_clk_en == false)
    {
        SYSBLKCTRL->u_208.BITS_208.r_DSP_CLK_SRC_EN = 1;
    }
#endif
    if (action == POWER_TEST_CMD_SET_DVFS_NORMAL0V9)
    {
        error_code = dvfs_set_mode(DVFS_NORMAL_VDD, DVFS_VDD_0V9);
        APP_PRINT_TRACE1("power test set dvfs normal 0V9 %d", error_code);
    }
#ifdef CONFIG_SOC_SERIES_RTL8773D
    else if (action == POWER_TEST_CMD_SET_DVFS_NORMAL0V8)
    {
        error_code = dvfs_set_mode(DVFS_NORMAL_VDD, DVFS_VDD_0V8);
        APP_PRINT_TRACE1("power test set dvfs normal 0V8 %d", error_code);
    }
    else if (action == POWER_TEST_CMD_SET_DVFS_LOW0V9)
    {
        error_code = dvfs_set_mode(DVFS_LOW_VDD, DVFS_VDD_0V9);
        APP_PRINT_TRACE1("power test set dvfs low 0V9 %d", error_code);
    }
    else if (action == POWER_TEST_CMD_SET_DVFS_LOW0V8)
    {
        error_code = dvfs_set_mode(DVFS_LOW_VDD, DVFS_VDD_0V8);
        APP_PRINT_TRACE1("power test set dvfs low 0V8 %d", error_code);
    }
    else if (action == POWER_TEST_CMD_SET_DVFS_LOW0V7)
    {
        error_code = dvfs_set_mode(DVFS_LOW_VDD, DVFS_VDD_0V7);
        APP_PRINT_TRACE1("power test set dvfs low 0V7 %d", error_code);
    }
    else if (action == POWER_TEST_CMD_SET_DVFS_LOW0V65)
    {
        error_code = dvfs_set_mode(DVFS_LOW_VDD, DVFS_VDD_0V65);
        APP_PRINT_TRACE1("power test set dvfs low 0V65 %d", error_code);
    }
    else if (action == POWER_TEST_CMD_SET_DVFS_LOW0V6125)
    {
        error_code = dvfs_set_mode(DVFS_LOW_VDD, DVFS_VDD_0V6125);
        APP_PRINT_TRACE1("power test set dvfs low 0V6125 %d", error_code);
    }
#else
    else if (action == POWER_TEST_CMD_SET_DVFS_NORMAL1V1)
    {
        error_code = dvfs_set_mode(DVFS_NORMAL_VDD, DVFS_VDD_1V1);
        APP_PRINT_TRACE1("power test set dvfs normal 1V1 %d", error_code);
    }
#endif
    else if (action == POWER_TEST_CMD_SET_DVFS_HIGH)
    {
#ifdef CONFIG_SOC_SERIES_RTL8773D
        DVFSErrorCode error_code2 = DVFS_SUCCESS;
        error_code = dvfs_set_mode(DVFS_LOW_VDD, DVFS_VDD_0V8);
        error_code2 = dvfs_set_mode(DVFS_NORMAL_VDD, DVFS_VDD_0V9);
        APP_PRINT_TRACE2("power test set dvfs high %d %d", error_code, error_code2);
#else
        error_code = dvfs_set_mode(DVFS_NORMAL_VDD, DVFS_VDD_1V1);
        APP_PRINT_TRACE1("power test set dvfs high %d", error_code);
#endif
    }
    else if (action == POWER_TEST_CMD_SET_DVFS_LOW)
    {
#ifdef CONFIG_SOC_SERIES_RTL8773D
        error_code = dvfs_set_mode(DVFS_NORMAL_VDD, DVFS_VDD_0V8);
        APP_PRINT_TRACE1("power test set dvfs low %d", error_code);
#else
        error_code = dvfs_set_mode(DVFS_NORMAL_VDD, DVFS_VDD_0V9);
        APP_PRINT_TRACE1("power test set dvfs low %d", error_code);
#endif
    }
#ifdef CONFIG_SOC_SERIES_RTL8773D
    else if (action == POWER_TEST_CMD_SET_VCORE2_CLOSE)
    {
        extern bool ext_buck_vcore2_disable(void);
        ext_buck_vcore2_disable();
        APP_PRINT_TRACE0("power test close low");
    }
#endif
    else
    {
        APP_PRINT_TRACE0("power test dvfs cmd error");
    }
#ifdef CONFIG_SOC_SERIES_RTL8773D
    if (is_dsp_clk_en == false)
    {
        /* close dsp1 clock */
        SYSBLKCTRL->u_208.BITS_208.r_DSP_CLK_SRC_EN = 0;
    }
#endif

    if (buf != NULL)
    {
        free(buf);
    }
}
#endif

#ifdef CONFIG_SOC_SERIES_RTL8773D
void dvfs_stress_test(void)
{
    static uint16_t action = POWER_TEST_CMD_SET_DVFS_LOW0V6125;
    DVFSVDDMode normalmode = DVFS_VDD_0V9;
    DVFSVDDMode lowmode = DVFS_VDD_0V7;

    normalmode = dvfs_get_mode(DVFS_NORMAL_VDD);
    lowmode = dvfs_get_mode(DVFS_LOW_VDD);

    APP_PRINT_INFO2("run normal %d low %d", normalmode, lowmode);

    if (action == POWER_TEST_CMD_SET_DVFS_LOW0V6125)
    {
        if (lowmode != DVFS_VDD_0V6125)
        {
            if (normalmode == DVFS_VDD_0V9)
            {
                power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_NORMAL0V8, NULL);
            }
            power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_LOW0V6125, NULL);
        }
    }

    if (action == POWER_TEST_CMD_SET_DVFS_LOW0V65)
    {
        if (lowmode != DVFS_VDD_0V65)
        {
            if (normalmode == DVFS_VDD_0V9)
            {
                power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_NORMAL0V8, NULL);
            }
            power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_LOW0V65, NULL);
        }
    }

    if (action == POWER_TEST_CMD_SET_DVFS_LOW0V7)
    {
        if (lowmode != DVFS_VDD_0V7)
        {
            if (normalmode == DVFS_VDD_0V9)
            {
                power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_NORMAL0V8, NULL);
            }
            power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_LOW0V7, NULL);
        }
    }

    if (action == POWER_TEST_CMD_SET_DVFS_LOW0V8)
    {
        if (lowmode != DVFS_VDD_0V8)
        {
            power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_LOW0V8, NULL);
        }
    }

    if (action == POWER_TEST_CMD_SET_DVFS_LOW0V9)
    {
        if (lowmode != DVFS_VDD_0V9)
        {
            if (lowmode < DVFS_VDD_0V8)
            {
                power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_LOW0V8, NULL);
            }
            if (normalmode == DVFS_VDD_0V8)
            {
                power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_NORMAL0V9, NULL);
            }
            power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_LOW0V9, NULL);
        }
    }

    if (action == POWER_TEST_CMD_SET_DVFS_NORMAL0V8)
    {
        if (normalmode != DVFS_VDD_0V8)
        {
            if (lowmode == DVFS_VDD_0V9)
            {
                power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_LOW0V8, NULL);
            }
            power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_NORMAL0V8, NULL);
        }
    }

    if (action == POWER_TEST_CMD_SET_DVFS_NORMAL0V9)
    {
        if (normalmode != DVFS_VDD_0V9)
        {
            if (lowmode < DVFS_VDD_0V8)
            {
                power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_LOW0V8, NULL);
            }
            power_test_set_dvfs(POWER_TEST_CMD_SET_DVFS_NORMAL0V9, NULL);
        }
    }

    if (action == POWER_TEST_CMD_SET_DVFS_NORMAL0V9)
    {
        action = POWER_TEST_CMD_SET_DVFS_LOW0V6125;
    }
    else
    {
        action--;
    }
}
#endif

void power_test_set_cpu_freq(uint16_t action, uint8_t *buf)
{
    uint32_t actual_mhz;
    int32_t ret;

    if (action == POWER_TEST_CMD_CPU_FREQ_625K)
    {
        ret = pm_cpu_freq_set(0, &actual_mhz);
        APP_PRINT_TRACE2("power test set cpu freq 625k %d %d", ret, actual_mhz);
    }
    else if (action == POWER_TEST_CMD_CPU_SLEEP)
    {
        os_cfg.cpu_sleep_en = 1;
        APP_PRINT_TRACE0("power test set cpu sleep enable");
    }
    else if (action == POWER_TEST_CMD_CPU_ACTIVE)
    {
        os_cfg.cpu_sleep_en = 0;
        APP_PRINT_TRACE0("power test set cpu active");
    }
    else if (action == POWER_TEST_CMD_CPU_FREQ_MAX)
    {
        ret = pm_cpu_freq_set(pm_cpu_max_freq_get(), &actual_mhz);
        APP_PRINT_TRACE3("power test set cpu freq max:%d ret:%d acu:%d", pm_cpu_max_freq_get(), ret,
                         actual_mhz);
    }
    else
    {
        ret = pm_cpu_freq_set(action, &actual_mhz);
        APP_PRINT_TRACE3("power test set cpu freq set:%d ret:%d acu:%d", action, ret, actual_mhz);
    }

    if (buf != NULL)
    {
        free(buf);
    }
}

void power_test_set_dsp1_freq(uint16_t action, uint8_t *buf)
{
    uint32_t actual_mhz;
    int32_t ret;

    if (action == POWER_TEST_CMD_DSP1_DISABLE)
    {
        SYSBLKCTRL->u_208.BITS_208.r_DSP_CLK_SRC_EN = 0;
        APP_PRINT_TRACE0("power test dsp power down");
    }
    else
    {
        SYSBLKCTRL->u_208.BITS_208.r_DSP_CLK_SRC_EN = 1;
        ret = pm_dsp1_freq_set(action, &actual_mhz);
        APP_PRINT_TRACE3("power test set dsp1 freq set:%d ret:%d acu:%d", action, ret, actual_mhz);
    }

    if (buf != NULL)
    {
        free(buf);
    }
}
#ifdef CONFIG_SOC_SERIES_RTL8773D
void power_test_set_mclk2(uint16_t action, uint8_t *buf)
{
    T_CLK_SETTING clk_setting;

    if (action == POWER_TEST_CMD_MCLK2_XTAL)
    {
        clk_setting.clock_soure = MCLK2_CLK_XTAL;
        clk_setting.div_setting = MCLK2_DIVIDE_2;
        clk_setting.div_sel = MCLK2_DIV_2;
        MCLK2_SetOutput(P2_4, &clk_setting);
        APP_PRINT_TRACE0("power test mclk2 xtal");
    }
    else if (action == POWER_TEST_CMD_MCLK2_PLL)
    {
        ActiveMux_InitType mux_config =
        {
            .mux_sel = CKO1_PLL2,
            .mux_src = CKO1_PLL2_SRC,
            .mux_enable = true,
            .mux_output_freq = CLK_240MHZ,
        };
        set_active_clock_mux_output(mux_config);
        set_clock_output(CKO1_PLL2_SRC, false);
        pll2_set_freq(80000);
        clk_setting.clock_soure = MCLK2_CKO1_PLL2;
        clk_setting.div_setting = MCLK2_DIVIDE_4;
        clk_setting.div_sel = MCLK2_DIV_2;
        MCLK2_SetOutput(P2_4, &clk_setting);
        APP_PRINT_TRACE0("power test mclk2 pll");
    }

    if (buf != NULL)
    {
        free(buf);
    }
}
#endif

void power_test_set_32k(uint16_t action, uint8_t *buf)
{
    if (action == POWER_TEST_CMD_32K_ON)
    {
        set_clk_32k_power_in_powerdown(true);
        APP_PRINT_TRACE0("power test 32k on");
    }
    else if (action == POWER_TEST_CMD_32K_OFF)
    {
        set_clk_32k_power_in_powerdown(false);
        APP_PRINT_TRACE0("power test 32k off");
    }

    if (buf != NULL)
    {
        free(buf);
    }
}


