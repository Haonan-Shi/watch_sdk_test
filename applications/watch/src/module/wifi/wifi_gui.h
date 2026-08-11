/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_GUI_H_
#define _WIFI_GUI_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** WiFi GUI to App event types */
typedef enum
{
    WIFI_GUI_CAMERA_ENTER = 0,     /**< Enter camera view */
    WIFI_GUI_CAMERA_EXIT = 1,      /**< Exit camera view */
    WIFI_GUI_POWER_ON = 2,         /**< Power on WiFi */

    /* WiFi Test GUI events */
    WIFI_GUI_TEST_ENTER = 10,      /**< Enter WiFi test */
    WIFI_GUI_TEST_EXIT = 11,       /**< Exit WiFi test */
    WIFI_GUI_TEST_SCAN = 12,       /**< Start WiFi scan */
    WIFI_GUI_TEST_IPERF_UPLOAD_START = 13,  /**< Start iperf upload */
    WIFI_GUI_TEST_IPERF_UPLOAD_STOP = 14,    /**< Stop iperf upload */
    WIFI_GUI_TEST_IPERF_DOWNLOAD_START = 15, /**< Start iperf download */
    WIFI_GUI_TEST_IPERF_DOWNLOAD_STOP = 16,   /**< Stop iperf download */
    WIFI_GUI_TEST_FILE_UPLOAD_START = 17,      /**< Start file upload */
    WIFI_GUI_TEST_FILE_UPLOAD_STOP = 18,       /**< Stop file upload */
    WIFI_GUI_TEST_FILE_DOWNLOAD_START = 19,    /**< Start file download */
    WIFI_GUI_TEST_FILE_DOWNLOAD_STOP = 20,     /**< Stop file download */
    WIFI_GUI_TEST_SSID_CONNECT = 21,   /**< Connect to SSID */
    WIFI_GUI_TEST_DISCONNECT = 22,     /**< Disconnect from AP */
} T_WIFI_GUI_TO_APP_TYPE;

void wifi_gui_to_app(T_WIFI_GUI_TO_APP_TYPE type, void *data);

/* WiFi Test callback function declarations */
void wifi_test_enter_proc(void *msg);
void wifi_test_exit_proc(void *msg);
void wifi_test_scan_proc(void *msg);
void wifi_test_iperf_upload_start_proc(void *msg);
void wifi_test_iperf_upload_stop_proc(void *msg);
void wifi_test_iperf_download_start_proc(void *msg);
void wifi_test_iperf_download_stop_proc(void *msg);
void wifi_test_file_upload_start_proc(void *msg);
void wifi_test_file_upload_stop_proc(void *msg);
void wifi_test_file_download_start_proc(void *msg);
void wifi_test_file_download_stop_proc(void *msg);
void wifi_test_ssid_connect_proc(void *msg);
void wifi_test_disconnect_proc(void *msg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIFI_GUI_H_ */
