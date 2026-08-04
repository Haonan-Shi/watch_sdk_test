#include "alipay_common.h"

#if CONFIG_ALIPAY

static char tmp_ram_save_buffer[3 * 1024] = {0,};
static uint32_t tmp_save_length = 0;

ALIPAY_WEAK_SYMBOL
int alipay_secure_save_data(PARAM_IN void *data, PARAM_IN uint32_t data_len)
{
//#warning alipay_secure_save_data demo
    if ((data == NULL) || (data_len == 0))
    {
        return -1;
    }
    if (data_len > sizeof(tmp_ram_save_buffer))
    {
        return -1;
    }
    memcpy(tmp_ram_save_buffer, data, data_len);
    tmp_save_length = data_len;
    return 0;
}

#endif //CONFIG_ALIPAY