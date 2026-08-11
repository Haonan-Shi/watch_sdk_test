#include "fmna_platform_includes.h"
#include "fmna_timer_platform.h"
/*============================================================================*
 *                              Global Variables
 *============================================================================*/
TimerHandle_t adv_timer;

/*============================================================================*
 *                              Loacal Functions
 *============================================================================*/
static void adv_timer_callback(TimerHandle_t p_timer);

/**
 * @brief advertising timer callback
 *
 * adv_timer_callback is used to stop advertising after timeout
 *
 * @param p_timer - timer handler
 * @return none
 * @retval void
 */
void adv_timer_callback(TimerHandle_t p_timer)
{
    fmna_adv_platform_stop_adv();
}

/*============================================================================*
 *                              Global Functions
 *============================================================================*/
void sw_timer_init()
{
    /* adv_timer is used to stop advertising after timeout */
    if (false == os_timer_create(&adv_timer, "adv_timer",  1, \
                                 3000, false, adv_timer_callback))
    {
        APP_PRINT_INFO0("sw_timer_init: init adv_timer failed");
    }

}

ret_code_t app_timer_create(void **p_timer_id, app_timer_mode_t mode,
                            app_timer_timeout_handler_t timeout_handler)
{
    assert_param(p_timer_id);
    assert_param(timeout_handler);

    if (timeout_handler == NULL)
    {
        return 1;
    }

    if (false == os_timer_create(p_timer_id, "fmna_timer", 1, 1000, mode, timeout_handler))
    {
        return 1;
    }

    return 0;
}

ret_code_t app_timer_start(void *p_timer, uint32_t timeout_ms, void *p_context)
{
    assert_param(p_timer);
    if (timeout_ms == 0)
    {
        APP_PRINT_INFO0("app_timer_start: received a 0 timer");
        return 0;
    }
    if (false == os_timer_restart(&p_timer, timeout_ms))
    {
        return 1;
    }

    return 0;
}

ret_code_t app_timer_stop(void *p_timer)
{
    assert_param(p_timer);
//    uint32_t timer_state = 0;
//    os_timer_state_get(&p_timer, &timer_state);
//    if (timer_state)
    {
        if (false == os_timer_stop(&p_timer))
        {
            return 1;
        }
    }

    return 0;
}

