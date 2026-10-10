// Copyright © 2026 Shiomachi Software. All rights reserved.
/**
 * @file Cdc.cpp
 * @brief USB CDC (仮想COMポート) 通信制御実装
 */
#include "Cdc.h"
#include "Nvm.h"
#include "Can.h"
#include "bsp_api.h"

// =============================================================================
// 定数定義
// =============================================================================
#define FRM_NO_DATA_SIZE            0           // データ部なしサイズ (0バイト)

// =============================================================================
// 静的変数
// =============================================================================
// シリアル受信バッファ (CDC_Task 専用)
static uint8_t f_serialRxBuf[sizeof(ST_FRM_REQ_FRAME) + CDC_SERIAL_BUFFER_EXTRA]; // シリアル受信バッファ
static size_t f_serialRxLen = 0; // シリアル受信データ長
static uint32_t f_serialRxLastTime = 0; // 最終受信時刻 (ms)

// シリアル送信バッファ (CDC_Task 専用: スタックオーバーフロー防止のため静的確保)
static uint8_t f_serialTxBuf[sizeof(ST_FRM_RES_FRAME)]; // シリアル送信バッファ

// =============================================================================
// プロトタイプ宣言 (内部関数)
// =============================================================================
static void CDC_ProcessSerialCommand(void);
static void CDC_ExecReqCmd_GetFwInfo(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
static void CDC_ExecReqCmd_GetCanConfig(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
static void CDC_ExecReqCmd_SetCanConfig(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
static void CDC_ExecReqCmd_EraseEeprom(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
static void CDC_ExecReqCmd_GetFwErr(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
static void CDC_ExecReqCmd_ClearFwErr(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData);
static void CDC_SendResFrame(uint16_t seqNo, uint16_t cmd, uint16_t errCode, uint16_t dataSize, const void* pData);

// =============================================================================
// USB CDC 初期化
// =============================================================================

/**
 * @brief USB CDC シリアル通信制御の初期化を行う。
 * @param なし
 * @return なし
 */
void CDC_Init(void) {
    f_serialRxLen = 0;
    f_serialRxLastTime = 0;
}

// =============================================================================
// USB CDC シリアル設定タスク [優先度: 1 (低)] - PCシリアル通信専任
// =============================================================================

/**
 * @brief USB CDC シリアル設定タスク(FreeRTOS タスク、優先度: 1)。
 *        PC (JigApp) からのシリアルコマンドを受信・解析し、対応する処理および応答を返す。
 * @param[in] pvParameters タスク起動パラメータ(未使用)
 * @return なし
 */
void CDC_Task(void* pvParameters) {
    (void)pvParameters;

    for (;;) {
        // WDT死活監視チェックイン
        CMN_WdtCheckIn(WDT_TASK_ID_CDC);

        // JigAppからの通信パケット処理 (受信処理およびタイムアウト判定)
        CDC_ProcessSerialCommand();

        if (Serial.available() == 0) {
            // 受信データがない通常時はスリープ(CPU負荷ゼロ)
            CMN_SafeDelayMs(CDC_POLL_DELAY_MS);
        }
    }
}

// =============================================================================
// USBシリアルコマンド処理 (JigApp)
// =============================================================================

/**
 * @brief USBシリアルから要求フレームを受信・パケット解析し、各コマンド処理を実行する。
 * @param なし
 * @return なし
 */
static void CDC_ProcessSerialCommand(void) {
    // 要求フレームの受信タイムアウト判定 (ヘッダ受信後に一定時間末尾まで届かない場合は破棄)
    if (f_serialRxLen > 0) {
        if ((millis() - f_serialRxLastTime) > CDC_RECV_TIMEOUT_MS) {
            f_serialRxLen = 0; // 受信タイムアウトによりフレーム破棄
        }
    }

    while (Serial.available() > 0) {
        uint8_t c = (uint8_t)Serial.read(); // 受信1バイトデータ

        if (f_serialRxLen == 0) {
            if (c == FRM_HEADER_REQ) {
                f_serialRxBuf[f_serialRxLen++] = c;
                f_serialRxLastTime = millis(); // 受信開始時刻を記録
            }
            continue;
        }

        f_serialRxBuf[f_serialRxLen++] = c;
        f_serialRxLastTime = millis(); // 最終受信時刻を更新

        // 要求フレーム最小ヘッダ: header(1) + seqNo(2) + cmd(2) + dataSize(2) = 7バイト
        if (f_serialRxLen >= FRM_REQ_HEADER_SIZE) {
            uint16_t dataSize = (uint16_t)f_serialRxBuf[5] | ((uint16_t)f_serialRxBuf[6] << 8); // 要求データサイズ
            size_t totalExpected = FRM_REQ_HEADER_SIZE + dataSize + FRM_CHECKSUM_SIZE; // 期待総フレームサイズ

            if (totalExpected > sizeof(f_serialRxBuf)) {
                f_serialRxLen = 0;
                continue;
            }

            if (f_serialRxLen >= totalExpected) {
                // チェックサム検証
                uint16_t rxChecksum = (uint16_t)f_serialRxBuf[totalExpected - FRM_CHECKSUM_SIZE] | ((uint16_t)f_serialRxBuf[totalExpected - 1] << 8); // 受信チェックサム
                uint16_t calcChecksum = CMN_CalcChecksum(f_serialRxBuf, totalExpected - FRM_CHECKSUM_SIZE); // 計算チェックサム

                if (rxChecksum == calcChecksum) {
                    uint16_t seqNo = (uint16_t)f_serialRxBuf[1] | ((uint16_t)f_serialRxBuf[2] << 8); // 要求シーケンス番号
                    uint16_t cmd = (uint16_t)f_serialRxBuf[3] | ((uint16_t)f_serialRxBuf[4] << 8); // コマンドID
                    uint8_t* pData = &f_serialRxBuf[FRM_REQ_HEADER_SIZE]; // データ部先頭ポインタ

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
                f_serialRxLen = 0;
            }
        }
    }
}

// =============================================================================
// コマンド別処理関数
// =============================================================================

/**
 * @brief FW情報取得コマンドの実行
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] dataSize 要求データサイズ
 * @param[in] pData 要求データポインタ
 * @return なし
 */
static void CDC_ExecReqCmd_GetFwInfo(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t resDataSize = FRM_NO_DATA_SIZE; // 応答フレームデータサイズ
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード
    ST_FW_INFO stFwInfo; // FW情報構造体一時領域
    const void* pBuf = NULL; // 応答データバッファポインタ

    // データサイズチェック
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

    // 応答フレーム送信
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
static void CDC_ExecReqCmd_GetCanConfig(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t resDataSize = FRM_NO_DATA_SIZE; // 応答フレームデータサイズ
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード
    uint8_t aCfgData[CAN_CONFIG_PAYLOAD_SIZE]; // 応答用設定データバッファ (bitRate[4]+sendId[4]+idType[1])
    const void* pBuf = NULL; // 応答データバッファポインタ

    // データサイズチェック
    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        ST_CAN_CONFIG stCanConfig; // CAN設定データ構造体一時領域
        NVM_GetCanConfig(&stCanConfig);

        memcpy(&aCfgData[0], &stCanConfig.bitRate, sizeof(uint32_t));
        memcpy(&aCfgData[4], &stCanConfig.sendId, sizeof(uint32_t));
        aCfgData[8] = stCanConfig.idType;

        resDataSize = sizeof(aCfgData);
        pBuf = aCfgData;
    }

    // 応答フレーム送信
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
static void CDC_ExecReqCmd_SetCanConfig(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    uint16_t expectedSize = CAN_CONFIG_PAYLOAD_SIZE; // 要求フレームデータサイズの期待値 (bitRate[4]+sendId[4]+idType[1])
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード
    ST_CAN_CONFIG stNewCfg; // 設定変更用CAN設定構造体一時領域

    // データサイズチェック
    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        memset(&stNewCfg, 0, sizeof(stNewCfg));
        memcpy(&stNewCfg.bitRate, &pData[0], sizeof(uint32_t));
        memcpy(&stNewCfg.sendId, &pData[4], sizeof(uint32_t));
        stNewCfg.idType = pData[8];

        // 引数バリデーション
        if ((stNewCfg.bitRate != CAN_BITRATE_125K) &&
            (stNewCfg.bitRate != CAN_BITRATE_250K) &&
            (stNewCfg.bitRate != CAN_BITRATE_500K) &&
            (stNewCfg.bitRate != CAN_BITRATE_1000K)) {
            errCode = FRM_ERR_PARAM; // ボーレート範囲外
        } else if ((stNewCfg.idType != CAN_ID_TYPE_STANDARD) && (stNewCfg.idType != CAN_ID_TYPE_EXTENDED)) {
            errCode = FRM_ERR_PARAM; // IDタイプ範囲外 (0:標準, 1:拡張)
        } else if ((stNewCfg.idType == CAN_ID_TYPE_STANDARD) && (stNewCfg.sendId > CAN_ID_STANDARD_MAX)) {
            errCode = FRM_ERR_PARAM; // 標準CAN ID範囲外 (0x000〜0x7FF)
        } else if ((stNewCfg.idType == CAN_ID_TYPE_EXTENDED) && (stNewCfg.sendId > CAN_ID_EXTENDED_MAX)) {
            errCode = FRM_ERR_PARAM; // 拡張CAN ID範囲外 (0x00000000〜0x1FFFFFFF)
        }
    }

    // 成功・失敗に関わらず応答フレームを送信
    CDC_SendResFrame(seqNo, cmd, errCode, FRM_NO_DATA_SIZE, NULL);

    if (errCode == FRM_ERR_SUCCESS) {
        // 正常系: 設定をEEPROMへ保存
        NVM_SaveCanConfig(&stNewCfg);

        // 応答フレームの送信完了(ホスト側からの読み取り完了)を待ってからマイコンをリセット
        CMN_SafeDelayMs(CMD_WAIT_SEND_END);
        CMN_ResetMcu();
    }
}

/**
 * @brief EEPROM消去コマンドの実行
 * @param[in] seqNo シーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] dataSize 要求データサイズ
 * @param[in] pData 要求データポインタ
 * @return なし
 */
static void CDC_ExecReqCmd_EraseEeprom(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード

    // データサイズチェック
    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    }

    // 成功・失敗に関わらず応答フレームを送信
    CDC_SendResFrame(seqNo, cmd, errCode, FRM_NO_DATA_SIZE, NULL);

    if (errCode == FRM_ERR_SUCCESS) {
        // 正常系: 設定をデフォルトに戻す
        NVM_ResetDefault();

        // 応答フレームの送信完了(ホスト側からの読み取り完了)を待ってからマイコンをリセット
        CMN_SafeDelayMs(CMD_WAIT_SEND_END);
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
static void CDC_ExecReqCmd_GetFwErr(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t resDataSize = FRM_NO_DATA_SIZE; // 応答フレームデータサイズ
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード
    uint32_t errBits = 0; // FWエラービット値
    const void* pBuf = NULL; // 応答データバッファポインタ

    // データサイズチェック
    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        errBits = CMN_GetErrorBits();
        resDataSize = sizeof(errBits);
        pBuf = &errBits;
    }

    // 応答フレーム送信
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
static void CDC_ExecReqCmd_ClearFwErr(uint16_t seqNo, uint16_t cmd, uint16_t dataSize, const uint8_t* pData) {
    (void)pData;
    uint16_t expectedSize = FRM_NO_DATA_SIZE; // 要求フレームデータサイズの期待値
    uint16_t errCode = FRM_ERR_SUCCESS; // エラーコード

    // データサイズチェック
    if (dataSize != expectedSize) {
        errCode = FRM_ERR_DATA_SIZE; // データサイズ不正
    } else {
        CMN_ClearErrorBits();
    }

    // 応答フレーム送信
    CDC_SendResFrame(seqNo, cmd, errCode, FRM_NO_DATA_SIZE, NULL);
}

// =============================================================================
// 応答フレーム送出
// =============================================================================

/**
 * @brief PC (JigApp) へ応答フレームを構築してシリアル送信する。
 * @param[in] seqNo 要求フレームから引き継いだシーケンス番号
 * @param[in] cmd コマンドID
 * @param[in] errCode エラーコード (FRM_ERR_SUCCESS等)
 * @param[in] dataSize 応答データサイズ(バイト単位)
 * @param[in] pData 応答データバッファへのポインタ(データなしの場合はNULL)
 * @return なし
 */
static void CDC_SendResFrame(uint16_t seqNo, uint16_t cmd, uint16_t errCode, uint16_t dataSize, const void* pData) {
    ST_FRM_RES_FRAME* pstRes = (ST_FRM_RES_FRAME*)f_serialTxBuf; // 応答フレーム構築用ポインタ

    pstRes->header = FRM_HEADER_RES;
    pstRes->seqNo = seqNo;
    pstRes->cmd = cmd;
    pstRes->errCode = errCode;
    pstRes->dataSize = dataSize;
    if (dataSize > 0 && pData != NULL) {
        memcpy(pstRes->aData, pData, dataSize);
    }

    // チェックサム計算 (header から aData 末尾まで)
    size_t sumSize = FRM_RES_HEADER_SIZE + dataSize; // チェックサム計算対象サイズ
    uint16_t chk = CMN_CalcChecksum(f_serialTxBuf, sumSize); // 計算チェックサム値
    memcpy(&f_serialTxBuf[sumSize], &chk, FRM_CHECKSUM_SIZE);

    // シリアル送信 (Serialを触るのはこのタスクのみ)
    Serial.write(f_serialTxBuf, sumSize + FRM_CHECKSUM_SIZE);
    Serial.flush();
}
