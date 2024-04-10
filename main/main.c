#include <stdio.h>
#include <nvs_flash.h>
#include "esp_check.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#define GATTS_TAG "GATTS_DEMO"

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xEE,
    0x00,
    0x00,
    0x00,
    // second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

// 都是在 广播包-数据包-结构体类型 可了解详情 ?连接间隔不清楚?
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,                                                // 扫描是否响应
    .include_name = true,                                                 // 是否显示设备名
    .include_txpower = true,                                              // 是否显示发射功率
    .min_interval = 0x0006,                                               // 从机连接最小间隔 Time = min_interval * 1.25 msec
    .max_interval = 0x000C,                                               // 从机连接最大间隔 Time = max_interval * 1.25 msec
    .appearance = 0x00,                                                   // 设备外观??? 猜测应该是描述是什么设备 耳机还是手机还是电脑
    .manufacturer_len = 0,                                                // 制造商数据长度,
    .p_manufacturer_data = NULL,                                          // 制造商数据 描述你是谷歌的还是华为的还是无名过客
    .service_data_len = 0,                                                // 服务数据长度
    .p_service_data = NULL,                                               // 服务信息
    .service_uuid_len = 32,                                               // 服务uuid长度(byte) 因为有2个服务uuid 所所以32byte 一个uuid长128bit=16byte
    .p_service_uuid = adv_service_uuid128,                                // 服务数据 uuid
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT), // 发现模式
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND, // 广播报文类型 通用广播 在广播包-header-PDU部分
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map = ADV_CHNL_ALL,                            // 广播哪几个信道 全部 其实BLUE4.2总共就3个广播信道，详见蓝牙协议
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, // 操作策略 可被扫描与连接
};
void ble_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&adv_params);

        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(GATTS_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;

    default:
        break;
    }
}

void app_main(void)
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // ble
    esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&cfg));
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    esp_bluedroid_init_with_cfg(&bluedroid_cfg);
    esp_bluedroid_enable();
    esp_ble_gap_register_callback(ble_gap_callback);
    esp_ble_gatts_app_register(0);
    esp_ble_gap_set_device_name("BLE_TEST");
    esp_ble_gap_config_adv_data(&adv_data);
    esp_ble_gap_start_advertising(&adv_params); // 可以写在回调事件中 广播配置完成事件时进行启动广播
    esp_ble_gatt_set_local_mtu(500);
}