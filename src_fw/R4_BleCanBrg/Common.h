// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Common.h
 * @brief 共通定義・エラーコード・グローバル変数宣言ヘッダ
 */
#pragma once

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

// =============================================================================
// システム基本情報
// =============================================================================
#define FW_NAME             "R4_BleCanBrg"    // ファームウェア名
#define MAKER_NAME          "SHIOMACHI_SOFT"  // メーカー名
#define FW_VER              0x26091400        // ファームウェアバージョン

#define FW_NAME_BUF_SIZE    16                // ファームウェア名バッファサイズ
#define MAKER_NAME_BUF_SIZE 16                // メーカー名バッファサイズ
#define DEVICE_UID_SIZE     16                // マイコン固有識別子 (UID) サイズ (バイト)

// =============================================================================
// PC (JigApp) 通信プロトコル定義
// =============================================================================
// フレーム定数
#define FRM_HEADER_REQ      0xA0              // 要求フレームヘッダ
#define FRM_HEADER_RES      0xA1              // 応答フレームヘッダ
#define FRM_DATA_MAX_SIZE   1024              // データ部最大サイズ (バイト)
#define FRM_REQ_HEADER_SIZE 7                 // 要求フレーム最小ヘッダサイズ (header:1 + seqNo:2 + cmd:2 + dataSize:2)
#define FRM_RES_HEADER_SIZE 9                 // 応答フレーム最小ヘッダサイズ (header:1 + seqNo:2 + cmd:2 + errCode:2 + dataSize:2)
#define FRM_CHECKSUM_SIZE   sizeof(uint16_t)  // フレームチェックサムサイズ (2バイト)

// コマンド定義
#define CMD_GET_FW_INFO     0x0001            // FW情報取得コマンド
#define CMD_GET_FW_ERR      0x0002            // FWエラー取得コマンド
#define CMD_CLEAR_FW_ERR    0x0003            // FWエラークリアコマンド
#define CMD_ERASE_EEPROM    0x0004            // EEPROM消去コマンド
#define CMD_SET_CAN_CONFIG  0x0005            // CAN通信設定変更コマンド
#define CMD_GET_CAN_CONFIG  0x0006            // CAN通信設定取得コマンド

// エラーコード
#define FRM_ERR_SUCCESS     0x0000            // 成功
#define FRM_ERR_DATA_SIZE   0x0001            // データサイズ不正
#define FRM_ERR_PARAM       0x0002            // 引数不正

// 送信完了待ち時間・タイムアウト・遅延設定
#define CMD_WAIT_SEND_END           1000      // リセット前の応答フレーム送信完了待ち時間 (ms)
#define CDC_RECV_TIMEOUT_MS         1500      // 要求フレーム受信タイムアウト (ms)
#define CMN_MIN_DELAY_TICKS         1         // 最小遅延tick数

// ウォッチドッグタイマ (WDT) 設定
#define WDT_TIMEOUT_MS              5000      // WDTハードウェアタイムアウト時間 (ms)
#define WDT_TASK_ID_CAN_RX          0         // CAN受信タスクID
#define WDT_TASK_ID_CAN_TX          1         // CAN送信タスクID
#define WDT_TASK_ID_BLE             2         // BLEタスクID
#define WDT_TASK_ID_CDC             3         // CDCタスクID
#define WDT_ALL_TASKS_MASK          ((1 << WDT_TASK_ID_CAN_RX) | (1 << WDT_TASK_ID_CAN_TX) | (1 << WDT_TASK_ID_BLE) | (1 << WDT_TASK_ID_CDC)) // 全タスク健全マスク (0b1111)

// ハードウェアレジスタ制御 (RA4M1)
#define PRCR_KEY_UNLOCK_RSTSR       0xA508    // PRC3 (リセット機能レジスタ書き込み保護) 解除キー
#define PRCR_KEY_LOCK               0xA500    // PRCR 書き込みプロテクト設定キー
#define RSTSR1_WDT_RESET_MASK       0x03      // RSTSR1 WDT/IWDTリセット検知フラグマスク (WDTRF | IWDTRF)

// =============================================================================
// FWエラービット定義
// =============================================================================
#define CMN_ERR_BIT_CAN_SEND_ERR        (1 << 0) // CAN送信エラー
#define CMN_ERR_BIT_BLE_SEND_ERR        (1 << 1) // BLE送信エラー
#define CMN_ERR_BIT_CAN_INIT_ERR        (1 << 2) // CAN初期化エラー
#define CMN_ERR_BIT_BUF_FULL_BLE_TO_CAN (1 << 3) // BLE->CANキュー満杯によるパケット破棄
#define CMN_ERR_BIT_BUF_FULL_CAN_TO_BLE (1 << 4) // CAN->BLEキュー満杯によるパケット破棄
#define CMN_ERR_BIT_WDT_RESET           (1 << 5) // WDTタイムアウトでマイコンがリセットした

// =============================================================================
// 共通構造体定義 (JigAppプロトコル)
// =============================================================================
#pragma pack(1)
// FW情報構造体 (合計52バイト: JigAppのSendCmd_GetFwInfoに完全準拠)
typedef struct {
    char     szMakerName[MAKER_NAME_BUF_SIZE]; // メーカー名文字列
    char     szFwName[FW_NAME_BUF_SIZE];       // ファームウェア名文字列
    uint32_t fwVer;                            // ファームウェアバージョン
    uint8_t  device_uid[DEVICE_UID_SIZE];      // マイコン固有識別子 (UID)
} ST_FW_INFO;

// 要求フレーム
typedef struct {
    uint8_t  header;                           // 要求ヘッダ (0xA0)
    uint16_t seqNo;                            // シーケンス番号
    uint16_t cmd;                              // コマンドID
    uint16_t dataSize;                         // データ部サイズ (バイト)
    uint8_t  aData[FRM_DATA_MAX_SIZE];         // データ部バッファ
    uint16_t checksum;                         // フレームチェックサム
} ST_FRM_REQ_FRAME;

// 応答フレーム
typedef struct {
    uint8_t  header;                           // 応答ヘッダ (0xA1)
    uint16_t seqNo;                            // シーケンス番号
    uint16_t cmd;                              // コマンドID
    uint16_t errCode;                          // エラーコード
    uint16_t dataSize;                         // データ部サイズ (バイト)
    uint8_t  aData[FRM_DATA_MAX_SIZE];         // データ部バッファ
    uint16_t checksum;                         // フレームチェックサム
} ST_FRM_RES_FRAME;
#pragma pack()

// =============================================================================
// FreeRTOS 共有リソース宣言
// =============================================================================
extern QueueHandle_t g_xQueueBleToCan;          // BLE -> CAN 転送用FreeRTOSキュー
extern QueueHandle_t g_xQueueCanToBle;          // CAN -> BLE 転送用FreeRTOSキュー

extern volatile uint32_t g_fwErrorBits;         // FWエラービットフラグ

// =============================================================================
// 共通関数
// =============================================================================
void CMN_Init(void);
void CMN_WdtInit(void);
void CMN_WdtCheckIn(uint8_t taskId);
void CMN_WdtRefreshIfAllHealthy(void);
void CMN_SetErrorBit(uint32_t bit);
uint32_t CMN_GetErrorBits(void);
void CMN_ClearErrorBits(void);
void CMN_ResetMcu(void);
void CMN_SafeDelayMs(uint32_t ms);
uint16_t CMN_CalcChecksum(const void* pBuf, size_t size);
