// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Ble.h
 * @brief BLE (Bluetooth Low Energy) 通信制御ヘッダ
 */
#pragma once

#include "Common.h"

// =============================================================================
// BLE 定義・定数
// =============================================================================

// Nordic UART Service (NUS) 128-bit UUID
#define NUS_SERVICE_UUID            "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // NUS サービスUUID
#define NUS_RX_CHAR_UUID            "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // NUS RXキャラクタリスティックUUID (書き込み用)
#define NUS_TX_CHAR_UUID            "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // NUS TXキャラクタリスティックUUID (通知用)

// BLE デバイス設定
#define BLE_DEVICE_NAME             FW_NAME     // BLE ローカル名 (アドバタイズ名: "R4_BleCanBrg")

// BLE 最大バッファサイズ・ペイロード定数
#define BLE_BUFFER_SIZE             256         // BLE最大送受信バッファサイズ (バイト)
#define BLE_GATT_HEADER_SIZE        3           // GATTヘッダサイズ (バイト)
#define BLE_DEFAULT_MAX_PAYLOAD     20          // BLE標準の1パケット最大実ペイロード (デフォルトMTU 23 - GATTヘッダ 3)

// BLE タスク・LED制御設定 (FreeRTOS)
#define BLE_TASK_PRIORITY           2           // BLE通信タスク優先度 (中)
#define BLE_TASK_STACK_SIZE         384         // BLEタスクスタックサイズ (words)
#define BLE_LED_BLINK_PERIOD_MS     500         // LED点滅・WDT監視更新周期 (ms)
#define BLE_QUEUE_SEND_TIMEOUT_MS   10          // BLE->CANキュー送信タイムアウト (ms)
#define BLE_CONNECTED_POLL_DELAY_MS 5           // BLE接続中のポーリング休止時間 (ms)
#define BLE_DISCONNECTED_POLL_DELAY_MS 20       // BLE未接続時の待機休止時間 (ms)

// =============================================================================
// BLE 通信制御 & LEDインジケータ
// =============================================================================
bool BLE_Init(void);
void BLE_Task(void* pvParameters);
uint16_t BLE_GetMaxNotifySize(void);
