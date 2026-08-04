#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "cwm_customio.h"
#include "cwm_port.h"


//20250121


#define SLEEP_DATA_MAX_SIZE (1024)
#define DEFAULT_START_HOUR (9)
#define DEFAULT_START_MIN (0)
#define DEFAULT_WAKE_TIMEOUT (60)

enum
{
    ALGO_DATA_SLEEP = 21,
    ALGO_DATA_NAP = 22,
    ALGO_DATA_SLEEP_TOTAL = 31,
    ALGO_DATA_NAP_TOTAL = 32,
};
enum
{
    ALGO_ID_EnterSleep = 0,
    ALGO_ID_LightSleep = 1,
    ALGO_ID_DeepSleep = 2,
    ALGO_ID_WakeSleep = 3,
    ALGO_ID_BlinkSleeP = 12,

    ALGO_ID_LATEST_BUT_NO_EXIT = 9,
    ALGO_ID_ExitSleep = 4,
    ALGO_ID_ExitNap = 14,
};
enum
{
    SLEEP_MODE_DISABLE = 0,
    SLEEP_MODE_SLEEP = 1,
    SLEEP_MODE_NAP = 2,
};
enum
{
    FG_INVALID = 0,
    FG_VALID = 1,
    FG_VALID_BUF_FULL = 2,
};
struct sleep_merge_config_t
{
    uint8_t sleep_st_hour;// 睡眠起始时间：小时
    uint8_t sleep_st_min; // 睡眠起始时间：分钟
    uint8_t nap_st_hour;  // 小睡起始时间：小时
    uint8_t nap_st_min;   // 小睡起始时间：分钟
};
struct buffer_t
{
    int16_t head;
    int16_t latest_id_addr;
    uint16_t fg_out_state: 2;
    uint16_t fg_full: 1;
    uint16_t fg_clean_buffer_afer_exit: 1;
    uint16_t fg_total_sleep: 1;
    uint16_t mins_total;
    uint16_t mins_light_sleep;
    uint16_t mins_deep_sleep;
    uint16_t mins_wake_sleep;
    uint16_t mins_blink_sleep;
    uint8_t data[SLEEP_DATA_MAX_SIZE];
};
struct sleep_merge_t
{
uint16_t config_mode:
    1; /*两种模式：0：设置睡眠起始时间和时长；1：不设置睡眠起始时间和时长，直接执行 sleep on 和 sleep off*/
    uint16_t sleep_time_mins: 11; /*模式 0 ：设置的睡眠时长*/
    uint16_t sleep_mode: 2; /*模式 0：睡眠 SLEEP_MODE_SLEEP；模式 1：小睡 SLEEP_MODE_NAP*/
    uint8_t end_hour;
    uint8_t end_min;
    uint16_t wake_threshold_mins;/*睡眠拼接时，清醒阈值。=0 时，表示不做拼接；!0 时，时间 <= wake_threshold_mins,需要拼接，注意判断条件*/
};

struct sleep_merge_config_t sleep_merge_config_back;
static struct sleep_merge_t sleep_merge;
static struct sleep_datetime
    start_sleep_date;/*睡眠起始日期，当使用 config_mode 1 时，需要记住，用于与 end 做对比，防止错误设置时间*/
static struct sleep_datetime
    end_sleep_date;/*睡眠结束日期，当使用 config_mode 1 时，需要记住，用于与 start 做对比，防止错误设置时间*/

static struct buffer_t sleep_buf;
static struct sleep_datetime pre_sleep_enter_date;
static struct sleep_datetime pre_sleep_exit_date;
static struct sleep_datetime cur_sleep_enter_date;
static struct sleep_datetime cur_sleep_exit_date;
// static uint16_t pre_sleep_enter_buf_addr;
// static uint16_t pre_sleep_exit_buf_addr;
// static uint16_t cur_sleep_enter_buf_addr;
static uint16_t cur_sleep_exit_buf_addr;
static uint8_t
pre_sleep_data_id;/*睡眠数据的上一次 id，用于计算每个小片段持续的分钟数*/
static struct sleep_datetime
    pre_sleep_data_date;/*睡眠数据的上一次日期，用于计算每个小片段持续的分钟数*/
static struct sleep_datetime
    sleep_data_date_before_exit;/*退出睡眠的上一次数据日期，用于 merge 计算*/
/*由于清除 buffer 可能发生在 exit 4/14 时，而总数在这个之后，所以不能放在 buffer 结构体中*/
static uint8_t sleep_merge_output_index;/*customio_notify_sleep_data 序列号*/

static uint8_t data_cost_sleep_buffer_bytes(uint8_t id)
{
    uint8_t bytes = 0;
    switch (id)
    {
    case ALGO_ID_EnterSleep:
    case ALGO_ID_ExitSleep:
    case ALGO_ID_ExitNap:
        bytes = 6;
        break;
    case ALGO_ID_LightSleep:
    case ALGO_ID_DeepSleep:
    case ALGO_ID_WakeSleep:
    case ALGO_ID_BlinkSleeP:
        bytes = 2;
        break;
    default:
        break;
    }
    return bytes;
}
static void clean_sleep_buffer(void)
{
    cwm_app_debug("[algo]clean_sleep_buffer\n");
    memset((uint8_t *)&sleep_buf, 0, sizeof(sleep_buf));
    memset((uint8_t *)&pre_sleep_enter_date, 0, sizeof(pre_sleep_enter_date));
    memset((uint8_t *)&pre_sleep_exit_date, 0, sizeof(pre_sleep_exit_date));
    memset((uint8_t *)&cur_sleep_enter_date, 0, sizeof(cur_sleep_enter_date));
    memset((uint8_t *)&cur_sleep_exit_date, 0, sizeof(cur_sleep_exit_date));
    // pre_sleep_enter_buf_addr = 0;
    // pre_sleep_exit_buf_addr = 0;
    // cur_sleep_enter_buf_addr = 0;
    cur_sleep_exit_buf_addr = 0;
    pre_sleep_data_id = 0xff;
    memset((uint8_t *)&pre_sleep_data_date, 0, sizeof(pre_sleep_data_date));
    memset((uint8_t *)&sleep_data_date_before_exit, 0, sizeof(sleep_data_date_before_exit));
}
/*存入 sleep 的 id 和 数据
return: 存入总数据量*/
static int32_t add_sleep_buffer(uint8_t id, uint8_t *data, uint16_t len)
{
    if (NULL == data) { return -1; }

    if ((sleep_buf.fg_full) ||
        (sleep_buf.fg_full == 0) & ((sleep_buf.head + len + 1) > SLEEP_DATA_MAX_SIZE))
    {
        sleep_buf.fg_full = 1;
        // cwm_app_debug("[algo]sleep_buf add error: full(%d >= %d)\n",(sleep_buf.head + len + 1),SLEEP_DATA_MAX_SIZE);
        return -1;
    }

    sleep_buf.latest_id_addr = sleep_buf.head;
    sleep_buf.data[sleep_buf.head++] = id;

    uint8_t *dest_p = data;
    uint16_t dest_len = len;
    while (dest_len)
    {
        sleep_buf.data[sleep_buf.head++] = *(dest_p++);
        dest_len--;
    }

    // if(2 == len){
    //     cwm_app_debug("[algo]add_sleep_buffer(%d , %d) \n",id,*((uint16_t*)data));
    // }else{
    //     struct sleep_datetime* tmp = (struct sleep_datetime*)data;
    //     cwm_app_debug("[algo]add_sleep_buffer(%d , %d,%d,%d,%d,%d) \n",id,
    //         tmp->year,tmp->mon,tmp->day,tmp->hour,tmp->min);
    // }
    return len + 1;
}
/*判断 buffer 是否空；非空获取 sleep id*/
static uint8_t check_sleep_buffer_is_empty(uint8_t *id)
{
    /*最少 3 bytes*/
    if (sleep_buf.head < 3)
    {
        // cwm_app_debug("[algo]check_sleep_buffer_is_empty: empty(%d < %d) \n",sleep_buf.head,3);
        return 1;
    }

    if (NULL == id) { return 0; }

    *id = sleep_buf.data[sleep_buf.latest_id_addr];
    /* get_latest_sleep_buffer 只是处理最后一包数据，不会更改 sleep_buf.latest_id_addr*/

    return 0;
}
/*判断 buffer 是否满；非空获取 sleep id*/
static uint8_t check_sleep_buffer_is_full(void)
{
    return sleep_buf.fg_full;
}
/*获取 buffer 中的 sleep 数据,但不删除. 返回数据地址*/
static int32_t get_latest_sleep_buffer(uint8_t *id, uint8_t *data, uint16_t len)
{
    if (NULL == id) { return -1; }
    if (NULL == data) { return -1; }

    if (sleep_buf.head < (1 + len))
    {
        // cwm_app_debug("[algo]get_latest_sleep_buffer error: empty(%d < %d) \n",sleep_buf.head,1 + len);
        return -1;
    }

    uint8_t *dest_p = data;
    uint16_t dest_len = len;
    uint8_t *source_p = &sleep_buf.data[sleep_buf.head - dest_len - 1];

    *id = *(source_p++);
    while (dest_len)
    {
        *(dest_p++) = *(source_p++);
        dest_len--;
    }

    return sleep_buf.head - len - 1;
}
/*addr、bytes：latest id 地址和数据 bytes。uint8_t id,uint8_t* data,uint16_t len：新 id 和数据 */
static int32_t replace_latest_sleep_buffer(uint16_t addr, uint16_t data_bytes, uint8_t id,
                                           uint8_t *data, uint16_t len)
{
    if (NULL == data) { return -1; }

    uint16_t dest_bytes = data_bytes + 1;
    uint8_t *dest_p = &sleep_buf.data[addr];

    while (dest_bytes)
    {
        *(dest_p++) = 0;
        dest_bytes--;
    }
    sleep_buf.head -= data_bytes + 1;

    add_sleep_buffer(id, data, len);

    return len + 1;
}
/*获取 buffer 中的 head */
int16_t get_sleep_buffer_latest_addr(void)
{
    return sleep_buf.head;
}

static int32_t output_one_sleep_buffer(uint16_t addr, uint8_t *id, uint8_t *data,
                                       uint16_t *data_len)
{
    if (NULL == id) { return -1; }
    if (NULL == data) { return -1; }
    if (NULL == data_len) { return -1; }

    if (sleep_buf.head < (1 + addr))
    {
        // cwm_app_debug("[algo]output_one_sleep_buffer error: empty(%d < %d) \n",sleep_buf.head,1 + addr);
        return -1;
    }

    uint8_t *source_p = &sleep_buf.data[addr];
    *id = *(source_p++);

    uint16_t len = data_cost_sleep_buffer_bytes(*id);

    if (!len) { return -1; }

    if (sleep_buf.head < (1 + len + addr))
    {
        // cwm_app_debug("[algo]output_one_sleep_buffer error: empty(%d < %d) \n",sleep_buf.head,1 + len + addr);
        return -1;
    }

    *data_len = len;

    uint8_t *dest_p = data;
    uint16_t dest_len = len;

    while (dest_len)
    {
        *(dest_p++) = *(source_p++);
        dest_len--;
    }

    return 1 + len;
}

static uint16_t diff_mins(uint8_t pre_hour, uint8_t pre_min, uint8_t cur_hour, uint8_t cur_min)
{
    uint16_t mins = 0;
    /*睡眠小片段不会超过 24 小时*/
    if ((cur_hour > pre_hour) ||
        ((cur_hour == pre_hour) && (cur_min >= pre_min)))
    {
        mins = (cur_hour - pre_hour) * 60 + cur_min - pre_min;
    }
    else
    {
        mins = (24 + cur_hour - pre_hour) * 60 + cur_min - pre_min;
    }
    return mins;
}
/*return: 存入总数据量*/
static int32_t add_long_data_to_sleepbuffer(float *f)
{
    pre_sleep_data_id = f[2];
    pre_sleep_data_date.year = f[7];
    pre_sleep_data_date.mon = f[3];
    pre_sleep_data_date.day = f[4];
    pre_sleep_data_date.hour = f[5];
    pre_sleep_data_date.min = f[6];

    uint8_t id = f[2];
    struct sleep_datetime date;
    date.year = f[7];
    date.mon = f[3];
    date.day = f[4];
    date.hour = f[5];
    date.min = f[6];

    cwm_app_debug("[algo]add_long %d (%d,%d:%d)\n", id, date.day, date.hour, date.min);
    return add_sleep_buffer(id, (uint8_t *)&date, sizeof(date));
}
static void add_short_data_to_sleepbuffer(float *f)
{
    /*mins 为当前 id 与上一个 id 之间的时长：分钟，并非该 id 表示的睡眠持续时间。
    因为从 log 上看， 算法 id 和其 date 是该状态的起始时间，而非结束时间。
    而第一个 id 0 的 date 与第一个非 0 id 的 date 不是一样的。
    行  802066: NOW(092230)RT(1720660950)POLAR(0)TAG=SLR,L SN(1234924)IDX(22)DF*4(210000,170000,0,    70000,110000,10000,480000,0,0,0,0,0,0,0,0,20000)TS(50731089)
    行  802068: NOW(092230)RT(1720660950)POLAR(0)TAG=SLR,L SN(1234925)IDX(22)DF*4(210000,180000,10000,70000,110000,10000,490000,0,0,0,0,0,1010000,0,0,20000)TS(50731091)
    ......
    行  802110: NOW(092230)RT(1720660950)POLAR(0)TAG=SLR,L SN(1234939)IDX(22)DF*4(210000,320000,10000,70000,110000,80000,580000,0,0,0,0,0,15010000,0,0,20000)TS(50731132)
    行  802113: NOW(092230)RT(1720660950)POLAR(0)TAG=SLR,L SN(1234940)IDX(22)DF*4(210000,330000,40000,70000,110000,90000,220000,0,0,0,0,0,16040000,0,0,20000)TS(50731135)
    旧算法是没有年输出的，新算法有年输出

    睡眠算法数据输出顺序：enter_t  1_st  2_st ... 3_st exit_t:  id 的起始时间，而非结束时间。
    buffre 数据格式: enter_t(1_st)  2_st ... 3_st exit_t: id 的起始时间。enter_t 与 1_st 之间也是 1 的时间。
    实际操作中： enter_t 之后需要插入一个与 enter_t 相同的 1 ，然后是之后的所有 id。如果两个前后两个 id 一致，则删掉后一个 id
    */
    uint16_t mins = 0;
    uint8_t id = f[2];
    struct sleep_datetime date;
    date.year = f[7];
    date.mon = f[3];
    date.day = f[4];
    date.hour = f[5];
    date.min = f[6];


    /*如果睡眠中，修改了算法时间，且时间异常，算法输出是什么样的？睡眠拼接该如何处理*/
    /*？？？？？？*/

    /*屏蔽前后相同的 id*/
    if (id != pre_sleep_data_id)
    {
        /*睡眠小片段不会超过 24 小时*/
        mins = diff_mins(pre_sleep_data_date.hour, pre_sleep_data_date.min, date.hour, date.min);

        cwm_app_debug("[algo]add_short %d (%d)\n", id, mins);
        add_sleep_buffer(id, (uint8_t *)&mins, sizeof(mins));

        pre_sleep_data_date.year = f[7];
        pre_sleep_data_date.mon = f[3];
        pre_sleep_data_date.day = f[4];
        pre_sleep_data_date.hour = f[5];
        pre_sleep_data_date.min = f[6];
        pre_sleep_data_id = f[2];

        memcpy(&sleep_data_date_before_exit, &pre_sleep_data_date, sizeof(sleep_data_date_before_exit));
    }
}

static uint32_t days_from_2000(struct sleep_datetime *date)
{
    uint32_t cur_days;
    uint8_t leap_years;
    uint8_t rest_years;
    uint16_t rest_days = 0;

    /*先计算当年之前的总 years*/
    leap_years = (date->year - 2000 + 3) / 4;
    rest_years = date->year - 2000 - leap_years;

    /*当年 days*/
    for (uint8_t i = 1; i < date->mon; i++)
    {
        uint8_t mon_days = 0;
        switch (i)
        {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            mon_days = 31;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            mon_days = 30;
            break;
        case 2:
            mon_days = 28;
            break;
        }
        rest_days += mon_days;
    }

    /*当年是否闰年，是否已过 2.29*/
    if ((!(date->year % 4)) && (date->mon >= 3))
    {
        rest_days += 1;
    }

    rest_days += date->day;

    cur_days = leap_years * 366 + rest_years * 365 + rest_days - 1; /*不包括当天*/
    return cur_days;
}
static int32_t overday_clean_sleep_buffer(struct sleep_datetime *pre, struct sleep_datetime *cur,
                                          uint8_t hour, uint8_t min)
{
    if ((NULL == pre) || (NULL == cur)) { return 0; }

    struct sleep_datetime pre_tmp;
    struct sleep_datetime cur_tmp;

    memcpy(&pre_tmp, pre, sizeof(struct sleep_datetime));
    memcpy(&cur_tmp, cur, sizeof(struct sleep_datetime));

    /*旧算法没有年输出，新算法有年输出*/
    if (cur_tmp.year == 0)
    {
        if (((2 == pre_tmp.mon) && (29 == pre_tmp.day)) || ((2 == cur_tmp.mon) && (29 == cur_tmp.day)))
        {
            pre_tmp.year = 2004;/*当年是闰年*/
            cur_tmp.year = 2004;/*当年是闰年*/
        }
        else
        {
            /*其它情况可以当作非闰年。因为没有 29 号的睡眠数据，可以认为这一天没有睡觉*/
            pre_tmp.year = 2001;/*当年是非闰年*/
            cur_tmp.year = 2001;/*当年是非闰年*/
        }
    }

    /*从 2000 年开始计算*/
    if ((cur_tmp.year <= 2000) || (pre_tmp.year <= 2000))
    {
        cwm_app_debug("[algo]sleep year(< 2000) error: %d,%d\n", cur_tmp.year, pre_tmp.year);
        /*1、year 错误，清除 buffer*/
        return 1;
    }
    else
    {
        uint32_t pre_mins = days_from_2000(&pre_tmp) * 24 * 60 + pre_tmp.hour * 60 + pre_tmp.min;
        uint32_t cur_mins = days_from_2000(&cur_tmp) * 24 * 60 + cur_tmp.hour * 60 + cur_tmp.min;

        if (cur_mins < pre_mins)
        {
            /*1、pre 大于 cur。清 buffer*/
            return 1;
        }
        else if ((cur_mins - pre_mins) >= 24 * 60)
        {
            /*2、pre 与 cur 相差 24 小时以上。清 buffer*/
            return 1;
        }
        else
        {
            /*3、pre 与 cur 相差 24 小时以内，hour.min 在 pre 与 cur 中间。清 buffer*/
            uint32_t pre_mins = pre_tmp.hour * 60 + pre_tmp.min;
            uint32_t cur_mins = cur_tmp.hour * 60 + cur_tmp.min;

            /*无论是否跨天，只要 pre_hour.pre_min 或者 cur_hour.cur_min 在 pre 与 cur 中间。清 buffer */
            if (cur_mins < pre_mins)
            {
                /*跨天*/
                uint32_t pre_end_mins, cur_end_mins;
                cur_mins += 24 * 60;

                pre_end_mins = hour * 60 + min;
                cur_end_mins = hour * 60 + min + 24 * 60;

                if ((cur_mins >= pre_end_mins) && (pre_mins <= pre_end_mins))
                {
                    /*3、清 buffer*/
                    return 1;
                }
                if ((cur_mins >= cur_end_mins) && (pre_mins <= cur_end_mins))
                {
                    /*3、清 buffer*/
                    return 1;
                }
            }
            else
            {
                /*不跨天*/
                uint32_t end_mins;
                end_mins = hour * 60 + min;
                if ((cur_mins >= end_mins) && (pre_mins <= end_mins))
                {
                    /*3、清 buffer*/
                    return 1;
                }
            }
        }
    }
    return 0;
}

static uint16_t merge_sleep_data(struct sleep_datetime *pre_exit, uint16_t pre_exit_addr,
                                 struct sleep_datetime *cur_enter,
                                 struct sleep_datetime *wake_before_date)
{
    if ((NULL == pre_exit) || (NULL == cur_enter)) { return 0; }

    /*sleep_merge.wake_threshold_mins = 0，不做拼接；!0 需要做拼接*/
    if (!sleep_merge.wake_threshold_mins) { return 0; }

    /*小睡不能拼接*/
    if (SLEEP_MODE_NAP == sleep_merge.sleep_mode) { return 0; }

    /*buffer 已满不做任何拼接*/
    if (check_sleep_buffer_is_full()) { return 0; }

    struct sleep_datetime pre_tmp;
    struct sleep_datetime cur_tmp;
    struct sleep_datetime date_before_exit_tmp;
    uint16_t wake_mins = 0;

    memcpy(&pre_tmp, pre_exit, sizeof(struct sleep_datetime));
    memcpy(&cur_tmp, cur_enter, sizeof(struct sleep_datetime));
    memcpy(&date_before_exit_tmp, wake_before_date, sizeof(struct sleep_datetime));

    /*旧算法没有年输出，新算法有年输出*/
    if (cur_tmp.year == 0)
    {
        if (((2 == pre_tmp.mon) && (29 == pre_tmp.day)) || ((2 == cur_tmp.mon) && (29 == cur_tmp.day)))
        {
            pre_tmp.year = 2004;/*当年是闰年*/
            cur_tmp.year = 2004;/*当年是闰年*/
            date_before_exit_tmp.year = 2004;/*当年是闰年*/
        }
        else
        {
            /*其它情况可以当作非闰年。因为没有 29 号的睡眠数据，可以认为这一天没有睡觉*/
            pre_tmp.year = 2001;/*当年是非闰年*/
            cur_tmp.year = 2001;/*当年是非闰年*/
            date_before_exit_tmp.year = 2001;/*当年是闰年*/
        }
    }

    /*从 2000 年开始计算*/
    if ((cur_tmp.year <= 2000) || (pre_tmp.year <= 2000))
    {
        cwm_app_debug("[algo]sleep year(< 2000) error: %d,%d\n", cur_tmp.year, pre_tmp.year);
        /*1、year 错误，不做任何拼接*/
        return 0;
    }
    else
    {
        uint32_t pre_mins = days_from_2000(&pre_tmp) * 24 * 60 + pre_tmp.hour * 60 + pre_tmp.min;
        uint32_t cur_mins = days_from_2000(&cur_tmp) * 24 * 60 + cur_tmp.hour * 60 + cur_tmp.min;

        if (cur_mins <= pre_mins)
        {
            /*1、pre 大于 cur。不做任何拼接*/
            return 0;
        }
        else
        {
            wake_mins = cur_mins - pre_mins;
            if (wake_mins > sleep_merge.wake_threshold_mins)
            {
                /*2、pre 与 cur 间隔大于 sleep_merge.wake_threshold_mins 。不做任何拼接*/
                return 0;
            }
        }
    }

    // cwm_app_debug("[algo]merge_sleep_data wake %d\n",wake_mins);
    if (!wake_mins) { return 0; }

    /*拼接: */
    uint16_t diff_mins;/*这个是用于计算 wake 的起始时间点*/
    struct sleep_datetime wake_date;
    uint8_t id;
    uint8_t data_bytes;
    uint16_t addr;

    uint32_t pre_mins = days_from_2000(&pre_tmp) * 24 * 60 + pre_tmp.hour * 60 + pre_tmp.min;
    uint32_t wake_before_mins = days_from_2000(&date_before_exit_tmp) * 24 * 60 +
                                date_before_exit_tmp.hour * 60 + date_before_exit_tmp.min;
    if (pre_mins <= wake_before_mins)
    {
        /*1、pre 小于 wake_before_mins。不做任何拼接*/
        return 0;
    }
    else
    {
        diff_mins = pre_mins - wake_before_mins;
    }

    // cwm_app_debug("[algo]merge_sleep_data diff %d\n",diff_mins);

    if (!check_sleep_buffer_is_empty(&id))
    {
        if ((id != ALGO_ID_ExitSleep) && (id != ALGO_ID_ExitNap) && (id != ALGO_ID_LATEST_BUT_NO_EXIT))
        {
            return 0;
        }

        data_bytes = data_cost_sleep_buffer_bytes(id);

        addr = get_latest_sleep_buffer(&id, (uint8_t *)&wake_date, data_bytes);
        if ((id != ALGO_ID_ExitSleep) && (id != ALGO_ID_ExitNap) && (id != ALGO_ID_LATEST_BUT_NO_EXIT))
        {
            return 0;
        }
        id = ALGO_ID_WakeSleep;
        // cwm_app_debug("[algo]merge_sleep_data addr %d,%d\n",addr,data_bytes);
        replace_latest_sleep_buffer(addr, data_bytes, id, (uint8_t *)&diff_mins, sizeof(diff_mins));
    }
    // cwm_app_debug("[algo]merge_sleep_data\n");
    return wake_mins;
}





void sleep_merge_get_config(uint8_t *sshour, uint8_t *ssmin, uint8_t *nshour, uint8_t *nsmin,
                            uint16_t *wake_timeout)
{
    struct sleep_merge_config_t *back = &sleep_merge_config_back;
    *sshour = back->sleep_st_hour;
    *ssmin = back->sleep_st_min;
    *nshour = back->nap_st_hour;
    *nsmin = back->nap_st_min;
    *wake_timeout = sleep_merge.wake_threshold_mins;
}
void sleep_merge_set_config(uint8_t sshour, uint8_t ssmin, uint8_t nshour, uint8_t nsmin,
                            uint16_t wake_timeout)
{
    struct sleep_merge_config_t *back = &sleep_merge_config_back;

    /*排除错误的设定时间*/
    if (sshour >= 24)
    {
        back->sleep_st_hour = DEFAULT_START_HOUR;
        // cwm_app_debug("[algo]error sleep hour, use default%d\n",DEFAULT_START_HOUR);
    }
    else
    {
        back->sleep_st_hour = sshour;
    }
    if (ssmin >= 60)
    {
        back->sleep_st_min = DEFAULT_START_MIN;
        // cwm_app_debug("[algo]error sleep min, use default%d\n",DEFAULT_START_MIN);
    }
    else
    {
        back->sleep_st_min = ssmin;
    }

    /*排除错误的设定时间*/
    if (nshour >= 24)
    {
        back->nap_st_hour = ((back->sleep_st_hour + 12) >= 24) ? (back->sleep_st_hour + 12 - 24) :
                            (back->sleep_st_hour + 12);
        // cwm_app_debug("[algo]error nap hour, use default%d\n",back->nap_st_hour);
    }
    else
    {
        back->nap_st_hour = nshour;
    }
    if (nsmin >= 60)
    {
        back->nap_st_min = DEFAULT_START_MIN;
        // cwm_app_debug("[algo]error nap min, use default%d\n",DEFAULT_START_MIN);
    }
    else
    {
        back->nap_st_min = nsmin;
    }

    if (wake_timeout > 3 * 60)
    {
        sleep_merge.wake_threshold_mins = DEFAULT_WAKE_TIMEOUT;
        // cwm_app_debug("[algo]error wake_threshold_mins, use default%d\n",DEFAULT_WAKE_TIMEOUT);
    }
    else
    {
        sleep_merge.wake_threshold_mins = wake_timeout;
    }

    cwm_app_debug("[algo]sleep start time(%d,%d)\n", back->sleep_st_hour, back->sleep_st_min);
    cwm_app_debug("[algo]nap start time(%d,%d)\n", back->nap_st_hour, back->nap_st_min);
    cwm_app_debug("[algo]wake_threshold_mins %d\n", sleep_merge.wake_threshold_mins);
}
void sleep_merge_init(void)
{
    clean_sleep_buffer();
    memset((uint8_t *)&sleep_merge, 0, sizeof(sleep_merge));
    memset(&start_sleep_date, 0, sizeof(start_sleep_date));
    memset(&end_sleep_date, 0, sizeof(end_sleep_date));
    memset(&sleep_merge_config_back, 0, sizeof(sleep_merge_config_back));
}
static void set_sleep_merge_out_fg(uint8_t state)
{
    sleep_buf.fg_out_state = state;

    if (FG_VALID == state)
    {
        if (sleep_buf.fg_full) { sleep_buf.fg_out_state = FG_VALID_BUF_FULL; }
    }
}
void sleep_merge_input(float *f)
{
    // cwm_app_debug("[algo]sleep_merge_input(%f , %f) \n",f[0],f[2]);

    if (f[0] == ALGO_DATA_SLEEP)
    {
        if (sleep_merge.sleep_mode != SLEEP_MODE_SLEEP)
        {
            clean_sleep_buffer();
            sleep_merge.sleep_mode = SLEEP_MODE_SLEEP;
        }
        switch ((uint32_t)f[2])
        {
        case ALGO_ID_EnterSleep:
            {
                /*入睡与出睡的时候，将 end_hour、end_min 填入,
                sleep 的 start time 是 nap 的 end time
                nap 的 start time 是 sleep 的 end time
                */
                sleep_merge.end_hour = sleep_merge_config_back.nap_st_hour;
                sleep_merge.end_min = sleep_merge_config_back.nap_st_min;

                sleep_merge_output_index = 0;
                set_sleep_merge_out_fg(FG_INVALID);

                struct sleep_datetime tmp;
                tmp.year = f[7];
                tmp.mon = f[3];
                tmp.day = f[4];
                tmp.hour = f[5];
                tmp.min = f[6];

                uint8_t id;
                uint8_t fg_merge = 0;
                if (check_sleep_buffer_is_empty(&id))
                {
                    /*do nothing*/
                }
                else
                {
                    /*检测是否是第二天睡眠 case1: 如果 end_hour.end_min 在 tmp 与 cur_exit 中间，清 buffer*/
                    // struct sleep_datetime* ttt = &cur_sleep_exit_date;
                    // cwm_app_debug("[algo]ALGO_ID_EnterSleep cur_sleep_enter_date (%d,%d,%d,%d,%d) \n",
                    //     ttt->year,ttt->mon,ttt->day,ttt->hour,ttt->min);
                    // ttt = &tmp;
                    // cwm_app_debug("[algo]ALGO_ID_EnterSleep tmp(%d,%d,%d,%d,%d) \n",
                    //     ttt->year,ttt->mon,ttt->day,ttt->hour,ttt->min);
                    // cwm_app_debug("[algo]ALGO_ID_EnterSleep tmp(%d,%d) \n",
                    //     sleep_merge.end_hour,sleep_merge.end_min);

                    if (overday_clean_sleep_buffer(&cur_sleep_exit_date, &tmp, sleep_merge.end_hour,
                                                   sleep_merge.end_min))
                    {
                        // cwm_app_debug("[algo]ALGO_ID_EnterSleep overday_clean_sleep_buffer\n");
                        clean_sleep_buffer();
                    }
                    else
                    {
                        /*睡眠拼接*/
                        /*id + time = 上一次的时间点 + id_mins*/
                        uint16_t wake_mins = merge_sleep_data(&cur_sleep_exit_date, cur_sleep_exit_buf_addr, &tmp,
                                                              &sleep_data_date_before_exit);
                        if (wake_mins)
                        {
                            fg_merge = 1;
                        }

                        sleep_buf.mins_total += wake_mins;
                        sleep_buf.mins_wake_sleep += wake_mins;
                        /*
                        拼接过程：
                        1、cur_exit 改成 wake。同时计算 mins。
                        2、enter 改成 id 1
                        3、重复 1 合并
                        */
                    }
                }

                memset((uint8_t *)&sleep_data_date_before_exit, 0, sizeof(sleep_data_date_before_exit));

                // uint16_t tmp_addr = get_sleep_buffer_latest_addr();

                /*cur date、pre date 与是否保存成功没有关系。*/
                if (!fg_merge)
                {
                    /*merge 成功，enter 取消*/
                    pre_sleep_enter_date.year = cur_sleep_enter_date.year;
                    pre_sleep_enter_date.mon = cur_sleep_enter_date.mon;
                    pre_sleep_enter_date.day = cur_sleep_enter_date.day;
                    pre_sleep_enter_date.hour = cur_sleep_enter_date.hour;
                    pre_sleep_enter_date.min = cur_sleep_enter_date.min;
                    cur_sleep_enter_date.year = tmp.year;
                    cur_sleep_enter_date.mon = tmp.mon;
                    cur_sleep_enter_date.day = tmp.day;
                    cur_sleep_enter_date.hour = tmp.hour;
                    cur_sleep_enter_date.min = tmp.min;

                    if (add_long_data_to_sleepbuffer(f) > 0)
                    {
                        /*只有数据入 buffer 成功，才记录 pre ddr 和 cur addr*/
                        // pre_sleep_enter_buf_addr = cur_sleep_enter_buf_addr;
                        // cur_sleep_enter_buf_addr = tmp_addr;
                    }
                }

                float tmp_f[8];
                memcpy(tmp_f, f, sizeof(tmp_f));
                tmp_f[2] = ALGO_ID_LightSleep;
                add_short_data_to_sleepbuffer(tmp_f);
            }
            break;
        case ALGO_ID_LightSleep:
        case ALGO_ID_DeepSleep:
        case ALGO_ID_WakeSleep:
        case ALGO_ID_BlinkSleeP:
            {
                add_short_data_to_sleepbuffer(f);
            }
            break;
        case ALGO_ID_ExitSleep:
            {
                /*入睡与出睡的时候，将 end_hour、end_min 填入,
                sleep 的 start time 是 nap 的 end time
                nap 的 start time 是 sleep 的 end time
                */
                sleep_merge.end_hour = sleep_merge_config_back.nap_st_hour;
                sleep_merge.end_min = sleep_merge_config_back.nap_st_min;

                struct sleep_datetime tmp;
                tmp.year = f[7];
                tmp.mon = f[3];
                tmp.day = f[4];
                tmp.hour = f[5];
                tmp.min = f[6];

                /*检测是否是第二天睡眠 case2: 如果 end_hour.end_min 在 tmp 与 cur_enter 中间，清 buffer*/
                // struct sleep_datetime* ttt = &cur_sleep_enter_date;
                // cwm_app_debug("[algo]ALGO_ID_ExitSleep cur_sleep_enter_date (%d,%d,%d,%d,%d) \n",
                //     ttt->year,ttt->mon,ttt->day,ttt->hour,ttt->min);
                // ttt = &tmp;
                // cwm_app_debug("[algo]ALGO_ID_ExitSleep tmp(%d,%d,%d,%d,%d) \n",
                //     ttt->year,ttt->mon,ttt->day,ttt->hour,ttt->min);

                if (overday_clean_sleep_buffer(&cur_sleep_enter_date, &tmp, sleep_merge.end_hour,
                                               sleep_merge.end_min))
                {
                    sleep_buf.fg_clean_buffer_afer_exit = 1;
                    // cwm_app_debug("[algo]ALGO_ID_ExitSleep fg_clean_buffer_afer_exit %d\n",sleep_buf.fg_clean_buffer_afer_exit);
                }

                /*cur date、pre date 与是否保存成功没有关系。*/
                pre_sleep_exit_date.year = cur_sleep_exit_date.year;
                pre_sleep_exit_date.mon = cur_sleep_exit_date.mon;
                pre_sleep_exit_date.day = cur_sleep_exit_date.day;
                pre_sleep_exit_date.hour = cur_sleep_exit_date.hour;
                pre_sleep_exit_date.min = cur_sleep_exit_date.min;
                cur_sleep_exit_date.year = tmp.year;
                cur_sleep_exit_date.mon = tmp.mon;
                cur_sleep_exit_date.day = tmp.day;
                cur_sleep_exit_date.hour = tmp.hour;
                cur_sleep_exit_date.min = tmp.min;

                /*只有数据入 buffer 成功，才记录 pre ddr 和 cur addr*/
                uint16_t tmp_addr = get_sleep_buffer_latest_addr();
                if (add_long_data_to_sleepbuffer(f) > 0)
                {
                    // pre_sleep_exit_buf_addr = cur_sleep_exit_buf_addr;
                    cur_sleep_exit_buf_addr = tmp_addr;
                }

                set_sleep_merge_out_fg(FG_VALID);
            }
            break;
        default:
            break;
        }
    }
    else if (f[0] == ALGO_DATA_NAP)
    {
        if (sleep_merge.sleep_mode != SLEEP_MODE_NAP)
        {
            clean_sleep_buffer();
            sleep_merge.sleep_mode = SLEEP_MODE_NAP;
        }
        switch ((uint32_t)f[2])
        {
        case ALGO_ID_EnterSleep:
            {
                /*入睡与出睡的时候，将 end_hour、end_min 填入,
                sleep 的 start time 是 nap 的 end time
                nap 的 start time 是 sleep 的 end time
                */
                sleep_merge.end_hour = sleep_merge_config_back.sleep_st_hour;
                sleep_merge.end_min = sleep_merge_config_back.sleep_st_min;

                sleep_merge_output_index = 0;
                set_sleep_merge_out_fg(FG_INVALID);

                struct sleep_datetime tmp;
                tmp.year = f[7];
                tmp.mon = f[3];
                tmp.day = f[4];
                tmp.hour = f[5];
                tmp.min = f[6];

                uint8_t id;
                uint8_t fg_merge = 0;
                if (check_sleep_buffer_is_empty(&id))
                {
                    /*do nothing*/
                }
                else
                {
                    /*检测是否是第二天睡眠 case1: 如果 end_hour.end_min 在 tmp 与 cur_exit 中间，清 buffer*/
                    if (overday_clean_sleep_buffer(&cur_sleep_exit_date, &tmp, sleep_merge.end_hour,
                                                   sleep_merge.end_min))
                    {
                        // cwm_app_debug("[algo]ALGO_ID_EnterSleep overday_clean_sleep_buffer\n");
                        clean_sleep_buffer();
                    }
                    else
                    {
                        /*睡眠拼接*/
                        /*id + time = 上一次的时间点 + id_mins*/
                        uint16_t wake_mins = merge_sleep_data(&cur_sleep_exit_date, cur_sleep_exit_buf_addr, &tmp,
                                                              &sleep_data_date_before_exit);
                        if (wake_mins)
                        {
                            fg_merge = 1;
                        }

                        sleep_buf.mins_total += wake_mins;
                        sleep_buf.mins_wake_sleep += wake_mins;
                        /*
                        拼接过程：
                        1、cur_exit 改成 wake。同时计算 mins。
                        2、enter 改成 id 1
                        3、重复 1 合并
                        */
                    }
                }

                memset((uint8_t *)&sleep_data_date_before_exit, 0, sizeof(sleep_data_date_before_exit));

                // uint16_t tmp_addr = get_sleep_buffer_latest_addr();

                /*cur date、pre date 与是否保存成功没有关系。*/
                if (!fg_merge)
                {
                    /*merge 成功，enter 取消*/
                    pre_sleep_enter_date.year = cur_sleep_enter_date.year;
                    pre_sleep_enter_date.mon = cur_sleep_enter_date.mon;
                    pre_sleep_enter_date.day = cur_sleep_enter_date.day;
                    pre_sleep_enter_date.hour = cur_sleep_enter_date.hour;
                    pre_sleep_enter_date.min = cur_sleep_enter_date.min;
                    cur_sleep_enter_date.year = tmp.year;
                    cur_sleep_enter_date.mon = tmp.mon;
                    cur_sleep_enter_date.day = tmp.day;
                    cur_sleep_enter_date.hour = tmp.hour;
                    cur_sleep_enter_date.min = tmp.min;

                    if (add_long_data_to_sleepbuffer(f) > 0)
                    {
                        /*只有数据入 buffer 成功，才记录 pre ddr 和 cur addr*/
                        // pre_sleep_enter_buf_addr = cur_sleep_enter_buf_addr;
                        // cur_sleep_enter_buf_addr = tmp_addr;
                    }
                }

                float tmp_f[8];
                memcpy(tmp_f, f, sizeof(tmp_f));
                tmp_f[2] = ALGO_ID_LightSleep;
                add_short_data_to_sleepbuffer(tmp_f);
            }
            break;
        case ALGO_ID_LightSleep:
        case ALGO_ID_DeepSleep:
        case ALGO_ID_WakeSleep:
        case ALGO_ID_BlinkSleeP:
            {
                add_short_data_to_sleepbuffer(f);
            }
            break;
        case ALGO_ID_ExitNap:
            {
                /*入睡与出睡的时候，将 end_hour、end_min 填入,
                sleep 的 start time 是 nap 的 end time
                nap 的 start time 是 sleep 的 end time
                */
                sleep_merge.end_hour = sleep_merge_config_back.sleep_st_hour;
                sleep_merge.end_min = sleep_merge_config_back.sleep_st_min;

                struct sleep_datetime tmp;
                tmp.year = f[7];
                tmp.mon = f[3];
                tmp.day = f[4];
                tmp.hour = f[5];
                tmp.min = f[6];

                /*检测是否是第二天睡眠 case2: 如果 end_hour.end_min 在 tmp 与 cur_enter 中间，清 buffer*/
                if (overday_clean_sleep_buffer(&cur_sleep_enter_date, &tmp, sleep_merge.end_hour,
                                               sleep_merge.end_min))
                {
                    sleep_buf.fg_clean_buffer_afer_exit = 1;
                    // cwm_app_debug("[algo]ALGO_ID_ExitNap fg_clean_buffer_afer_exit %d\n",sleep_buf.fg_clean_buffer_afer_exit);
                }

                /*cur date、pre date 与是否保存成功没有关系。*/
                pre_sleep_exit_date.year = cur_sleep_exit_date.year;
                pre_sleep_exit_date.mon = cur_sleep_exit_date.mon;
                pre_sleep_exit_date.day = cur_sleep_exit_date.day;
                pre_sleep_exit_date.hour = cur_sleep_exit_date.hour;
                pre_sleep_exit_date.min = cur_sleep_exit_date.min;
                cur_sleep_exit_date.year = tmp.year;
                cur_sleep_exit_date.mon = tmp.mon;
                cur_sleep_exit_date.day = tmp.day;
                cur_sleep_exit_date.hour = tmp.hour;
                cur_sleep_exit_date.min = tmp.min;

                /*只有数据入 buffer 成功，才记录 pre ddr 和 cur addr*/
                uint16_t tmp_addr = get_sleep_buffer_latest_addr();
                if (add_long_data_to_sleepbuffer(f) > 0)
                {
                    // pre_sleep_exit_buf_addr = cur_sleep_exit_buf_addr;
                    cur_sleep_exit_buf_addr = tmp_addr;
                }

                set_sleep_merge_out_fg(FG_VALID);
            }
            break;
        default:
            break;
        }
    }
    else if (f[0] == ALGO_DATA_SLEEP_TOTAL)
    {
        sleep_buf.mins_total += f[1];
        sleep_buf.mins_light_sleep += f[2];
        sleep_buf.mins_deep_sleep += f[3];
        sleep_buf.mins_wake_sleep += f[4];
        sleep_buf.mins_blink_sleep += f[5];
        sleep_buf.fg_total_sleep = 1;
    }
    else if (f[0] == ALGO_DATA_NAP_TOTAL)
    {
        sleep_buf.mins_total += f[1];
        sleep_buf.mins_light_sleep += f[2];
        sleep_buf.mins_deep_sleep += f[3];
        sleep_buf.mins_wake_sleep += f[4];
        sleep_buf.mins_blink_sleep += f[5];
        sleep_buf.fg_total_sleep = 1;
    }
    else if ((f[0] == ALGO_DATA_SLEEP) && (f[2] == ALGO_ID_LATEST_BUT_NO_EXIT))
    {
        /*睡眠是有：9: 未出睡_最後一筆資料
            小睡没有这个。
            因为睡眠没有结束，所以同级总数居暂不加入，只输出数据
        */
    }
}
static uint16_t output_one_lump_sleepdata(uint16_t enter_addr, struct sleep_datetime *enter,
                                          struct sleep_datetime *exit)
{
    uint8_t id;
    uint8_t data[6];
    uint16_t len;
    struct sleep_datetime tmp_date;
    uint16_t total_bytes = 0;

    if (NULL == enter) { return total_bytes; }
    if (NULL == exit) { return total_bytes; }

    memcpy(&tmp_date, enter, sizeof(tmp_date));
    for (uint16_t addr = enter_addr; addr < get_sleep_buffer_latest_addr();)
    {
        if (output_one_sleep_buffer(addr, &id, data, &len) > 0)
        {
            if (id == ALGO_ID_EnterSleep)
            {
#if (SLEEPMERGE_OUTPUTNAP_CumulativeResults == 0)
                if (sleep_merge.sleep_mode == SLEEP_MODE_SLEEP)
#endif
                {
                    customio_notify_sleep_data(sleep_merge_output_index++, sleep_merge.sleep_mode, id, data);
                }
                // struct sleep_datetime* tmp = (struct sleep_datetime*)data;
                // cwm_app_debug("[algo]output_one_lump_sleepdata(%d(%d) , %d,%d,%d,%d,%d) \n",sleep_merge_output_index,id,
                //     tmp->year,tmp->mon,tmp->day,tmp->hour,tmp->min);
            }
            else if ((id == ALGO_ID_ExitSleep) || (id == ALGO_ID_ExitNap) || (id == ALGO_ID_LATEST_BUT_NO_EXIT))
            {
#if (SLEEPMERGE_OUTPUTNAP_CumulativeResults == 0)
                if (sleep_merge.sleep_mode == SLEEP_MODE_SLEEP)
#endif
                {
                    customio_notify_sleep_data(sleep_merge_output_index++, sleep_merge.sleep_mode, id, data);
                }
                // struct sleep_datetime* tmp = (struct sleep_datetime*)data;
                // cwm_app_debug("[algo]output_one_lump_sleepdata(%d(%d) , %d,%d,%d,%d,%d) \n",sleep_merge_output_index,id,
                //     tmp->year,tmp->mon,tmp->day,tmp->hour,tmp->min);
                addr += len + 1;
                total_bytes += len + 1;
                break;
            }
            else
            {
                uint16_t mins = 0;
                memcpy((uint8_t *)&mins, data, sizeof(mins));

                mins += tmp_date.min;
                tmp_date.hour += mins / 60;
                tmp_date.min = mins % 60;

                if (mins > 255)
                {
                    cwm_app_debug("[algo]get_short %d (%d)\n", id, mins);
                }

                /*睡眠小片段（浅睡、深睡...）不会超过 24 小时*/
                if (tmp_date.hour / 24)
                {
                    tmp_date.hour = tmp_date.hour % 24;
                    tmp_date.day = exit->day;
                    tmp_date.mon = exit->mon;
                    tmp_date.year = exit->year;
                }

#if (SLEEPMERGE_OUTPUTNAP_CumulativeResults == 0)
                if (sleep_merge.sleep_mode == SLEEP_MODE_SLEEP)
#endif
                {
                    customio_notify_sleep_data(sleep_merge_output_index++, sleep_merge.sleep_mode, id,
                                               (uint8_t *)&tmp_date);
                    // cwm_app_debug("[algo]output_one_lump_sleepdata(%d(%d) , %d) \n",sleep_merge_output_index,id,*((uint16_t*)data));
                }
            }
            addr += len + 1;
            total_bytes += len + 1;
        }
        else
        {
            return total_bytes;
        }
    }
    // cwm_app_debug("[algo]output_one_lump_sleepdata %d\n",total_bytes);
    return total_bytes;
}
void sleep_merge_output(void)
{
    if (sleep_buf.fg_out_state == FG_INVALID)
    {
        goto END_SLEEP_MERGE_OUT;
    }
    else
    {
        /*sleep_buf.fg_out_state = 1,输出数据后清除标识*/
        sleep_buf.fg_out_state = FG_INVALID;
    }

    uint8_t id;
    uint8_t data[6];
    uint16_t len;
    struct sleep_datetime cur_enter, cur_exit;
    uint8_t fg_no_error;

    // cwm_app_debug("[algo]head %d\n",get_sleep_buffer_latest_addr());
    for (uint16_t enter_addr = 0; enter_addr < get_sleep_buffer_latest_addr();)
    {
        fg_no_error = 0;
        id = 0;
        len = 0;
        memset(data, 0, sizeof(data));
        memset(&cur_enter, 0, sizeof(cur_enter));
        memset(&cur_exit, 0, sizeof(cur_exit));

        /*需要将当前睡眠段的 enter 和 exit 提前读出来，用于其它小片段的日期确认*/
        if (output_one_sleep_buffer(enter_addr, &id, data, &len) > 0)
        {
            if ((id == ALGO_ID_EnterSleep) && (6 == len))
            {
                memcpy(&cur_enter, data, len);
                fg_no_error += 1;
            }
        }
        else
        {
            // cwm_app_debug("[algo]sleep_merge_output error no enter id\n");
            goto END_SLEEP_MERGE_OUT;
        }

        // if(2 == len){
        //     cwm_app_debug("[algo]1sleep_merge_output(%d , %d) \n",id,*((uint16_t*)data));
        // }else{
        //     struct sleep_datetime* tmp = (struct sleep_datetime*)data;
        //     cwm_app_debug("[algo]1sleep_merge_output(%d , %d,%d,%d,%d,%d) \n",id,
        //         tmp->year,tmp->mon,tmp->day,tmp->hour,tmp->min);
        // }

        /*需要将当前睡眠段的 enter 和 exit 提前读出来，用于其它小片段的日期确认*/
        uint16_t addr;
        for (addr = enter_addr; addr < get_sleep_buffer_latest_addr();)
        {
            if (output_one_sleep_buffer(addr, &id, data, &len) > 0)
            {
                if (((id == ALGO_ID_ExitSleep) || (id == ALGO_ID_ExitNap) || (id == ALGO_ID_LATEST_BUT_NO_EXIT)) &&
                    (6 == len))
                {
                    memcpy(&cur_exit, data, len);
                    fg_no_error += 1;
                }
                addr += len + 1;

                // if(2 == len){
                //     cwm_app_debug("[algo]2sleep_merge_output(%d , %d) \n",id,*((uint16_t*)data));
                // }else{
                //     struct sleep_datetime* tmp = (struct sleep_datetime*)data;
                //     cwm_app_debug("[algo]2sleep_merge_output(%d , %d,%d,%d,%d,%d) \n",id,
                //         tmp->year,tmp->mon,tmp->day,tmp->hour,tmp->min);
                // }

                if (2 == fg_no_error)
                {
                    /*当前睡眠段数据输出*/
                    enter_addr += output_one_lump_sleepdata(enter_addr, &cur_enter, &cur_exit);
                    break;
                }
            }
            else
            {
                // cwm_app_debug("[algo]sleep_merge_output error id\n");
                goto END_SLEEP_MERGE_OUT;
            }
        }

        /*找不到 exit id*/
        if ((addr == get_sleep_buffer_latest_addr()) && (2 != fg_no_error))
        {
            // cwm_app_debug("[algo]sleep_merge_output error no enter or exit id\n");
            goto END_SLEEP_MERGE_OUT;
        }
    }

END_SLEEP_MERGE_OUT:
    // cwm_app_debug("[algo]sleep_merge_output end %d,%d\n",sleep_buf.fg_send,sleep_merge.sleep_mode);
    if (sleep_buf.fg_total_sleep)
    {
        sleep_buf.fg_total_sleep = 0;

        uint16_t total[5];
        total[0] = sleep_buf.mins_total;
        total[1] = sleep_buf.mins_light_sleep;
        total[2] = sleep_buf.mins_deep_sleep;
        total[3] = sleep_buf.mins_wake_sleep;
        total[4] = sleep_buf.mins_blink_sleep;

        if (sleep_merge.sleep_mode == SLEEP_MODE_SLEEP)
        {
            customio_notify_sleep_data(sleep_merge_output_index++, ALGO_DATA_SLEEP_TOTAL, 0, (uint8_t *)total);
            // cwm_app_debug("[algo]output_one_lump_sleepdata(%d(%d) , %d,%d,%d,%d,%d \n",sleep_merge_output_index,ALGO_DATA_SLEEP_TOTAL,
            //     total[0],total[1],total[2],total[3],total[4]);
        }
#if (SLEEPMERGE_OUTPUTNAP_CumulativeResults == 1)
        else if (sleep_merge.sleep_mode == SLEEP_MODE_NAP)
        {
            customio_notify_sleep_data(sleep_merge_output_index++, ALGO_DATA_NAP_TOTAL, 0, (uint8_t *)total);
            // cwm_app_debug("[algo]output_one_lump_sleepdata(%d(%d) , %d,%d,%d,%d,%d \n",sleep_merge_output_index,ALGO_DATA_NAP_TOTAL,
            //     total[0],total[1],total[2],total[3],total[4]);
        }
#endif

        //cwm_app_debug("[algo]sleep_buf.fg_clean_buffer_afer_exit %d\n",sleep_buf.fg_clean_buffer_afer_exit);
        if (sleep_buf.fg_clean_buffer_afer_exit)
        {
            sleep_buf.fg_clean_buffer_afer_exit = 0;
            clean_sleep_buffer();
        }
    }
}
