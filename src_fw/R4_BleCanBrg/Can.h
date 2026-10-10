// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Can.h
 * @brief CAN 通信制御ヘッダ
 */
#pragma once

#include "Common.h"

// =============================================================================
// CAN 定義・定数
// =============================================================================

// CAN 通信ボーレート設定
#define CAN_BITRATE_125K            125000      // CAN 通信ボーレート 125 kbps
#define CAN_BITRATE_250K            250000      // CAN 通信ボーレート 250 kbps
#define CAN_BITRATE_500K            500000      // CAN 通信ボーレート 500 kbps (標準)
#define CAN_BITRATE_1000K           1000000     // CAN 通信ボーレート 1000 kbps (1 Mbps)

// CAN 通信仕様・デフォルト設定値
#define CAN_ID_TYPE_STANDARD        0           // Standard ID (11-bit)
#define CAN_ID_TYPE_EXTENDED        1           // Extended ID (29-bit)
#define CAN_ID_STANDARD_MAX         0x07FF      // Standard CAN ID 最大値 (11-bit: 0x000〜0x7FF)
#define CAN_ID_EXTENDED_MAX         0x1FFFFFFF  // Extended CAN ID 最大値 (29-bit: 0x00000000〜0x1FFFFFFF)

#define CAN_DEFAULT_BITRATE         CAN_BITRATE_500K // デフォルトCAN通信ボーレート (500 kbps)
#define CAN_DEFAULT_SEND_ID         0x0100      // デフォルト送信CAN ID
#define CAN_DEFAULT_ID_TYPE         CAN_ID_TYPE_STANDARD // デフォルトIDタイプ (Standard)

#define CAN_FRAME_MAX_DLC           8           // CANデータ長最大値 (バイト)
#define CAN_CONFIG_PAYLOAD_SIZE     9           // CAN設定通信データ部サイズ (bitRate:4 + sendId:4 + idType:1)

// CAN キュー・タスク設定 (FreeRTOS)
#define CAN_QUEUE_SIZE              16          // CAN送受信キュー長
#define CAN_RX_TASK_PRIORITY        3           // CAN受信タスク優先度 (高: ハードウェアFIFO吸い出し専任)
#define CAN_TX_TASK_PRIORITY        3           // CAN送信タスク優先度 (高: イベント駆動即時送出専任)
#define CAN_TASK_STACK_SIZE         200         // CANタスクスタックサイズ (words)
#define CAN_TX_QUEUE_WAIT_TIMEOUT_MS 200        // CAN送信キュー受信待機タイムアウト (ms)
#define CAN_TX_MAX_RETRY            10          // CAN送信リトライ最大回数
#define CAN_TX_QUEUE_WAIT_INIT_MS   100         // キュー未初期化時の待機時間 (ms)
#define CAN_RX_TASK_DELAY_TICKS     1           // CAN受信タスク休止tick数 (CPU明け渡し用)
#define CAN_TX_RETRY_DELAY_TICKS    1           // CAN送信リトライ待機tick数 (メールボックス空き待ち用)

// =============================================================================
// CAN 構造体定義
// =============================================================================
#pragma pack(1)
// CAN設定構造体 (不揮発メモリ保存用)
typedef struct {
    uint32_t magic;                            // マジック番号 ("R4CB")
    uint32_t bitRate;                          // CAN通信ボーレート (bps: 125000, 250000, 500000, 1000000)
    uint32_t sendId;                           // 送信CAN ID (デフォルト: 0x100)
    uint8_t  idType;                           // IDタイプ (0: Standard 11-bit, 1: Extended 29-bit)
    uint8_t  reserved[3];                      // アライメント用予約領域
    uint16_t checksum;                         // 設定データチェックサム
} ST_CAN_CONFIG;

// タスク間通信用 CANフレーム構造体 (14バイト)
typedef struct {
    uint32_t id;                               // CAN ID
    uint8_t  dlc;                              // データ長 (0〜8バイト)
    uint8_t  data[CAN_FRAME_MAX_DLC];          // ペイロードデータ
    uint8_t  idType;                           // IDタイプ (0: Standard, 1: Extended)
} ST_CAN_FRAME;
#pragma pack()

// =============================================================================
// CAN 通信制御
// =============================================================================
bool CAN_Init(void);
void CAN_RxTask(void* pvParameters);
void CAN_TxTask(void* pvParameters);
