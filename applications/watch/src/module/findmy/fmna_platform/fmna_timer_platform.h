
#ifndef fmna_timer_platform_h
#define fmna_timer_platform_h

/*============================================================================*
 *                        Header Files
 *============================================================================*/
//#include <board.h>
#include "os_timer.h"
#include "rtl876x.h"
#include "fmna_adv_platform.h"
/*============================================================================*
 *                              Macros
 *============================================================================*/
/**
 * @brief Create a timer identifier and statically allocate memory for the timer.
 *
 * @param timer_id Name of the timer identifier variable that will be used to control the timer.
 */
#define APP_TIMER_DEF(timer_id) _APP_TIMER_DEF(timer_id)

#define _APP_TIMER_DEF(timer_id)               \
    static TimerHandle_t (timer_id)

#define APP_TIMER_TICKS(MS) (MS)
#define APP_TIMER_MIN_TIMEOUT_TICKS 10

/*============================================================================*
 *                         Types
 *============================================================================*/
typedef void *TimerHandle_t;
typedef uint32_t ret_code_t;

/**@brief Timer modes. */
typedef enum
{
    APP_TIMER_MODE_SINGLE_SHOT,                 /**< The timer will expire only once. */
    APP_TIMER_MODE_REPEATED                     /**< The timer will restart each time it expires. */
} app_timer_mode_t;

/**@brief Application time-out handler type. */
typedef void (*app_timer_timeout_handler_t)(void *p_context);

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern TimerHandle_t adv_timer;

/*============================================================================*
 *                         Functions
 *============================================================================*/
void sw_timer_init(void);

ret_code_t app_timer_create(void       **p_timer_id,  \
                            app_timer_mode_t            mode,        \
                            app_timer_timeout_handler_t timeout_handler);

ret_code_t app_timer_start(void *p_timer, uint32_t timeout_ms, void *p_context);

ret_code_t app_timer_stop(void *p_timer);

#endif
