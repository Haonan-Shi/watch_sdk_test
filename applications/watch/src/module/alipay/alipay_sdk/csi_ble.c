
#include "csi_ble.h"
#include "csi_common.h"
#include "alipay_config.h"
#include "alipay_ble_transport.h"

#if CONFIG_ALIPAY

csi_error_t csi_ble_write(uint8_t *data, uint16_t len)
{
    AliPay_LOG("[Yuyin] csi_ble_write");

    alipay_ble_send(0x42, data, len);
    // ALIPAY_ble_write(data, len);
    return CSI_OK;
}

#endif //CONFIG_ALIPAY
