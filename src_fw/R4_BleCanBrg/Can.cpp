// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Can.cpp
 * @brief CAN 通信制御実装
 */
#include "Can.h"
#include "Nvm.h"
#include <Arduino_CAN.h>

// =============================================================================
// 定数定義
// =============================================================================
#define CAN_WRITE_SUCCESS_MIN       0           // CAN書き込み成功判定値 (0以上で成功)

// =============================================================================
// CAN 初期化
// =============================================================================

/**
 * @brief CANコントローラの初期化を行う。
 *        EEPROMの設定値(ボーレート等)を読み込み、CANモジュールの開始を行う。
 * @param なし
 * @return true: 初期化成功, false: 初期化失敗
 */
bool CAN_Init(void) {
    ST_CAN_CONFIG stCanConfig; // 現在のCAN設定データ一時領域
    NVM_GetCanConfig(&stCanConfig);

    CanBitRate br = CanBitRate::BR_500k; // CANボーレート設定値
    switch (stCanConfig.bitRate) {
        case CAN_BITRATE_125K:  br = CanBitRate::BR_125k; break;
        case CAN_BITRATE_250K:  br = CanBitRate::BR_250k; break;
        case CAN_BITRATE_500K:  br = CanBitRate::BR_500k; break;
        case CAN_BITRATE_1000K: br = CanBitRate::BR_1000k; break;
        default:
            br = CanBitRate::BR_500k;
            break;
    }

    if (!CAN.begin(br)) {
        CMN_SetErrorBit(CMN_ERR_BIT_CAN_INIT_ERR);
        return false;
    }

    return true;
}

// =============================================================================
// CAN 受信タスク [優先度: 3 (高)] - ハードウェアFIFO吸い出し専任
// =============================================================================

/**
 * @brief CAN受信専任タスク(FreeRTOS タスク、優先度: 3)。
 *        CANバスから届いたフレームを吸い出し、BLE通知用キューへ最速投入する。
 * @param[in] pvParameters タスク起動パラメータ(未使用)
 * @return なし
 */
void CAN_RxTask(void* pvParameters) {
    (void)pvParameters;

    for (;;) {
        // WDT死活監視チェックイン
        CMN_WdtCheckIn(WDT_TASK_ID_CAN_RX);

        // CAN受信処理 (CAN -> Queue)
        while (CAN.available()) {
            CanMsg const msg = CAN.read(); // CANバス受信メッセージ
            if (msg.data_length > 0) {
                ST_CAN_FRAME stRxFrame; // BLE転送用CANフレーム一時領域
                memset(&stRxFrame, 0, sizeof(stRxFrame));
                bool isExt = msg.isExtendedId(); // 拡張ID判定フラグ
                stRxFrame.id = isExt ? msg.getExtendedId() : msg.getStandardId();
                uint8_t dlc = (uint8_t)msg.data_length; // 受信データ長
                if (dlc > CAN_FRAME_MAX_DLC) {
                    dlc = CAN_FRAME_MAX_DLC; // 8バイト上限クリップ (バッファオーバーラン防止)
                }
                stRxFrame.dlc = dlc;
                stRxFrame.idType = isExt ? CAN_ID_TYPE_EXTENDED : CAN_ID_TYPE_STANDARD;
                memcpy(stRxFrame.data, msg.data, stRxFrame.dlc);

                // BLE転送用キューに投入 (満杯ならドロップしエラービットセット)
                if (g_xQueueCanToBle != NULL) {
                    if (xQueueSend(g_xQueueCanToBle, &stRxFrame, 0) != pdPASS) {
                        CMN_SetErrorBit(CMN_ERR_BIT_BUF_FULL_CAN_TO_BLE);
                    }
                }
            }
        }

        // 確実にBlocked状態へ移行し、CPUを他タスクへ明け渡す (飢餓防止)
        vTaskDelay(CAN_RX_TASK_DELAY_TICKS);
    }
}

// =============================================================================
// CAN 送信タスク [優先度: 3 (高)] - 完全イベント駆動 (0ms即時送出)
// =============================================================================

/**
 * @brief CAN送信専任タスク(FreeRTOS タスク、優先度: 3)。
 *        BLEからキューにデータが投入された瞬間に 0ms (遅延ゼロ) で起床し、CANバスへ即座に送出する。
 *        データがない間は完全休止 (Blocked: CPU消費0%)。
 * @param[in] pvParameters タスク起動パラメータ(未使用)
 * @return なし
 */
void CAN_TxTask(void* pvParameters) {
    (void)pvParameters;
    ST_CAN_FRAME stTxFrame; // CAN送信フレーム一時領域

    for (;;) {
        // WDT死活監視チェックイン
        CMN_WdtCheckIn(WDT_TASK_ID_CAN_TX);

        // キューからイベント駆動で受信 (データ到着で即時0ms起床、アイドル時は待機)
        if (g_xQueueBleToCan != NULL) {
            if (xQueueReceive(g_xQueueBleToCan, &stTxFrame, pdMS_TO_TICKS(CAN_TX_QUEUE_WAIT_TIMEOUT_MS)) == pdTRUE) {
                int writeResult = -1; // CAN送信戻り値
                int retry = 0; // リトライ回数カウンタ

                do {
                    if (stTxFrame.idType == CAN_ID_TYPE_EXTENDED) {
                        CanMsg const msg(CanExtendedId(stTxFrame.id), stTxFrame.dlc, stTxFrame.data); // 送信メッセージ (Extended)
                        writeResult = CAN.write(msg);
                    } else {
                        CanMsg const msg(CanStandardId(stTxFrame.id), stTxFrame.dlc, stTxFrame.data); // 送信メッセージ (Standard)
                        writeResult = CAN.write(msg);
                    }

                    // メールボックスが満杯の場合は待機して再試行 (この間も CAN_RxTask は独立して動作可能)
                    if (writeResult < CAN_WRITE_SUCCESS_MIN) {
                        vTaskDelay(CAN_TX_RETRY_DELAY_TICKS);
                        retry++;
                    }
                } while (writeResult < CAN_WRITE_SUCCESS_MIN && retry < CAN_TX_MAX_RETRY);

                if (writeResult < CAN_WRITE_SUCCESS_MIN) {
                    CMN_SetErrorBit(CMN_ERR_BIT_CAN_SEND_ERR);
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(CAN_TX_QUEUE_WAIT_INIT_MS));
        }
    }
}
