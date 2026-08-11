///////////////////////////////////
/*
   FPU Version
*/
///////////////////////////////////
#ifndef __HANDUPDOWN_FPU_H__
#define __HANDUPDOWN_FPU_H__

#ifdef __cplusplus
extern "C" {
#endif

//------------------ Algorithms input definition --------------------- //
struct InputDataWHUD
{
    float acc[3];   // unit: m/s^2   ,  9.8m/s^2 for 1 gravity
    float dt_us; // time difference between each algorithm call(us)
};

//------------------ Algorithms output definition --------------------- //
struct OutputDataWHUD
{
    int watch_handupdown_status;     // (event 0: nothing; event 1: handup; event 2: handdown )
    int watch_hand_up_ct;            // (hand up count)
    int watch_hand_down_ct;          // (hand down count)
    int wrist_status;
};

/*
    *
    *   Initialize all variables
    *
*/
void init_cwm_watch_handupdown(void *mem);

int cwm_handupdown_FPU(void *mem, struct InputDataWHUD *input, struct OutputDataWHUD *output);

#ifdef __cplusplus
}
#endif

#endif

