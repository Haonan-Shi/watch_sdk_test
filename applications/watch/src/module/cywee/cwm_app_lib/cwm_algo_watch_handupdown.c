#include "stdint.h"
#include "string.h"
#include "stdbool.h"
#include "cwm_lib_dml.h"
#include "cwm_handupdown_FPU.h"
#include "cwm_customio.h"

#if (CWM_HANDUPDOWN_Split == 1)
extern int cwm_app_debug(const char *format, ...);

// #define DEBUG_HANDUP(format,...)
#define DEBUG_HANDUP(format,...) cwm_app_debug(format,##__VA_ARGS__)
#define RAW_MAX 52

void *etMems = NULL;
uint32_t buffer[1000 / 4];

/*sensor data，use for watch_handupdown*/
struct AccelerationData
{
    int32_t dt;
    float acc_x;
    float acc_y;
    float acc_z;
};
struct AccelerationData rawdata[RAW_MAX];
int AccdataCount = 0;
int handupdown_sn = 0;

/*cwm_watch_handupdown_init*/
void cwm_watch_handupdown_init(void)
{
    memset(buffer, 0, sizeof(buffer));
    if (etMems == NULL)
    {
        etMems = buffer;
        init_cwm_watch_handupdown(etMems);
    }
}

/*sensor data input and running */
void cwm_call_watch_handupdown(void)
{

    struct InputDataWHUD algo_in;
    struct OutputDataWHUD algo_out;

    memset(&algo_in, 0, sizeof(algo_in));
    memset(&algo_out, 0, sizeof(algo_out));

    for (int i = 0; i < AccdataCount; i++)
    {
        algo_in.acc[0] = rawdata[i].acc_x;
        algo_in.acc[1] = rawdata[i].acc_y;
        algo_in.acc[2] = rawdata[i].acc_z;
        algo_in.dt_us = rawdata[i].dt;
        //DEBUG_HANDUP("TAG=DLOG3 SN(%d)DT(%d)A3(%d,%d,%d)\n",handupdown_sn,(int)algo_in.dt_us,(int)(algo_in.acc[0]*1000), (int)(algo_in.acc[1]*1000),(int)(algo_in.acc[2]*1000));   //sensor input data log
        int result = cwm_handupdown_FPU(etMems, &algo_in, &algo_out);
        // DEBUG_HANDUP("DEBUG: hand_up_ct = %d,hand_down_ct=%d \n",algo_out.watch_hand_up_ct,algo_out.watch_hand_down_ct);  //debug log

        if (result > 0)
        {
            int algo_result = algo_out.watch_handupdown_status;
            DEBUG_HANDUP("cwm_handupdown_FPU result = %d, handupdown = %d\n", result, algo_result);
            if (algo_result == 1)
            {
                void customio_notify_handup(void);
                customio_notify_handup();
                // DEBUG_HANDUP("hand up evevt trigger\n");
            }
            if (algo_result == 2)
            {
                void customio_notify_handdown(void);
                customio_notify_handdown();
                // DEBUG_HANDUP("hand down evevt trigger\n");
            }
        }
        handupdown_sn++;
    }
    memset(rawdata, 0, sizeof(rawdata));
    AccdataCount = 0;
}

/*DML sensor data output*/
void customio_hw_sensor_data_for_handup(const dml_sensorData_t *pSdo)
{
    if (AccdataCount >= RAW_MAX)
    {
        return;
    }

    if (pSdo->validData & DML_SENSOR_DATA_VALID_DT)
    {
        rawdata[AccdataCount].dt = pSdo->dt_us;
    }
    if (pSdo->validData & DML_SENSOR_DATA_VALID_ACC)
    {
        rawdata[AccdataCount].acc_x = pSdo->acc_data[0];
        rawdata[AccdataCount].acc_y = pSdo->acc_data[1];
        rawdata[AccdataCount].acc_z = pSdo->acc_data[2];
        AccdataCount++;
    }
}
#endif
