// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file R4_BleCanBrg.ino
 * @brief Arduino UNO R4 WiFi BLE-CAN ブリッジ メインスケッチ
 */
// =============================================================================
// 対応ボード制限ガード (Arduino UNO R4 WiFi 専用)
// =============================================================================
#if !defined(ARDUINO_UNOR4_WIFI)
#error "R4_BleCanBrg is only supported on Arduino UNO R4 WiFi. Please select 'Arduino UNO R4 WiFi' from Tools -> Board menu."
#endif

// Arduino UNO R4 公式同梱の FreeRTOS (これ1つで task.h, queue.h, timers.h 等を内包)
#include <Arduino_FreeRTOS.h>

#include <Arduino.h>
#include "Common.h"
#include "Nvm.h"
#include "Can.h"
#include "Ble.h"
#include "Cdc.h"

// =============================================================================
// スケッチ定数
// =============================================================================
#define BLE_INIT_FAIL_RETRY_DELAY_MS 500         // BLE初期化失敗時のリトライ・待機ディレイ (ms)
#define MAIN_LOOP_DELAY_MS           1000        // メインループ待機ディレイ (ms)

// =============================================================================
// タスクハンドル (ファイル内管理)
// =============================================================================
static TaskHandle_t f_hTaskCAN_Rx = NULL; // CAN受信タスクハンドル
static TaskHandle_t f_hTaskCAN_Tx = NULL; // CAN送信タスクハンドル
static TaskHandle_t f_hTaskBLE = NULL; // BLE通信タスクハンドル
static TaskHandle_t f_hTaskCdc = NULL; // USB CDC通信タスクハンドル

// =============================================================================
// セットアップ
// =============================================================================

/**
 * @brief マイコン起動時の初期化処理を行う。
 *        シリアル、LED、各種ハードウェア/通信モジュール (Common, NVM, CAN, BLE, CDC)、
 *        およびFreeRTOSタスクを生成・起動する。
 * @param なし
 * @return なし
 */
void setup() {
    Serial.begin(CDC_SERIAL_BAUDRATE);
    // 共通リソース初期化 (キュー生成等)
    CMN_Init();

    // 不揮発設定読み込み (EEPROM)
    NVM_Init();

    // CAN初期化 (D10: CANTX, D13: CANRX)
    CAN_Init();

    // BLEモジュール初期化 (失敗時は無限ループで待機)
    if (!BLE_Init()) {
        while (1) {
            delay(BLE_INIT_FAIL_RETRY_DELAY_MS);
        }
    }

    // USB CDC 初期化
    CDC_Init();

    // FreeRTOS タスク生成
    // CAN_RxTask: 高優先度 - CAN受信・ハードウェアFIFO吸い出し専任
    xTaskCreate(CAN_RxTask, "CAN_RxTask", CAN_TASK_STACK_SIZE, NULL, CAN_RX_TASK_PRIORITY, &f_hTaskCAN_Rx);

    // CAN_TxTask: 高優先度 - 完全イベント駆動 (0ms即時送出専任)
    xTaskCreate(CAN_TxTask, "CAN_TxTask", CAN_TASK_STACK_SIZE, NULL, CAN_TX_TASK_PRIORITY, &f_hTaskCAN_Tx);

    // BLE_Task: 中優先度 - BLE通信 & 接続管理 & LED制御・WDT監視専任
    xTaskCreate(BLE_Task, "BLE_Task", BLE_TASK_STACK_SIZE, NULL, BLE_TASK_PRIORITY, &f_hTaskBLE);

    // CDC_Task: 低優先度 - PCシリアル通信専任
    xTaskCreate(CDC_Task, "CDC_Task", CDC_TASK_STACK_SIZE, NULL, CDC_TASK_PRIORITY, &f_hTaskCdc);

    // ウォッチドッグタイマ (WDT) 初期化 & 開始 (タイムアウト: 5000ms)
    CMN_WdtInit();

    // スケジューラ明示起動
    vTaskStartScheduler();
}

// =============================================================================
// メインループ
// =============================================================================

/**
 * @brief Arduinoメインループ。FreeRTOS制御下のため通常は休止する。
 * @param なし
 * @return なし
 */
void loop() {
    // FreeRTOSタスクがすべて動作しているため、万一loopが呼ばれた場合も安全に休止
    CMN_SafeDelayMs(MAIN_LOOP_DELAY_MS);
}
