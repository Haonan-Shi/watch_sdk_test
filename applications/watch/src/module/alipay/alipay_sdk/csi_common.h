#ifndef __CSI_COMMON_H__
#define __CSI_COMMON_H__

#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include <alipay_common.h>

typedef enum
{
    CSI_OK          =  0,
    CSI_ERROR       = -1,
    CSI_BUSY        = -2,
    CSI_TIMEOUT     = -3,
    CSI_UNSUPPORTED = -4
} csi_error_t;

typedef enum
{
    AES_KEY_LEN_BITS_128  = 128,
    AES_KEY_LEN_BITS_256  = 256
} csi_aes_key_bits_t;
csi_error_t csi_sha_init(void *context);

/*○ description:
    ■ uninit sha context
  ○ param
    ■ context - sha context to uninit
  ○ return
    ■ CSI_OK:success，otherwise:fail*/
csi_error_t csi_sha_uninit(void *context);

/*○ description:
    ■ start sha operate
  ○ param
    ■ context - sha context
        ■ mode - sha mode(sha256)
  ○ return
    ■ CSI_OK:success，otherwise:fail*/
csi_error_t csi_sha_start(void *context, uint32_t mode);

/*○ description:
    ■ finish sha operate
  ○ param
    ■ context - sha context
        ■ in - hash data buffer
    ■ size - hash data len
        ■ digest - digest data buffer
  ○ return
    ■ CSI_OK:success，otherwise:fail*/
csi_error_t csi_sha_finish(void *context, void *in, uint32_t size, void *digest);

/*○ description:
    ■ open asset and get asset handler
  ○ param
    ■ fd  - asset handler
    ■ asset_name - asset name
  ○ return
    ■ CSI_OK:success，otherwise:fail*/
csi_error_t csi_open_asset(void **fd, char asset_name[36]);

/*○ description:
    ■ read asset data
  ○ param
    ■ fd  - asset handler
    ■ offset - asset offset
    ■ buffer - buffer which store data to read
    ■ data_len - length of data read to buffer
  ○ return
    ■ CSI_OK:success，otherwise:fail*/
csi_error_t csi_read_asset(void *fd, uint32_t offset, void *buffer, uint32_t *data_len);

/*○ description:
    ■ close asset
  ○ param
    ■ fd - asset handler
  ○ return
    ■ CSI_OK:success，otherwise:fail*/
csi_error_t csi_close_asset(void *fd);
#define CHECK_UNUSED_ARG(a)

#endif