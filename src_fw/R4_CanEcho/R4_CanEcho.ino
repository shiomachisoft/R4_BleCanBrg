// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file R4_CanEcho.ino
 * @brief Arduino UNO R4 Minima CAN Echo (折り返し通信) プログラム
 */

// =============================================================================
// 対応ボード制限ガード (Arduino UNO R4 Minima 専用)
// =============================================================================
#if !defined(ARDUINO_UNOR4_MINIMA)
#error "R4_CanEcho is only supported on Arduino UNO R4 Minima. Please select 'Arduino UNO R4 Minima' from Tools -> Board menu."
#endif

#include <Arduino_CAN.h>
#include <EEPROM.h>
#include "bsp_api.h"

// =============================================================================
// システム基本情報定数 (define定数)
// =============================================================================
#define FW_NAME                     "R4_CanEcho"      // ファームウェア名
#define MAKER_NAME                  "SHIOMACHI_SOFT"  // メーカー名
#define FW_VER                      0x26091400        // ファームウェアバージョン

#define FW_NAME_BUF_SIZE            16                // ファームウェア名バッファサイズ
#define MAKER_NAME_BUF_SIZE         16                // メーカー名バッファサイズ
#define DEVICE_UID_SIZE             16                // マイコン固有識別子 (UID) サイズ (バイト)

// =============================================================================
// PC (JigApp) 通信プロトコル定数
// =============================================================================
#define FRM_HEADER_REQ              0xA0              // 要求フレームヘッダ
#define FRM_HEADER_RES              0xA1              // 応答フレームヘッダ
#define FRM_DATA_MAX_SIZE           1024              // データ部最大サイズ (バイト)
#define FRM_REQ_HEADER_SIZE         7                 // 要求フレーム最小ヘッダサイズ (header:1 + seqNo:2 + cmd:2 + dataSize:2)
#define FRM_RES_HEADER_SIZE         9                 // 応答フレーム最小ヘッダサイズ (header:1 + seqNo:2 + cmd:2 + errCode:2 + dataSize:2)
#define FRM_CHECKSUM_SIZE           sizeof(uint16_t)  // フレームチェックサムサイズ (2バイト)
#define FRM_NO_DATA_SIZE            0                 // データ部なしサイズ (0バイト)

#define CMD_GET_FW_INFO             0x0001            // FW情報取得コマンド
#define CMD_GET_FW_ERR              0x0002            // FWエラー取得コマンド
#define CMD_CLEAR_FW_ERR            0x0003            // FWエラークリアコマンド
#define CMD_ERASE_EEPROM            0x0004            // EEPROM消去コマンド
#define CMD_SET_CAN_CONFIG          0x0005            // CAN通信設定変更コマンド
#define CMD_GET_CAN_CONFIG          0x0006            // CAN通信設定取得コマンド

#define FRM_ERR_SUCCESS             0x0000            // 成功
#define FRM_ERR_DATA_SIZE           0x0001            // データサイズ不正
#define FRM_ERR_PARAM               0x0002            // 引数不正

#define CMD_WAIT_SEND_END_MS        1000              // 応答フレーム送信完了待ち時間 (ms)
#define CDC_RECV_TIMEOUT_MS         1500              // 要求フレーム受信タイムアウト (ms)
#define CDC_SERIAL_BUFFER_EXTRA     32                // シリアルバッファ予備サイズ (バイト)

// =============================================================================
// CAN 通信仕様・設定定数
// =============================================================================
#define CAN_BITRATE_125K            125000            // CAN 通信ボーレート 125 kbps
#define CAN_BITRATE_250K            250000            // CAN 通信ボーレート 250 kbps
#define CAN_BITRATE_500K            500000            // CAN 通信ボーレート 500 kbps (標準)
#define CAN_BITRATE_1000K           1000000           // CAN 通信ボーレート 1000 kbps (1 Mbps)

#define CAN_ID_TYPE_STANDARD        0                 // Standard ID (11-bit)
#define CAN_ID_TYPE_EXTENDED        1                 // Extended ID (29-bit)
#define CAN_ID_STANDARD_MAX         0x07FF            // Standard CAN ID 最大値 (11-bit: 0x000〜0x7FF)
#define CAN_ID_EXTENDED_MAX         0x1FFFFFFF        // Extended CAN ID 最大値 (29-bit: 0x00000000〜0x1FFFFFFF)

#define CAN_DEFAULT_BITRATE         CAN_BITRATE_500K  // デフォルトCAN通信ボーレート (500 kbps)
#define CAN_DEFAULT_SEND_ID         0x0101            // デフォルトエコー返信CAN ID
#define CAN_DEFAULT_RECV_ID         0x0100            // デフォルト受信対象CAN ID
#define CAN_DEFAULT_ID_TYPE         CAN_ID_TYPE_STANDARD // デフォルトIDタイプ (Standard)

#define CAN_FRAME_MAX_DLC           8                 // CANデータ長最大値 (バイト)
#define CAN_CONFIG_PAYLOAD_SIZE     13                // CanEcho用CAN設定通信データ部サイズ (bitRate:4 + sendId:4 + idType:1 + recvId:4)

// =============================================================================
// 不揮発性メモリ (EEPROM) 定数
// =============================================================================
#define EEPROM_MAGIC                0x52344543        // マジック番号 ("R4EC": R4 Echo Can)
#define EEPROM_ADDR_CAN_CONFIG      0                 // CAN設定保存先EEPROM先頭アドレス

// =============================================================================
// 動作制御・タイミング定数
// =============================================================================
#define ECHO_SERIAL_BAUDRATE        115200            // シリアル通信ボーレート (bps)
#define ECHO_TX_MAX_RETRY           10                // 送信メールボックス空き待ちリトライ回数
#define ECHO_TX_RETRY_DELAY_US      500               // リトライ間隔 (μs)
#define ECHO_LED_BLINK_INTERVAL_MS  500               // LED点滅間隔 (ms)
#define ECHO_WRITE_SUCCESS_MIN      0                 // CAN書き込み成功判定値 (0以上で成功)

// =============================================================================
// FWエラービット定義 (JigAppプロトコル)
// =============================================================================
#define ECHO_ERR_BIT_CAN_SEND_ERR   (1 << 0)          // CAN送信エラー
#define ECHO_ERR_BIT_CAN_INIT_ERR   (1 << 2)          // CAN初期化エラー

// =============================================================================
// 構造体定義
// =============================================================================
#pragma pack(1)
// FW情報構造体 (合計52バイト: JigAppプロトコル完全準拠)
typedef struct {
    char     szMakerName[MAKER_NAME_BUF_SIZE]; // メーカー名文字列
    char     szFwName[FW_NAME_BUF_SIZE];       // ファームウェア名文字列
    uint32_t fwVer;                            // ファームウェアバージョン
    uint8_t  device_uid[DEVICE_UID_SIZE];      // マイコン固有識別子 (UID)
} ST_FW_INFO;

// CAN設定構造体 (EEPROM保存用: 20バイト)
typedef struct {
    uint32_t magic;                            // マジック番号 ("R4EC")
    uint32_t bitRate;                          // CAN通信ボーレート (bps)
    uint32_t sendId;                           // 送信CAN ID (エコー返信用)
    uint8_t  idType;                           // IDタイプ (0: Standard, 1: Extended)
    uint32_t recvId;                           // 受信CAN ID (受信対象)
    uint8_t  reserved[3];                      // アライメント用予約領域
    uint16_t checksum;                         // 設定データチェックサム
} ST_CAN_CONFIG;

// 要求フレーム構造体
typedef struct {
    uint8_t  header;                           // 要求ヘッダ (0xA0)
    uint16_t seqNo;                            // シーケンス番号
    uint16_t cmd;                              // コマンドID
    uint16_t dataSize;                         // データ部サイズ (バイト)
    uint8_t  aData[FRM_DATA_MAX_SIZE];         // データ部バッファ
    uint16_t checksum;                         // フレームチェックサム
} ST_FRM_REQ_FRAME;

// 応答フレーム構造体
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
// グローバル変数
// =============================================================================
static ST_CAN_CONFIG g_stCanConfig; // CAN通信設定データ (RAM上の実体)
static uint32_t g_lastLedToggleMs = 0; // LED点滅制御用前回切り替え時刻 (ms)
static bool g_ledState = false; // 内蔵LED点灯状態フラグ
static uint32_t g_fwErrBits = 0; // FWエラービット (エラー情報保持用)

// シリアル受信バッファ (JigAppコマンド処理用)
static uint8_t g_serialRxBuf[sizeof(ST_FRM_REQ_FRAME) + CDC_SERIAL_BUFFER_EXTRA]; // シリアル受信バッファ
static size_t g_serialRxLen = 0; // シリアル受信データ長
static uint32_t g_serialRxLastTime = 0; // 最終受信時刻 (ms)

// シリアル送信バッファ
static uint8_t g_serialTxBuf[sizeof(ST_FRM_RES_FRAME)]; // シリアル送信バッファ

// =============================================================================
// プロトタイプ宣言 (内部関数)
// =============================================================================
void CAN_InitController(void);
void CAN_ProcessEcho(void);
void CDC_ProcessSerialCommand(void);
void CDC_ExecReqCmd_GetFwInfo(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
void CDC_ExecReqCmd_GetCanConfig(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
void CDC_ExecReqCmd_SetCanConfig(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
void CDC_ExecReqCmd_EraseEeprom(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
void CDC_ExecReqCmd_GetFwErr(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
void CDC_ExecReqCmd_ClearFwErr(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
void CDC_SendResFrame(uint16_t seqNo, uint16_t cmd, uint16_t errCode, uint16_t dataSize, const void* pData);
void NVM_Init(void);
void NVM_ResetDefault(void);
void NVM_SaveCanConfig(const ST_CAN_CONFIG* pstCfg);
uint16_t CMN_CalcChecksum(const void* pBuf, size_t size);
void CMN_ResetMcu(void);

// =============================================================================
// 初期化 (setup)
// =============================================================================

/**
 * @brief マイコン起動時の初期化処理を行う。
 *        LEDピン、EEPROM設定読み込み、USB CDCシリアル、CANコントローラの初期化を行う。
 * @param なし
 * @return なし
 */
void setup() {
    // 内蔵LEDピン初期化
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // EEPROMからCAN通信設定を読み込み
    NVM_Init();

    // USB CDCシリアル初期化 (R4JigAppとの通信ポート)
    Serial.begin(ECHO_SERIAL_BAUDRATE);

    // CANコントローラ初期化
    CAN_InitController();
}

// =============================================================================
// メインループ (loop)
// =============================================================================

/**
 * @brief メインループ処理。
 *        1. CANバスからのフレーム受信・エコー返信処理
 *        2. PC (R4JigApp) からのUSB CDCシリアルコマンド処理
 *        3. 500ms間隔の内蔵LED点滅 (ハートビート)
 * @param なし
 * @return なし
 */
void loop() {
    // 1. CANバスからの受信・エコー返信処理
    CAN_ProcessEcho();

    // 2. PC (R4JigApp) からのUSB CDCシリアルコマンド処理
    CDC_ProcessSerialCommand();

    // 3. 500ms間隔のLED点滅 (ハートビート)
    uint32_t const currentMs = millis(); // 現在時刻 (ms)
    if (currentMs - g_lastLedToggleMs >= ECHO_LED_BLINK_INTERVAL_MS) {
        g_lastLedToggleMs = currentMs;
        g_ledState = !g_ledState;
        digitalWrite(LED_BUILTIN, g_ledState ? HIGH : LOW);
    }
}

// =============================================================================
// CAN 通信処理
// =============================================================================

/**
 * @brief 設定値に基づいてCANコントローラを初期化する。
 * @param なし
 * @return なし
 */
void CAN_InitController(void) {
    CanBitRate br = CanBitRate::BR_500k; // CANボーレート設定値
    switch (g_stCanConfig.bitRate) {
        case CAN_BITRATE_125K:  br = CanBitRate::BR_125k; break;
        case CAN_BITRATE_250K:  br = CanBitRate::BR_250k; break;
        case CAN_BITRATE_500K:  br = CanBitRate::BR_500k; break;
        case CAN_BITRATE_1000K: br = CanBitRate::BR_1000k; break;
        default:                br = CanBitRate::BR_500k; break;
    }
    if (!CAN.begin(br)) {
        g_fwErrBits |= ECHO_ERR_BIT_CAN_INIT_ERR; // CAN初期化エラービットをセット
    }
}

/**
 * @brief CANバスからの受信フレームを監視し、設定された受信IDと一致した場合に
 *        設定された送信IDで折り返し送信 (エコー) する。
 * @param なし
 * @return なし
 */
void CAN_ProcessEcho(void) {
    while (CAN.available()) {
        CanMsg const rxMsg = CAN.read(); // CAN受信メッセージ
        bool isExt = rxMsg.isExtendedId(); // 拡張ID判定フラグ
        uint32_t rxId = isExt ? rxMsg.getExtendedId() : rxMsg.getStandardId(); // 受信CAN ID
        uint8_t dlc = (uint8_t)rxMsg.data_length; // 受信データ長
        if (dlc > CAN_FRAME_MAX_DLC) {
            dlc = CAN_FRAME_MAX_DLC; // 8バイト上限クリップ (バッファ保護)
        }

        // 設定された受信IDおよびIDタイプと一致した場合のみエコー処理を実行
        bool idTypeMatch = (g_stCanConfig.idType == CAN_ID_TYPE_EXTENDED) ? isExt : !isExt; // IDタイプ一致判定
        if (idTypeMatch && (rxId == g_stCanConfig.recvId)) {
            int writeResult = -1; // CAN送信戻り値
            int retries = 0; // リトライ回数カウンタ
            bool txIsExt = (g_stCanConfig.idType == CAN_ID_TYPE_EXTENDED); // 送信IDタイプフラグ

            // メールボックス空き待ちリトライ付き送信
            do {
                if (txIsExt) {
                    CanMsg const txMsg(CanExtendedId(g_stCanConfig.sendId), dlc, rxMsg.data); // 送信メッセージ (Extended)
                    writeResult = CAN.write(txMsg);
                } else {
                    CanMsg const txMsg(CanStandardId(g_stCanConfig.sendId), dlc, rxMsg.data); // 送信メッセージ (Standard)
                    writeResult = CAN.write(txMsg);
                }

                if (writeResult < ECHO_WRITE_SUCCESS_MIN) {
                    delayMicroseconds(ECHO_TX_RETRY_DELAY_US);
                    retries++;
                }
            } while (writeResult < ECHO_WRITE_SUCCESS_MIN && retries < ECHO_TX_MAX_RETRY);

            if (writeResult < ECHO_WRITE_SUCCESS_MIN) {
                g_fwErrBits |= ECHO_ERR_BIT_CAN_SEND_ERR; // CAN送信エラービットをセット
            }
        }
    }
}

// =============================================================================
// USB CDC シリアルコマンド処理 (R4JigApp)
// =============================================================================

/**
 * @brief USBシリアルから要求フレームを受信・解析し、各コマンド処理を実行する。
 * @param なし
 * @return なし
 */
void CDC_ProcessSerialCommand(void) {
    // 要求フレームの受信タイムアウト判定 (ヘッダ受信後に一定時間末尾まで届かない場合は破棄)
    if (g_serialRxLen > 0) {
        if ((millis() - g_serialRxLastTime) > CDC_RECV_TIMEOUT_MS) {
            g_serialRxLen = 0; // 受信タイムアウトによりフレーム破棄
        }
    }

    while (Serial.available() > 0) {
        uint8_t c = (uint8_t)Serial.read(); // 受信1バイトデータ

        if (g_serialRxLen == 0) {
            if (c == FRM_HEADER_REQ) {
                g_serialRxBuf[g_serialRxLen++] = c;
                g_serialRxLastTime = millis(); // 受信開始時刻を記録
            }
            continue;
        }

        g_serialRxBuf[g_serialRxLen++] = c;
        g_serialRxLastTime = millis(); // 最終受信時刻を更新

        // 要求フレーム最小ヘッダ: header(1) + seqNo(2) + cmd(2) + dataSize(2) = 7バイト
        if (g_serialRxLen >= FRM_REQ_HEADER_SIZE) {
            uint16_t dataSize = (uint16_t)g_serialRxBuf[5] | ((uint16_t)g_serialRxBuf[6] << 8); // 要求データサイズ
            size_t totalExpected = FRM_REQ_HEADER_SIZE + dataSize + FRM_CHECKSUM_SIZE; // 期待総フレームサイズ

            if (totalExpected > sizeof(g_serialRxBuf)) {
                g_serialRxLen = 0;
                continue;
            }

            if (g_serialRxLen >= totalExpected) {
                // チェックサム検証
                uint16_t rxChecksum = (uint16_t)g_serialRxBuf[totalExpected - FRM_CHECKSUM_SIZE] | ((uint16_t)g_serialRxBuf[totalExpected - 1] << 8); // 受信チェックサム
                uint16_t calcChecksum = CMN_CalcChecksum(g_serialRxBuf, totalExpected - FRM_CHECKSUM_SIZE); // 計算チェックサム

                if (rxChecksum == calcChecksum) {
                    uint16_t seqNo = (uint16_t)g_serialRxBuf[1] | ((uint16_t)g_serialRxBuf[2] << 8); // 要求シーケンス番号
                    uint16_t cmd = (uint16_t)g_serialRxBuf[3] | ((uint16_t)g_serialRxBuf[4] << 8); // コマンドID
                    uint8_t* pData = &g_serialRxBuf[FRM_REQ_HEADER_SIZE]; // データ部先頭ポインタ

                    switch (cmd) {
                        case CMD_GET_FW_INFO:
                            CDC_ExecReqCmd_GetFwInfo(seqNo, cmd, dataSize, pData);
                            break;

                        case CMD_GET_CAN_CONFIG:
                            CDC_ExecReqCmd_GetCanConfig(seqNo, cmd, dataSize, pData);
                            break;

                        case CMD_SET_CAN_CONFIG:
                            CDC_ExecReqCmd_SetCanConfig(seqNo, cmd, dataSize, pData);
                            break;

                        case CMD_ERASE_EEPROM:
                            CDC_ExecReqCmd_EraseEeprom(seqNo, cmd, dataSize, pData);
                            break;

                        case CMD_GET_FW_ERR:
                            CDC_ExecReqCmd_GetFwErr(seqNo, cmd, dataSize, pData);
                            break;

                        case CMD_CLEAR_FW_ERR:
                            CDC_ExecReqCmd_ClearFwErr(seqNo, cmd, dataSize, pData);
                            break;

                        default:
                            CDC_SendResFrame(seqNo, cmd, FRM_ERR_PARAM, FRM_NO_DATA_SIZE, NULL);
                            break;
                    }
                }

                // フレーム処理完了によりバッファクリア
                g_serialRxLen = 0;
            }
        }
    }
}

/**
 * @brief FW情報取得コマンドの実行
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] dataSize 要求データサイズ
 * @param[in] pData 要求データポインタ
 * @return なし
 */
void CDC_ExecReqCmd_GetFwInfo(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t resDataSize = FRM_NO_DATA_SIZE; // 応答フレームデータサイズ
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード
    ST_FW_INFO stFwInfo; // FW情報構造体一時領域
    const void* pBuf = NULL; // 応答データバッファポインタ

    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        memset(&stFwInfo, 0, sizeof(stFwInfo));
        strncpy(stFwInfo.szMakerName, MAKER_NAME, sizeof(stFwInfo.szMakerName) - 1);
        strncpy(stFwInfo.szFwName, FW_NAME, sizeof(stFwInfo.szFwName) - 1);
        stFwInfo.fwVer = FW_VER;

        // Device Unique ID (RA4M1 128bit UID)
        bsp_unique_id_t const * pUid = R_BSP_UniqueIdGet(); // マイコン固有UID取得ポインタ
        if (pUid != NULL) {
            memcpy(stFwInfo.device_uid, pUid->unique_id_bytes, sizeof(stFwInfo.device_uid));
        }

        resDataSize = sizeof(stFwInfo);
        pBuf = &stFwInfo;
    }

    CDC_SendResFrame(seqNo, cmd, errCode, resDataSize, pBuf);
}

/**
 * @brief CAN通信設定取得コマンドの実行
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] dataSize 要求データサイズ
 * @param[in] pData 要求データポインタ
 * @return なし
 */
void CDC_ExecReqCmd_GetCanConfig(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t resDataSize = FRM_NO_DATA_SIZE; // 応答フレームデータサイズ
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード
    uint8_t aPayload[CAN_CONFIG_PAYLOAD_SIZE]; // 送信用ペイロードバッファ (13バイト: bitRate:4 + sendId:4 + idType:1 + recvId:4)
    const void* pBuf = NULL; // 応答データバッファポインタ

    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        memcpy(&aPayload[0], &g_stCanConfig.bitRate, sizeof(g_stCanConfig.bitRate));
        memcpy(&aPayload[4], &g_stCanConfig.sendId, sizeof(g_stCanConfig.sendId));
        aPayload[8] = g_stCanConfig.idType;
        memcpy(&aPayload[9], &g_stCanConfig.recvId, sizeof(g_stCanConfig.recvId));

        resDataSize = sizeof(aPayload);
        pBuf = aPayload;
    }

    CDC_SendResFrame(seqNo, cmd, errCode, resDataSize, pBuf);
}

/**
 * @brief CAN通信設定変更コマンドの実行
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] dataSize 要求データサイズ
 * @param[in] pData 要求データポインタ
 * @return なし
 */
void CDC_ExecReqCmd_SetCanConfig(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    uint16_t expectedSize = CAN_CONFIG_PAYLOAD_SIZE; // 期待要求サイズ (13バイト)
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード

    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        uint32_t bitRate; // 設定用ボーレート一時領域
        uint32_t sendId; // 設定用送信CAN ID一時領域
        uint8_t idType; // 設定用IDタイプ一時領域
        uint32_t recvId; // 設定用受信CAN ID一時領域

        memcpy(&bitRate, &pData[0], sizeof(bitRate));
        memcpy(&sendId, &pData[4], sizeof(sendId));
        idType = pData[8];
        memcpy(&recvId, &pData[9], sizeof(recvId));

        // パラメータ検証
        bool isValidBaud = (bitRate == CAN_BITRATE_125K || bitRate == CAN_BITRATE_250K ||
                            bitRate == CAN_BITRATE_500K || bitRate == CAN_BITRATE_1000K); // 有効ボーレート判定
        bool isValidIdType = (idType == CAN_ID_TYPE_STANDARD || idType == CAN_ID_TYPE_EXTENDED); // 有効IDタイプ判定
        bool isValidSendId = (idType == CAN_ID_TYPE_EXTENDED) ? (sendId <= CAN_ID_EXTENDED_MAX) : (sendId <= CAN_ID_STANDARD_MAX); // 有効送信ID判定
        bool isValidRecvId = (idType == CAN_ID_TYPE_EXTENDED) ? (recvId <= CAN_ID_EXTENDED_MAX) : (recvId <= CAN_ID_STANDARD_MAX); // 有効受信ID判定
        bool isDiffId = (sendId != recvId); // 送受信ID重複回避判定

        if (!isValidBaud || !isValidIdType || !isValidSendId || !isValidRecvId || !isDiffId) {
            errCode = FRM_ERR_PARAM; // パラメータ不正
        } else {
            ST_CAN_CONFIG stNewCfg; // 新規設定データ一時領域
            memset(&stNewCfg, 0, sizeof(stNewCfg));
            stNewCfg.magic = EEPROM_MAGIC;
            stNewCfg.bitRate = bitRate;
            stNewCfg.sendId = sendId;
            stNewCfg.idType = idType;
            stNewCfg.recvId = recvId;

            // EEPROMへ永続保存
            NVM_SaveCanConfig(&stNewCfg);
        }
    }

    // 応答フレーム送信
    CDC_SendResFrame(seqNo, cmd, errCode, FRM_NO_DATA_SIZE, NULL);

    // 成功時は応答送信後にマイコンを再起動して新設定を反映
    if (errCode == FRM_ERR_SUCCESS) {
        delay(CMD_WAIT_SEND_END_MS);
        CMN_ResetMcu();
    }
}

/**
 * @brief EEPROM消去 (工場出荷設定初期化) コマンドの実行
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] dataSize 要求データサイズ
 * @param[in] pData 要求データポインタ
 * @return なし
 */
void CDC_ExecReqCmd_EraseEeprom(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード

    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        NVM_ResetDefault(); // 工場出荷時設定へ初期化
    }

    // 応答フレーム送信
    CDC_SendResFrame(seqNo, cmd, errCode, FRM_NO_DATA_SIZE, NULL);

    // 成功時は応答送信後にマイコンを再起動して初期設定を反映
    if (errCode == FRM_ERR_SUCCESS) {
        delay(CMD_WAIT_SEND_END_MS);
        CMN_ResetMcu();
    }
}

/**
 * @brief FWエラー取得コマンドの実行
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] dataSize 要求データサイズ
 * @param[in] pData 要求データポインタ
 * @return なし
 */
void CDC_ExecReqCmd_GetFwErr(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード
    const void* pBuf = NULL; // 応答データバッファポインタ
    uint16_t resDataSize = FRM_NO_DATA_SIZE; // 応答フレームデータサイズ

    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        resDataSize = sizeof(g_fwErrBits);
        pBuf = &g_fwErrBits;
    }

    CDC_SendResFrame(seqNo, cmd, errCode, resDataSize, pBuf);
}

/**
 * @brief FWエラークリアコマンドの実行
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] dataSize 要求データサイズ
 * @param[in] pData 要求データポインタ
 * @return なし
 */
void CDC_ExecReqCmd_ClearFwErr(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード

    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        g_fwErrBits = 0; // エラービットクリア
    }

    CDC_SendResFrame(seqNo, cmd, errCode, FRM_NO_DATA_SIZE, NULL);
}

/**
 * @brief 応答フレームを作成しUSBシリアルへ送信する。
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] errCode エラーコード
 * @param[in] dataSize データサイズ
 * @param[in] pData データ部ポインタ
 * @return なし
 */
void CDC_SendResFrame(uint16_t seqNo, uint16_t cmd, uint16_t errCode, uint16_t dataSize, const void* pData) {
    size_t totalSize = FRM_RES_HEADER_SIZE + dataSize + FRM_CHECKSUM_SIZE; // 応答総フレームサイズ
    if (totalSize > sizeof(g_serialTxBuf)) {
        return;
    }

    // 応答ヘッダ構築
    g_serialTxBuf[0] = FRM_HEADER_RES;
    g_serialTxBuf[1] = (uint8_t)(seqNo & 0xFF);
    g_serialTxBuf[2] = (uint8_t)((seqNo >> 8) & 0xFF);
    g_serialTxBuf[3] = (uint8_t)(cmd & 0xFF);
    g_serialTxBuf[4] = (uint8_t)((cmd >> 8) & 0xFF);
    g_serialTxBuf[5] = (uint8_t)(errCode & 0xFF);
    g_serialTxBuf[6] = (uint8_t)((errCode >> 8) & 0xFF);
    g_serialTxBuf[7] = (uint8_t)(dataSize & 0xFF);
    g_serialTxBuf[8] = (uint8_t)((dataSize >> 8) & 0xFF);

    // データ部コピー
    if (dataSize > 0 && pData != NULL) {
        memcpy(&g_serialTxBuf[FRM_RES_HEADER_SIZE], pData, dataSize);
    }

    // チェックサム計算
    uint16_t checksum = CMN_CalcChecksum(g_serialTxBuf, FRM_RES_HEADER_SIZE + dataSize); // 計算チェックサム
    g_serialTxBuf[totalSize - 2] = (uint8_t)(checksum & 0xFF);
    g_serialTxBuf[totalSize - 1] = (uint8_t)((checksum >> 8) & 0xFF);

    // 送信
    Serial.write(g_serialTxBuf, totalSize);
    Serial.flush();
}

// =============================================================================
// 不揮発性メモリ (EEPROM) 設定管理
// =============================================================================

/**
 * @brief 不揮発設定 (EEPROM) の初期化を行う。
 *        マジック番号またはチェックサムが不正な場合は工場出荷時設定に初期化する。
 * @param なし
 * @return なし
 */
void NVM_Init(void) {
    EEPROM.get(EEPROM_ADDR_CAN_CONFIG, g_stCanConfig);
    uint16_t expected = CMN_CalcChecksum(&g_stCanConfig, sizeof(g_stCanConfig) - sizeof(uint16_t)); // チェックサム期待値
    if (g_stCanConfig.magic != EEPROM_MAGIC || g_stCanConfig.checksum != expected) {
        NVM_ResetDefault();
    }
}

/**
 * @brief CAN設定を工場出荷時のデフォルト値に初期化し、EEPROMに保存する。
 * @param なし
 * @return なし
 */
void NVM_ResetDefault(void) {
    memset(&g_stCanConfig, 0, sizeof(g_stCanConfig));
    g_stCanConfig.magic = EEPROM_MAGIC;
    g_stCanConfig.bitRate = CAN_DEFAULT_BITRATE;
    g_stCanConfig.sendId = CAN_DEFAULT_SEND_ID;
    g_stCanConfig.idType = CAN_DEFAULT_ID_TYPE;
    g_stCanConfig.recvId = CAN_DEFAULT_RECV_ID;
    g_stCanConfig.checksum = CMN_CalcChecksum(&g_stCanConfig, sizeof(g_stCanConfig) - sizeof(uint16_t));

    EEPROM.put(EEPROM_ADDR_CAN_CONFIG, g_stCanConfig);
}

/**
 * @brief CAN設定をRAMおよびEEPROMへ保存する。
 * @param[in] pstCfg 保存するCAN設定へのポインタ
 * @return なし
 */
void NVM_SaveCanConfig(const ST_CAN_CONFIG* pstCfg) {
    if (pstCfg == NULL) return;
    memcpy(&g_stCanConfig, pstCfg, sizeof(ST_CAN_CONFIG));
    g_stCanConfig.magic = EEPROM_MAGIC;
    g_stCanConfig.checksum = CMN_CalcChecksum(&g_stCanConfig, sizeof(ST_CAN_CONFIG) - sizeof(uint16_t));

    EEPROM.put(EEPROM_ADDR_CAN_CONFIG, g_stCanConfig);
}

// =============================================================================
// 共通ユーティリティ
// =============================================================================

/**
 * @brief バッファ全体の加算チェックサム (16-bit) を計算する。
 * @param[in] pBuf 計算対象バッファのポインタ
 * @param[in] size バイト数
 * @return 計算された16ビットチェックサム値
 */
uint16_t CMN_CalcChecksum(const void* pBuf, size_t size) {
    const uint8_t* p = (const uint8_t*)pBuf; // バッファポインタ
    uint16_t sum = 0; // チェックサム累積値
    for (size_t i = 0; i < size; i++) { // チェックサム計算ループカウンタ
        sum = (uint16_t)(sum + p[i]);
    }
    return sum;
}

/**
 * @brief マイコンのソフトウェアリセット (再起動) を実行する。
 * @param なし
 * @return なし
 */
void CMN_ResetMcu(void) {
    NVIC_SystemReset();
}
