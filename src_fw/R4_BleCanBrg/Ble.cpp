// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Ble.cpp
 * @brief BLE (Bluetooth Low Energy) 通信制御実装
 */
#include "Ble.h"
#include "Can.h"
#include "Nvm.h"
#include <ArduinoBLE.h>
#include <Arduino_LED_Matrix.h>

// =============================================================================
// 定数定義
// =============================================================================
#define BLE_LED_FRAME_WORDS         3           // LEDフレームの32bitワード数

// =============================================================================
// LED Matrix (12x8) インスタンスおよびフレーム定義
// =============================================================================
static ArduinoLEDMatrix f_ledMatrix; // 12x8 LEDマトリクス制御インスタンス

// 未接続時フレーム: センターバー (中央4x2ドット)
static const uint32_t FRAME_BLE_DISCONNECTED[BLE_LED_FRAME_WORDS] = { // BLE未接続時LEDマトリクス表示フレーム (中央センターバー)
    0x00000000,
    0x00F00F00,
    0x00000000
};

// 接続中フレーム: リンクライン (全通12x2ライン)
static const uint32_t FRAME_BLE_CONNECTED[BLE_LED_FRAME_WORDS] = { // BLE接続中LEDマトリクス表示フレーム (全通リンクライン)
    0x00000000,
    0x0FFFFFF0,
    0x00000000
};

// 消灯フレーム
static const uint32_t FRAME_CLEAR[BLE_LED_FRAME_WORDS] = { // LEDマトリクス消灯フレーム
    0x00000000,
    0x00000000,
    0x00000000
};

// =============================================================================
// BLE サービス・キャラクタリスティック定義
// =============================================================================
static BLEService f_uartService(NUS_SERVICE_UUID); // Nordic UART Service (NUS) インスタンス
static BLECharacteristic f_rxCharacteristic(NUS_RX_CHAR_UUID, BLEWrite | BLEWriteWithoutResponse, BLE_BUFFER_SIZE); // BLE RXキャラクタリスティック (データ受信用)
static BLECharacteristic f_txCharacteristic(NUS_TX_CHAR_UUID, BLENotify, BLE_BUFFER_SIZE); // BLE TXキャラクタリスティック (通知配信用)

// BLE 接続状態フラグ (ファイル内ローカル)
static volatile bool f_isBleConnected = false; // BLE接続状態フラグ (true: 接続中)

// BLE 送信保留データ (送信失敗時の再試行用)
static ST_CAN_FRAME f_stPendingSendFrame; // 送信保留CANフレーム一時領域
static bool f_hasPendingSendFrame = false; // 送信保留フラグ (true: 送信待ちデータあり)

// =============================================================================
// BLE 初期化
// =============================================================================

/**
 * @brief BLEモジュールの初期化とNUS (Nordic UART Service) の設定を行う。
 * @param なし
 * @return true: 初期化成功, false: 初期化失敗
 */
bool BLE_Init(void) {
    if (!BLE.begin()) {
        return false;
    }

    // 12x8 LED Matrix 開始
    f_ledMatrix.begin();

    // デバイス名設定
    BLE.setLocalName(BLE_DEVICE_NAME);
    BLE.setAdvertisedService(f_uartService);

    // キャラクタリスティック追加
    f_uartService.addCharacteristic(f_rxCharacteristic);
    f_uartService.addCharacteristic(f_txCharacteristic);
    BLE.addService(f_uartService);

    // アドバタイズ開始
    BLE.advertise();
    return true;
}

// =============================================================================
// BLE 通信タスク [優先度: 2 (中)]
// =============================================================================

/**
 * @brief BLE通信・LED制御・死活監視タスク (FreeRTOS タスク、優先度: 2)。
 *        BLEポーリング、接続管理、BLE RX -> CAN Queue 転送、CAN Queue -> BLE TX Notify 転送、
 *        および 12x8 LED Matrix 表示制御・全タスク WDT リフレッシュを行う。
 * @param[in] pvParameters タスク起動パラメータ (未使用)
 * @return なし
 */
void BLE_Task(void* pvParameters) {
    (void)pvParameters;
    bool wasConnected = false; // 前回のBLE接続状態フラグ
    TickType_t xLastLedUpdateTick = xTaskGetTickCount(); // LED点滅・WDT監視前回実行時刻
    bool bLedBlinkState = true; // LED点滅トグル状態フラグ (初期値: 点灯)

    // 起動時の初期未接続フレーム (センターバー) を表示
    f_ledMatrix.loadFrame(FRAME_BLE_DISCONNECTED);

    for (;;) {
        // WDT死活監視チェックイン
        CMN_WdtCheckIn(WDT_TASK_ID_BLE);

        // BLEイベントポーリング
        BLE.poll();

        BLEDevice central = BLE.central(); // 接続先セントラルデバイス
        if (central && central.connected()) {
            if (!wasConnected) {
                wasConnected = true;
                f_isBleConnected = true;
                f_ledMatrix.loadFrame(FRAME_BLE_CONNECTED); // 接続中フレームを表示
            }

            // BLE RX -> CAN Queue 転送
            if (f_rxCharacteristic.written()) {
                int length = f_rxCharacteristic.valueLength(); // 受信データ長
                const uint8_t* data = f_rxCharacteristic.value(); // 受信データポインタ

                if (length > 0) {
                    // CAN設定を安全に取得
                    ST_CAN_CONFIG stCanConfig; // CAN設定データ一時領域
                    NVM_GetCanConfig(&stCanConfig);

                    // CANフレーム最大長ごとに分割してキューへ投入
                    const int MAX_CAN_DLC = CAN_FRAME_MAX_DLC; // CANフレーム最大データ長
                    for (int offset = 0; offset < length; offset += MAX_CAN_DLC) { // データ分割送信用オフセット
                        uint8_t dlc = (uint8_t)(((length - offset) < MAX_CAN_DLC) ? (length - offset) : MAX_CAN_DLC); // 今回送信DLC
                        ST_CAN_FRAME stFrame; // 送信用CANフレーム一時領域
                        memset(&stFrame, 0, sizeof(stFrame));
                        stFrame.id = stCanConfig.sendId;
                        stFrame.dlc = dlc;
                        stFrame.idType = stCanConfig.idType;
                        memcpy(stFrame.data, &data[offset], dlc);

                        if (g_xQueueBleToCan != NULL) {
                            TickType_t waitTicks = pdMS_TO_TICKS(BLE_QUEUE_SEND_TIMEOUT_MS); // キュー送信待ちtick数
                            if (xQueueSend(g_xQueueBleToCan, &stFrame, waitTicks > 0 ? waitTicks : CMN_MIN_DELAY_TICKS) != pdPASS) {
                                CMN_SetErrorBit(CMN_ERR_BIT_BUF_FULL_BLE_TO_CAN);
                            }
                        }
                    }
                }
            }

            // CAN Queue -> BLE TX Notify 転送 (保留バッファ＆次回ループ再試行方式)
            // 1. 送信待ち (保留中) のフレームがない場合のみ、キューから1フレーム取り出す
            if (!f_hasPendingSendFrame) {
                if (g_xQueueCanToBle != NULL) {
                    if (xQueueReceive(g_xQueueCanToBle, &f_stPendingSendFrame, 0) == pdTRUE) {
                        if (f_stPendingSendFrame.dlc > 0) {
                            f_hasPendingSendFrame = true; // 送信待ちフレームあり
                        }
                    }
                }
            }

            // 2. 送信待ちフレームがある場合、最大ペイロード (20byte超対応) を確認して送信を試みる
            if (f_hasPendingSendFrame) {
                uint16_t maxPayload = BLE_GetMaxNotifySize(); // 1回あたりの最大ペイロードサイズ
                uint8_t sendDlc = f_stPendingSendFrame.dlc; // 送信DLC
                if (sendDlc > maxPayload) {
                    sendDlc = (uint8_t)maxPayload; // 最大ペイロードでクリップ
                }

                int sent = f_txCharacteristic.writeValue((const uint8_t*)f_stPendingSendFrame.data, sendDlc); // 送信結果
                if (sent >= sendDlc) {
                    // 送信成功: 保留フラグを解除 (次回ループで次のフレームを取り出し可能)
                    f_hasPendingSendFrame = false;
                } else {
                    // 送信失敗 (BLEチップ満杯等): データを破棄せず保持したまま次回ループで再試行
                    CMN_SetErrorBit(CMN_ERR_BIT_BLE_SEND_ERR);
                }
            }

            // 500ms周期の接続維持・WDTリフレッシュ
            TickType_t xCurrentTick = xTaskGetTickCount(); // 現在tick数
            if ((xCurrentTick - xLastLedUpdateTick) >= pdMS_TO_TICKS(BLE_LED_BLINK_PERIOD_MS)) {
                xLastLedUpdateTick = xCurrentTick;
                CMN_WdtRefreshIfAllHealthy(); // 全タスク健全時にWDTリフレッシュ
            }

            CMN_SafeDelayMs(BLE_CONNECTED_POLL_DELAY_MS);
        } else {
            if (wasConnected) {
                wasConnected = false;
                f_isBleConnected = false;

                // 切断時に残存通信キューおよび送信保留フレームをクリア (再接続時の古いパケット混入を防止)
                f_hasPendingSendFrame = false;
                if (g_xQueueBleToCan != NULL) {
                    xQueueReset(g_xQueueBleToCan);
                }
                if (g_xQueueCanToBle != NULL) {
                    xQueueReset(g_xQueueCanToBle);
                }

                BLE.advertise();
                bLedBlinkState = true; // 切断時は即時センターバーを表示して点滅サイクル再開
                f_ledMatrix.loadFrame(FRAME_BLE_DISCONNECTED);
                xLastLedUpdateTick = xTaskGetTickCount();
            }

            // 500ms周期の未接続時LED点滅 (センターバー点灯 / 消灯トグル) & WDTリフレッシュ
            TickType_t xCurrentTick = xTaskGetTickCount(); // 現在tick数
            if ((xCurrentTick - xLastLedUpdateTick) >= pdMS_TO_TICKS(BLE_LED_BLINK_PERIOD_MS)) {
                xLastLedUpdateTick = xCurrentTick;
                bLedBlinkState = !bLedBlinkState;
                f_ledMatrix.loadFrame(bLedBlinkState ? FRAME_BLE_DISCONNECTED : FRAME_CLEAR);
                CMN_WdtRefreshIfAllHealthy(); // 全タスク健全時にWDTリフレッシュ
            }

            // 未接続時はスリープ
            CMN_SafeDelayMs(BLE_DISCONNECTED_POLL_DELAY_MS);
        }
    }
}

// =============================================================================
// 補助関数: 最大Notifyペイロードサイズ取得
// =============================================================================

/**
 * @brief 1回のNotifyで送信可能な最大ペイロードサイズを取得する。
 *        BLE接続中はキャラクタリスティック容量 (20バイト超の拡張MTU対応) を考慮した最大サイズを返し、
 *        未接続時は標準の安全なデフォルトペイロードサイズ (20バイト) を返す。
 * @param なし
 * @return 送信可能最大バイト数
 */
uint16_t BLE_GetMaxNotifySize(void) {
    if (f_isBleConnected) {
        uint16_t charSize = (uint16_t)f_txCharacteristic.valueSize(); // キャラクタリスティックバッファサイズ (バイト)
        if (charSize > BLE_GATT_HEADER_SIZE) {
            return charSize - BLE_GATT_HEADER_SIZE; // GATTヘッダを除外した最大実ペイロードサイズ
        }
    }
    return BLE_DEFAULT_MAX_PAYLOAD; // デフォルト最大実ペイロード (20バイト)
}
